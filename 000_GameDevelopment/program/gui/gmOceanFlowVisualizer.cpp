#include "gmOceanFlowVisualizer.h"
#include "../map/gmMapManager.h"
#include "../gmGameConfig.h"
#include "../util/gmRenderUtil.h"
#include <DxLib.h>
#include <cmath>

namespace gm {
    
    // ヘルパー関数
    namespace {
        // 3Dワールド座標からスクリーン座標(とZ深度)を安全に計算する
        // 戻り値: x, y はスクリーンピクセル座標、z は 0.0〜1.0 の深度値
        bool ProjectToScreen(
            const tnl::Vector3& worldPos,
            const tnl::Matrix& view,
            const tnl::Matrix& proj,
            float screenW,
            float screenH,
            tnl::Vector3& outScreenPos
        ) {
            // 1) ビュー・プロジェクション（合成）行列の作成
            tnl::Matrix vp = view * proj;

            // 2) 行列とベクトルの乗算（Row-Major 前提のパースペクティブ変換）
            // ※w成分の計算を含め、各座標の射影計算を行います
            float x = worldPos.x * vp._11 + worldPos.y * vp._21 + worldPos.z * vp._31 + vp._41;
            float y = worldPos.x * vp._12 + worldPos.y * vp._22 + worldPos.z * vp._32 + vp._42;
            float z = worldPos.x * vp._13 + worldPos.y * vp._23 + worldPos.z * vp._33 + vp._43;
            float w = worldPos.x * vp._14 + worldPos.y * vp._24 + worldPos.z * vp._34 + vp._44;

            // 3) カメラの後方にある（wが0以下）場合はクリップ（変換失敗）
            if (w <= 0.0f) {
                return false;
            }

            // 4) w で割ってデバイス正規化座標（NDC: -1.0 〜 1.0）へ変換
            float ndcX = x / w;
            float ndcY = y / w;
            float ndcZ = z / w;

            // 5) ビューポート変換（スクリーンピクセル座標へのマッピング）
            float halfW = screenW * 0.5f;
            float halfH = screenH * 0.5f;

            outScreenPos.x = halfW + (halfW * ndcX);
            outScreenPos.y = halfH - (halfH * ndcY); // 3Dの上方向はスクリーン上ではマイナスなので反転
            outScreenPos.z = ndcZ;                   // 正しい深度(0.0〜1.0)を代入

            return true;
        }
    }

    gmOceanFlowVisualizer::gmOceanFlowVisualizer(std::shared_ptr<gmMapManager> map)
        : map_(map)
    {
        loadResources();
    }

    gmOceanFlowVisualizer::~gmOceanFlowVisualizer()
    {
        releaseResources();
    }

