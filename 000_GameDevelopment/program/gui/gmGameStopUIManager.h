// gmGameStopUIManager.h
// 「ゲームプレイを一時停止して表示するUI」をまとめる薄い中間層。
//
// Note:
// 現状はポーズメニュー(gmPauseMenuUI)のみを持つが、将来的に成長要素UI等の
// 同じく「ゲームを止めて表示する」UIが増えた際の受け皿として、gmGameSceneと
// 個々のUIクラスの間にこの層を挟んでいる。
//
// gmGameScene側は isGameStopped() を毎フレーム見て、trueの間はゲームプレイ系の
// update処理(船・NPC・衝突判定・武器入力等)を丸ごとスキップする。
#pragma once
#include <memory>
#include <functional>

namespace gm {

    class gmPauseMenuUI;

    class gmGameStopUIManager {
    public:
        // arg1... Resumeが選ばれた時のコールバック
        // arg2... Restartが確認(Yes)された時のコールバック
        // arg3... Return to Startが確認(Yes)された時のコールバック
        // arg4... Return to Titleが確認(Yes)された時のコールバック
        gmGameStopUIManager(
            std::function<void()> onResume,
            std::function<void()> onRestartConfirmed,
            std::function<void()> onReturnToStartConfirmed,
            std::function<void()> onReturnToTitleConfirmed);
        ~gmGameStopUIManager();

        // 現在、何らかの理由でゲームが一時停止中かどうか。
        bool isGameStopped() const { return pauseMenuOpen_; }

        void openPauseMenu();
        void closePauseMenu();
        void togglePauseMenu();

        void update(float dt);
        void draw();

    private:
        std::unique_ptr<gmPauseMenuUI> pauseMenu_;
        bool pauseMenuOpen_ = false;
    };

}
