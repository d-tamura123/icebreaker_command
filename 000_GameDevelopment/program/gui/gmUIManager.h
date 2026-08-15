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
    class gmMapManager;
    class gmPlayerShip;
    class gmIcebergManager;
    class gmWallet;

    class gmUIManager {
    public:
        gmUIManager(
            const tnl::Vector2f& miniMapPos,
            std::shared_ptr<gmMapManager> map,
            std::shared_ptr<gmPlayerShip> player,
            std::shared_ptr<gmIcebergManager> icebergManager,
            std::shared_ptr<gmWallet> wallet,
            std::function<void()> onMenuClick,
            std::shared_ptr<gmWeaponSelectionState> weaponSelection,
            std::function<void()> onRecoveryClick
        );

        ~gmUIManager();

        // UI全体の更新
        // arg3... カーソルモード(Alt押下中)かどうか。ボタン等のホバー/クリック判定の有効/無効に使う
        void update(float dt, const Shared<dxe::Camera>& camera, bool cursorModeActive);


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

    };
}
