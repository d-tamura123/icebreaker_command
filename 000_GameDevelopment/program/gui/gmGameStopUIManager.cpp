// gmGameStopUIManager.cpp
#include "gmGameStopUIManager.h"
#include "gmPauseMenuUI.h"
#include "../util/gmCursorUtil.h"
#include <dxe.h>

namespace gm {

    gmGameStopUIManager::gmGameStopUIManager(
        std::function<void()> onResume,
        std::function<void()> onRestartConfirmed,
        std::function<void()> onReturnToStartConfirmed,
        std::function<void()> onReturnToTitleConfirmed)
    {
        pauseMenu_ = std::make_unique<gmPauseMenuUI>(
            std::move(onResume),
            std::move(onRestartConfirmed),
            std::move(onReturnToStartConfirmed),
            std::move(onReturnToTitleConfirmed));
    }

    gmGameStopUIManager::~gmGameStopUIManager() = default;

    void gmGameStopUIManager::openPauseMenu()
    {
        pauseMenuOpen_ = true;

        // 一時停止中は、Alt(カーソルモード)を押していなくても即座にメニュー操作できるようにする
        // (ゲームプレイ系のupdate()自体をこの間スキップするため、cameraController_による
        //  カーソル非表示/ロックの毎フレーム上書きも同時に止まる)。
        dxe::SetVisibleMousePointer(true);
        gmCursorUtil::UnlockCursorFromWindow();
    }

    void gmGameStopUIManager::closePauseMenu()
    {
        pauseMenuOpen_ = false;
    }

    void gmGameStopUIManager::togglePauseMenu()
    {
        if (pauseMenuOpen_) {
            closePauseMenu();
        }
        else {
            openPauseMenu();
        }
    }

    void gmGameStopUIManager::update(float dt)
    {
        if (!pauseMenuOpen_) return;
        if (pauseMenu_) pauseMenu_->update(dt);
    }

    void gmGameStopUIManager::draw()
    {
        if (!pauseMenuOpen_) return;
        if (pauseMenu_) pauseMenu_->draw();
    }

}
