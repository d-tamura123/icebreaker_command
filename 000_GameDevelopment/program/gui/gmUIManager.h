#pragma once
#include <memory>
#include <functional>
#include <dxe.h>

namespace gm {

    // 前方宣言
    class gmMiniMap;
    class gmOceanFlowVisualizer;
    class gmTopBarUI;
    class gmSpeedHUD;
    class gmRudderHUD;
    class gmWeaponSelectHUD;
    class gmWeaponSelectionState;
    class gmHpBarUI;
    class gmAimReticleUI;
    class gmMapManager;
    class gmPlayerShip;
    class gmIcebergManager;
    class gmTradeShipManager;
    class gmWallet;

    class gmUIManager {
    public:
        gmUIManager(
            const tnl::Vector2f& miniMapPos,
            std::shared_ptr<gmMapManager> map,
            std::shared_ptr<gmPlayerShip> player,
            std::shared_ptr<gmIcebergManager> icebergManager,
            std::shared_ptr<gmTradeShipManager> tradeShipManager,
            std::shared_ptr<gmWallet> wallet,
            std::function<void()> onMenuClick,
            std::shared_ptr<gmWeaponSelectionState> weaponSelection,
            std::function<void()> onRecoveryClick
        );

        ~gmUIManager();

        // UI全体の更新
        // arg4... 照準ドット(3x3)を表示するかどうか(周回・エイムいずれのモードでもtrue。
        //         撃沈演出中・フリーカメラ中等、狙い先自体が無意味なタイミングのみfalse)
        // arg5... エイムモード中かどうか(目盛りメーター・距離表示の表示/非表示に使う)
        // arg6... プレイヤー船〜狙い先までの距離(world単位。距離表示に使う)
        void update(float dt, const Shared<dxe::Camera>& camera, bool cursorModeActive, bool showAimDot, bool isAimMode, float aimTargetDistance);


        // UI全体の描画（要件に合わせてrenderという名前にしています）
        void renderTerrainIntegration(const Shared<dxe::Camera>& camera);
        void render(const Shared<dxe::Camera>& camera);

    private:
        // 将来的に他のUIオブジェクト（体力バー、アイテムUI等）が増えた際にも
        // 管理しやすいよう、個別のポインタまたはコンテナで管理します
        std::unique_ptr<gmMiniMap> miniMap_;

        std::unique_ptr<gmOceanFlowVisualizer> flowVisualizer_;

        std::unique_ptr<gmTopBarUI> topBar_;

        std::unique_ptr<gmSpeedHUD> speedHUD_;

        std::unique_ptr<gmRudderHUD> rudderHUD_;

        std::unique_ptr<gmWeaponSelectHUD> weaponSelectHUD_;

        std::unique_ptr<gmHpBarUI> hpBarUI_;

        std::unique_ptr<gmAimReticleUI> aimReticleUI_;

    };
}