    void gmOceanFlowVisualizer::loadResources()
    {
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

    void gmOceanFlowVisualizer::update()
    {
        if (tnl::Input::IsKeyDownTrigger(tnl::Input::eKeys::KB_F1)) {
            visible_ = !visible_;
        }
    }

    void gmOceanFlowVisualizer::draw(const Shared<dxe::Camera>& camera)
    {
        // 表示OFF / リソース未ロードチェック
        if (!visible_) return;
        if (arrowHandle_ == -1) return;

        gm::ApplyCamera3D(camera);

        auto map = map_.lock();
        if (!map) return;

        // カメラ情報
        tnl::Vector3 camPos = camera->getPosition();
        tnl::Vector3 camForward = camera->forward();

        // 描画サイズ設定（ワールド座標系での一辺の大きさ。セルサイズ等に合わせて調整してください）
        const float HALF_SIZE = CELL_SIZE * 0.4f; // セルから少し隙間を空けるサイズ

        const float renderDistSq = RENDER_DISTANCE * RENDER_DISTANCE;

        // 3D描画用のインデックスデータ（4頂点で2つの三角形を作るための定義。固定値でOK）
        static const WORD indices[6] = { 0, 1, 2, 1, 3, 2 };

        // マップセルループ
        for (int y = 0; y < MAP_CHIP_HEIGHT; y += sampleStep_) {
            for (int x = 0; x < MAP_CHIP_WIDTH; x += sampleStep_) {

                // 島セルは描画スキップ
                if (map->IsLand(x, y)) continue;

                // セル中心のワールド座標
                const float worldX = x * CELL_SIZE + (CELL_SIZE * 0.5f);
                const float worldY = 0.05f; // 地面（0.0f）と完全に重ねるとチラつく（Zファイティング）ため、わずかに浮かせる
                const float worldZ = -y * CELL_SIZE - (CELL_SIZE * 0.5f);
                tnl::Vector3 worldPos{ worldX, worldY, worldZ };

                // 1) カメラ前方チェック
                tnl::Vector3 camTo = worldPos - camPos;
                float forwardDot = tnl::Vector3::Dot(camTo, camForward);
                if (forwardDot <= 0.0f) continue;

                // 2) 距離チェック
                float dx = worldX - camPos.x;
                float dz = worldZ - camPos.z;
                if ((dx * dx + dz * dz) > renderDistSq) continue;

                // 3) 海流データ取得
                tnl::Vector2f fv = map->GetFlow(x, y);
                float fx = fv.x;
                float fz = fv.y;
                float mag = std::sqrt(fx * fx + fz * fz);
                if (mag < 1e-4f) continue;

                // 4) 回転角（XZ平面上での回転。右方向が基準、または画像に合わせて調整）
                // ※アングルが北基準（Zプラス）で、画像が上向きの場合の回転を計算
                float angle = std::atan2f(fx, fz);
                float sinA = std::sin(angle);
                float cosA = std::cos(angle);

                // 5) 回転させた4頂点のローカル座標（XZ平面上の板ポリゴン）
                // 左上 (LT), 右上 (RT), 左下 (LB), 右下 (RB)
                // 画像のテクスチャ座標（U, V）に合わせて定義します
                VERTEX3D vertices[4];

                // 共通の初期化（色は白、アルファ最大）
                for (int i = 0; i < 4; ++i) {
                    vertices[i].dif = GetColorU8(255, 255, 255, 255);
                    vertices[i].spc = GetColorU8(0, 0, 0, 0);
                    vertices[i].norm = VGet(0.0f, 1.0f, 0.0f); // 法線は真上向き
                }

                // ローカル位置（回転前）
                float l_left = -HALF_SIZE;
                float l_right = HALF_SIZE;
                float l_top = HALF_SIZE;  // Zプラス方向（上）
                float l_bottom = -HALF_SIZE;  // Zマイナス方向（下）

                // 回転行列を適用してワールド座標を決定するヘルパーラムダ
                auto setVertex = [&](int idx, float localX, float localZ, float u, float v) {
                    // XZ平面上での2D回転
                    float rx = localX * cosA - localZ * sinA;
                    float rz = localX * sinA + localZ * cosA;

                    vertices[idx].pos = VGet(worldPos.x + rx, worldPos.y, worldPos.z + rz);
                    vertices[idx].u = u;
                    vertices[idx].v = v;
                    };

                // 各頂点の設定 (UVの割り当て)
                setVertex(0, l_left, l_top, 0.0f, 0.0f); // 左上
                setVertex(1, l_right, l_top, 1.0f, 0.0f); // 右上
                setVertex(2, l_left, l_bottom, 0.0f, 1.0f); // 左下
                setVertex(3, l_right, l_bottom, 1.0f, 1.0f); // 右下

                // 6) 3D空間上にポリゴンを描画
                // 透過対応のため、DXLibのブレンドモードが適切に設定されている必要があります
                SetUseLighting(FALSE);
                SetWriteZBuffer3D(FALSE);
                SetUseZBuffer3D(FALSE);
                SetUseBackCulling(FALSE);
                SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
                int rtn = DrawPolygonIndexed3D(vertices, 4, indices, 2, arrowHandle_, TRUE);
                SetUseBackCulling(TRUE);
                SetUseZBuffer3D(TRUE);
                SetWriteZBuffer3D(TRUE);
                SetUseLighting(TRUE);
            }
        }
    }

} // namespace gm
