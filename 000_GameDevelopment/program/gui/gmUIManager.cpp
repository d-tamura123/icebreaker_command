#include "gmUIManager.h"
#include "gmMiniMap.h" // 実装ファイル側で実体をインクルード
#include "gmOceanFlowVisualizer.h"
#include "gmTopBarUI.h"
#include "gmSpeedHUD.h"
#include "gmRudderHUD.h"
#include "gmWeaponSelectHUD.h"
#include "gmHpBarUI.h"

namespace gm {

    gmUIManager::gmUIManager(
        const tnl::Vector2f& miniMapPos,
        std::shared_ptr<gmMapManager> map,
        std::shared_ptr<gmPlayerShip> player,
        std::shared_ptr<gmIcebergManager> icebergManager,
        std::shared_ptr<gmWallet> wallet,
        std::function<void()> onMenuClick,
        std::shared_ptr<gmWeaponSelectionState> weaponSelection,
        std::function<void()> onRecoveryClick
    )
    {
        // gmMiniMap のインスタンスを生成して保持
        miniMap_ = std::make_unique<gmMiniMap>(miniMapPos, map, player, icebergManager);

        // gmOceanFlowVisualizer のインスタンスを生成して保持
        flowVisualizer_ = std::make_unique<gmOceanFlowVisualizer>(map);

        // gmTopBarUI(画面上端バー)のインスタンスを生成して保持
        topBar_ = std::make_unique<gmTopBarUI>(std::move(wallet), std::move(onMenuClick));

        // gmSpeedHUD(速度HUD)のインスタンスを生成して保持
        speedHUD_ = std::make_unique<gmSpeedHUD>(player);

        // gmRudderHUD(舵角HUD)のインスタンスを生成して保持
        rudderHUD_ = std::make_unique<gmRudderHUD>(player);

        // gmWeaponSelectHUD(武器選択HUD)のインスタンスを生成して保持
        weaponSelectHUD_ = std::make_unique<gmWeaponSelectHUD>(std::move(weaponSelection), std::move(onRecoveryClick));

        // gmHpBarUI(プレイヤーHPバー)のインスタンスを生成して保持
        hpBarUI_ = std::make_unique<gmHpBarUI>(player);
    }

    gmUIManager::~gmUIManager()
    {
        // std::unique_ptr が自動的に解放するため、明示的なdeleteは不要です
    }

    void gmUIManager::update(float dt, const Shared<dxe::Camera>& camera, bool cursorModeActive)
    {
        if (flowVisualizer_) {
            flowVisualizer_->update(camera);
        }

        if (miniMap_) {
            miniMap_->update(dt); // ミニマップの更新
        }

        if (topBar_) {
            topBar_->setCursorModeActive(cursorModeActive);
            topBar_->update(dt);
        }

        if (speedHUD_) {
            speedHUD_->update(dt);
        }

        if (rudderHUD_) {
            rudderHUD_->update(dt);
        }

        if (weaponSelectHUD_) {
            weaponSelectHUD_->setCursorModeActive(cursorModeActive);
            weaponSelectHUD_->update(dt);
        }

        if (hpBarUI_) {
            hpBarUI_->update(dt);
        }

        // 今後、他のUIのupdate処理が増えたらここに追記します
    }

    void gmUIManager::renderTerrainIntegration(const Shared<dxe::Camera>& camera)
    {
        // Note: 描画順が雑実装。複雑化した場合はレイヤーの実装を検討すること
        if (flowVisualizer_) {
            flowVisualizer_->draw(camera);
        }
        // 今後、他のUIのdraw処理が増えたらここに追記します
    }

    void gmUIManager::render(const Shared<dxe::Camera>& camera)
    {

        // 【注意】DxLibは2D描画関数(DrawExtendGraph等)を呼ぶと
        // 内部の3Dカメラ・射影変換が無効化されるため、
        // 3Dワールド座標を扱う描画は必ず2D描画より先に行うこと。
        // またはgm::ApplyCamera3D(camera)の対策すること
        if (miniMap_) {

            miniMap_->draw(); // ミニマップの描画
        }

        if (topBar_) {
            topBar_->draw(); // 画面上端バーの描画
        }

        if (speedHUD_) {
            speedHUD_->draw(); // 速度HUDの描画
        }

        if (rudderHUD_) {
            rudderHUD_->draw(); // 舵角HUDの描画
        }

        if (weaponSelectHUD_) {
            weaponSelectHUD_->draw(); // 武器選択HUDの描画
        }

        if (hpBarUI_) {
            hpBarUI_->draw(); // プレイヤーHPバーの描画
        }

        // 今後、他のUIのdraw処理が増えたらここに追記します
    }
}
