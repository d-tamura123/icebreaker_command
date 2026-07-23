#include "gmOceanFlowVisualizer.h"
#include "../map/gmMapManager.h"
#include "../gmGameConfig.h"
#include "../util/gmRenderUtil.h"
#include <DxLib.h>
#include <cmath>

// ---- スロットリング(段階的更新)について ----
// マップ/海流データは起動時ロード後は不変のため、
// 「見える範囲を決めるグリッドセル(centerGX/centerGY)が変わったか」
// だけで再構築の要否を判定できる（＝時間経過による定期判定は不要）。

namespace gm {

    gmOceanFlowVisualizer::gmOceanFlowVisualizer(std::shared_ptr<gmMapManager> map)
        : map_(map)
    {
        loadResources();
        initializeBuffers();
    }

    gmOceanFlowVisualizer::~gmOceanFlowVisualizer()
    {
        releaseResources();
    }

    void gmOceanFlowVisualizer::loadResources()
    {
        // DrawPolygonIndexed3D は生のグラフィックハンドル(int)を要求するため、
        // dxe::Texture ではなく LoadGraph の生ハンドルを使用する
        if (arrowHandle_ == -1) {
            arrowHandle_ = LoadGraph(GRAPHICS_FILE_PATH__OCEAN_FLOW_ARROW);
        }
    }

    void gmOceanFlowVisualizer::releaseResources()
    {
        if (arrowHandle_ != -1) {
            DeleteGraph(arrowHandle_);
            arrowHandle_ = -1;
        }
    }

    //============================================================
    // バッチ用バッファの事前確保
    // （毎フレームのvector再アロケーションを避けるため、最初に1回だけ確保）
    //============================================================
    void gmOceanFlowVisualizer::initializeBuffers()
    {
        vtxBuf_.reserve(MAX_BATCH_VERTICES);
        idxBuf_.reserve((MAX_BATCH_VERTICES / 4) * 6);
    }

    // ============================================================
    // update: F1トグル判定 + 必要ならフローフィールドの再構築
    // フレーム内でcontext_->camera->update()の後に呼ばれる想定のため、
    // このカメラ位置はそのフレームで確定済み。
    // ============================================================
    void gmOceanFlowVisualizer::update(const Shared<dxe::Camera>& camera)
    {
        if (tnl::Input::IsKeyDownTrigger(tnl::Input::eKeys::KB_F1)) {
            visible_ = !visible_;
        }

        if (!visible_) {
            return; // 非表示中は再構築しない
        }

        auto map = map_.lock();
        if (!map) {
            return;
        }

        const tnl::Vector3 camPos = camera->getPosition();
        const tnl::Vector3 camForward = camera->forward();

        // ------------------------------------------------------------
        // グリッド座標の算出
        // カメラ位置をグリッド座標に変換し、RENDER_DISTANCE分だけの矩形範囲だけを回す
        // ------------------------------------------------------------
        const int centerGX = static_cast<int>(camPos.x / CELL_SIZE);
        const int centerGY = static_cast<int>(-camPos.z / CELL_SIZE); // worldZ = -y*CELL_SIZE の符号に注意

        // ------------------------------------------------------------
        // スロットリング判定（段階的更新）
        // マップ/海流データは不変のため、表示すべきセル範囲を決める
        // centerGX/centerGYが前回と同じなら結果も同じ＝再構築不要。
        // ------------------------------------------------------------
        const bool needRebuild = !hasCachedBatches_
            || centerGX != lastCenterGX_
            || centerGY != lastCenterGY_;

        if (!needRebuild) {
            return;
        }

        rebuildBatches(*map, camPos, camForward, centerGX, centerGY);

        lastCenterGX_ = centerGX;
        lastCenterGY_ = centerGY;
        hasCachedBatches_ = true;
    }

    //============================================================
    // バッファに溜まった内容をキャッシュ(vtxBatches_/idxBatches_)へ積む（＝改ページ）
    // 再構築時にのみ呼ばれる。ここでは即描画しない。
    //============================================================
    void gmOceanFlowVisualizer::flushBatch()
    {
        if (vtxBuf_.empty()) {
            return;
        }

        vtxBatches_.push_back(std::move(vtxBuf_));
        idxBatches_.push_back(std::move(idxBuf_));

        vtxBuf_.clear();
        idxBuf_.clear();
        vtxBuf_.reserve(MAX_BATCH_VERTICES);
        idxBuf_.reserve((MAX_BATCH_VERTICES / 4) * 6);
    }

