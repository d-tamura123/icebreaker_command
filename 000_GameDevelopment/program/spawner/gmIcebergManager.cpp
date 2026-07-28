#include "gmIcebergManager.h"
#include "../map/gmMapManager.h"
#include "../object/gmWaterPlane.h"
#include "../mesh_ex/gmMeshEX.h"
#include <dxe.h>

namespace gm {

    gmIcebergManager::gmIcebergManager(
        const std::shared_ptr<gmMapManager>& map,
        const std::shared_ptr<gmWaterPlane>& water,
        const std::vector<std::string>& crystalPaths,
        const std::string& texturePath)
        : map_(map)
        , water_(water)
        , crystalPaths_(crystalPaths)
    {
        texture_ = dxe::Texture::CreateFromFile(texturePath);

        // 氷山メッシュはここでまとめて生成しておき、スポーン時は使い回す
        // Note:
        // MeshEX::CreateIceChunkは内部でモデルロード+X形式変換を伴う重い処理のため、
        // スポーンのたびに毎回呼ぶと負荷/安定性の両面で問題になるため
        meshTemplates_.reserve(MESH_TEMPLATE_COUNT);
        for (int i = 0; i < MESH_TEMPLATE_COUNT; ++i) {
            auto mesh = MeshEX::CreateIceChunk(crystalPaths_, texture_, 1.0f, 8, -1);
            
            // CreateConvertMV(内部でMV1LoadModelFromMemを使う不安定な経路)が
            // 正しくハンドルを返しているか、生成直後に検証しておく
            if (mesh->getDxMvHdl() == 0) {
#ifdef _DEBUG
                OutputDebugStringA("gmIcebergManager: mesh template creation FAILED (invalid handle)\n");
#endif
                continue; // このテンプレートは使わない
            }
            
            mesh->setDefaultLightEnable(true);
            meshTemplates_.push_back(mesh);
        }

        maxEntities_ = 20; // 同時存在数の上限
        spawnTimer_ = rollNextInterval(); // 開始直後にまとめて湧かないよう最初の間隔を設定
    }

    // ------------------------------------------------------------
    // 南端の帯状エリアから、陸地でないセルを探して1個生成する
    // ------------------------------------------------------------
    void gmIcebergManager::trySpawn()
    {
        if (!map_) return;
        if (meshTemplates_.empty()) return; // 有効なテンプレートが1つも無ければ何もしない

        int gx = 0;
        int gy = 0;
        bool found = false;

        for (int i = 0; i < SPAWN_LAND_RETRY; ++i) {
            gx = tnl::GetRandomDistribution<int>(SPAWN_COL_MARGIN, MAP_CHIP_WIDTH - 1 - SPAWN_COL_MARGIN);
            gy = tnl::GetRandomDistribution<int>(SPAWN_ROW_MIN, SPAWN_ROW_MAX);

            if (!map_->IsLand(gx, gy)) {
                found = true;
                break;
            }
        }

        // 陸地ばかりで候補が見つからなくても、次のタイマーで再挑戦すればよいので無視する
        if (!found) return;

        const float worldX = gx * CELL_SIZE + CELL_SIZE * 0.5f;
        const float worldZ = -gy * CELL_SIZE - CELL_SIZE * 0.5f;

        // 初期Y座標はその場の波高に合わせておく(固定値だと出現時に不自然にポップする)
        float initialY = 0.0f;
        if (water_) {
            initialY = water_->sampleHeight({ worldX, 0.0f, worldZ }, 0.0f);
        }

        // 流氷生成
        // あらかじめ用意したテンプレートからランダムに1つ選ぶ(毎回生成しない)
        auto mesh = meshTemplates_[tnl::GetRandomDistribution<int>(0, (int)meshTemplates_.size() - 1)];

        auto iceberg = std::make_shared<gmIceberg>(
            "iceberg", tnl::Vector3(worldX, initialY, worldZ), mesh);
        iceberg->setMap(map_);
        iceberg->setWater(water_);

        entities_.push_back(iceberg);
    }

    float gmIcebergManager::rollNextInterval() const
    {
        return tnl::GetRandomDistribution<float>(SPAWN_INTERVAL_MIN, SPAWN_INTERVAL_MAX);
    }
}
