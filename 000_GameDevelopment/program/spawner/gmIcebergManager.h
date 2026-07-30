#pragma once
#include <string>
#include <vector>
#include <memory>
#include "gmSpawnerManagerBase.h"
#include "../object/gmIceberg.h"
#include "../gmGameConfig.h"

namespace gm {

    class gmMapManager;
    class gmWaterPlane;
    class gmCollisionSystem;

    // ------------------------------------------------------------
    // 氷山のスポーンと一元管理
    // マップ南端の帯状エリアからランダムな間隔・位置で氷山を生成する。
    // ------------------------------------------------------------
    class gmIcebergManager : public gmSpawnerManagerBase<gmIceberg> {
    public:
        gmIcebergManager(
            const std::shared_ptr<gmMapManager>& map,
            const std::shared_ptr<gmWaterPlane>& water,
            const std::vector<std::string>& crystalPaths,
            const std::string& texturePath,
            const std::shared_ptr<gmCollisionSystem>& collisionSystem);

    protected:
        void trySpawn() override;
        float rollNextInterval() const override;

    private:
        std::shared_ptr<gmMapManager> map_;
        std::shared_ptr<gmWaterPlane> water_;
        std::vector<std::string> crystalPaths_;
        Shared<dxe::Texture> texture_;

        // 衝突システム
        std::shared_ptr<gmCollisionSystem> collisionSystem_;

        // 氷山メッシュのテンプレート(起動時にまとめて生成し、以後使い回す)
        std::vector<Shared<dxe::Mesh>> meshTemplates_;
        static constexpr int MESH_TEMPLATE_COUNT = 5; // 見た目のバリエーション数

        // ---- 生成範囲(グリッド単位。南端の帯状エリア) ----
        // worldZ = -y * CELL_SIZE のため、"南端"は行番号(y)が大きい側
        static constexpr int SPAWN_ROW_MIN = MAP_CHIP_HEIGHT - 3;  // 南端から3セル分の帯
        static constexpr int SPAWN_ROW_MAX = MAP_CHIP_HEIGHT - 2;  // 端ぎりぎりは避ける
        static constexpr int SPAWN_COL_MARGIN = 4;                 // 東西の端だけ避ける

        static constexpr float SPAWN_INTERVAL_MIN = 22.5f;   // 秒
        static constexpr float SPAWN_INTERVAL_MAX = 45.0f;

        static constexpr int SPAWN_LAND_RETRY = 8; // 候補セルが陸地だった場合のリトライ回数
    };
}