    //============================================================
    // グリッドセルをループしてフローフィールドの矢印メッシュを生成し、
    // vtxBatches_/idxBatches_ に積む（＝再構築本体）。
    // DxLibの描画API(DrawPolygonIndexed3D等)は一切呼ばない、CPU側ロジック。
    //============================================================
    void gmOceanFlowVisualizer::rebuildBatches(
        const gmMapManager& map,
        const tnl::Vector3& camPos,
        const tnl::Vector3& camForward,
        int centerGX,
        int centerGY)
    {
        const float HALF_SIZE = CELL_SIZE * 0.4f;
        const float renderDistSq = (RENDER_DISTANCE / 3) * (RENDER_DISTANCE / 3);
        const float gridRange = RENDER_DISTANCE / CELL_SIZE;

        vtxBuf_.clear();
        idxBuf_.clear();
        vtxBatches_.clear();
        idxBatches_.clear();

        int minGX = std::max(0, centerGX - static_cast<int>(gridRange));
        int maxGX = std::min(MAP_CHIP_WIDTH - 1, centerGX + static_cast<int>(gridRange));
        int minGY = std::max(0, centerGY - static_cast<int>(gridRange));
        int maxGY = std::min(MAP_CHIP_HEIGHT - 1, centerGY + static_cast<int>(gridRange));

        for (int y = minGY; y <= maxGY; y += sampleStep_) {
            for (int x = minGX; x <= maxGX; x += sampleStep_) {

                // 島セルは非表示
                if (map.IsLand(x, y)) {
                    continue;
                }

                // セル中心座標
                const float worldX = x * CELL_SIZE + CELL_SIZE * 0.5f;
                const float worldY = 0.05f; // 地面とのZファイティング回避のため僅かに浮かせる
                const float worldZ = -y * CELL_SIZE - CELL_SIZE * 0.5f;
                tnl::Vector3 worldPos{ worldX, worldY, worldZ };

                // カメラ前方チェック
                tnl::Vector3 camTo = worldPos - camPos;
                if (tnl::Vector3::Dot(camTo, camForward) <= 0.0f) {
                    continue;
                }

                // 距離チェック
                const float dx = worldX - camPos.x;
                const float dz = worldZ - camPos.z;
                if ((dx * dx + dz * dz) > renderDistSq) {
                    continue;
                }

                // FlowField
                tnl::Vector2f fv = map.GetFlow(x, y);
                const float fx = fv.x;
                const float fz = fv.y;
                const float mag = std::sqrt(fx * fx + fz * fz);
                if (mag < 1e-4f) {
                    continue;
                }

                // 回転角
                const float angle = std::atan2f(fx, fz);
                const float sinA = std::sin(angle);
                const float cosA = std::cos(angle);

                // ------------------------------------------------------------
                // バッファ溢れチェック（"改ページ" = 一旦flushしてから続ける）
                // WORDインデックスの範囲(65535)を超えないよう、
                // 4頂点追加する前に必ずチェックする
                // ------------------------------------------------------------
                if (vtxBuf_.size() + 4 > MAX_BATCH_VERTICES) {
                    flushBatch();
                }

                const WORD base = static_cast<WORD>(vtxBuf_.size());

                auto pushVertex = [&](float localX, float localZ, float u, float v) {
                    const float rx = localX * cosA - localZ * sinA;
                    const float rz = localX * sinA + localZ * cosA;

                    VERTEX3D vtx{};
                    vtx.pos = VGet(worldPos.x + rx, worldPos.y, worldPos.z + rz);
                    vtx.dif = GetColorU8(255, 255, 255, 255);
                    vtx.spc = GetColorU8(0, 0, 0, 0);
                    vtx.norm = VGet(0.0f, 1.0f, 0.0f);
                    vtx.u = u;
                    vtx.v = v;
                    vtxBuf_.push_back(vtx);
                    };

                pushVertex(-HALF_SIZE, HALF_SIZE, 0.0f, 0.0f); // 左上
                pushVertex(HALF_SIZE, HALF_SIZE, 1.0f, 0.0f); // 右上
                pushVertex(-HALF_SIZE, -HALF_SIZE, 0.0f, 1.0f); // 左下
                pushVertex(HALF_SIZE, -HALF_SIZE, 1.0f, 1.0f); // 右下

                idxBuf_.push_back(base + 0);
                idxBuf_.push_back(base + 1);
                idxBuf_.push_back(base + 2);
                idxBuf_.push_back(base + 1);
                idxBuf_.push_back(base + 3);
                idxBuf_.push_back(base + 2);
            }
        }

        // ループを抜けた時点で残っている分をまとめて出力（最終ページの印字）
        flushBatch();
    }

    //============================================================
    // キャッシュ済みバッチを実際にドローコールへ流す
    // 再構築の有無に関わらず毎フレーム呼ばれるが、
    // ここでの処理はDrawPolygonIndexed3Dの呼び出しのみで軽量。
    //============================================================
    void gmOceanFlowVisualizer::drawCachedBatches()
    {
        for (size_t i = 0; i < vtxBatches_.size(); ++i) {
            if (vtxBatches_[i].empty()) {
                continue;
            }
            DrawPolygonIndexed3D(
                vtxBatches_[i].data(), static_cast<int>(vtxBatches_[i].size()),
                idxBatches_[i].data(), static_cast<int>(idxBatches_[i].size() / 3),
                arrowHandle_, TRUE);
        }
    }

    // ============================================================
    // draw: キャッシュ済みバッチを実際にGPUへ提出するだけ。
    // マップ/セルのロジックは持たない（update()側の責務）。
    // ============================================================
    void gmOceanFlowVisualizer::draw(const Shared<dxe::Camera>& camera)
    {
        if (!visible_) return;
        if (arrowHandle_ == -1) return;

        gm::ApplyCamera3D(camera);

        // ------------------------------------------------------------
        // 描画ステート設定 + キャッシュ済みバッチの描画
        // ------------------------------------------------------------
        SetUseLighting(FALSE);
        SetWriteZBuffer3D(FALSE);
        SetUseZBuffer3D(FALSE);
        SetUseBackCulling(FALSE);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);

        drawCachedBatches();

        // ------------------------------------------------------------
        // ステートを元に戻す
        // ------------------------------------------------------------
        SetUseBackCulling(TRUE);
        SetUseZBuffer3D(TRUE);
        SetWriteZBuffer3D(TRUE);
        SetUseLighting(TRUE);
    }

} // namespace gm
