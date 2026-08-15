// gmTitleScene.cpp
#include "gmTitleScene.h"
#include "gmGameScene.h"
#include "../gmSceneManager.h"
#include <DxLib.h>

namespace gm {

    bool gmTitleScene::s_hasPlayedIntroVideo = false;

    void gmTitleScene::onEnter(std::shared_ptr<gmSceneManager> manager)
    {
        sceneManager_ = manager;
        // ---- オープニング動画(アプリケーション全体を通して初回のみ) ----
        // DxLibの動画関連関数(PlayMovieToGraph/GetMovieStateToGraph等)を使う。
        // 既に1回再生済み、またはLoadGraphが失敗した場合(-1)は、動画無しでいきなりメニューへ進む
        // (動画ファイルが無い/読み込みに失敗しても遊べなくなることは避ける)。
        if (s_hasPlayedIntroVideo) {
            phase_ = Phase::Menu;
        }
        else {
            phase_ = Phase::Video;
            hMovie_ = LoadGraph("resource/video/IC_TITLE_VIDEO.mp4");
            if (hMovie_ >= 0) {
                PlayMovieToGraph(hMovie_);
                s_hasPlayedIntroVideo = true;
            }
            else {
                phase_ = Phase::Menu;
            }
        }

        // ---- 動画→メニューの暗転フェード(シーン切り替えと同じgmFadeTransitionEffectを流用) ----
        videoToMenuFade_ = std::make_shared<gmFadeTransitionEffect>();
        videoToMenuFade_->setScreenSize(DXE_WINDOW_WIDTH, DXE_WINDOW_HEIGHT);
        videoToMenuFade_->setFadeColor(0, 0, 0);
        videoTransitionActive_ = false;

        // ---- タイトル背景(1280x720。ウィンドウサイズと一致するため拡縮不要) ----
        hTitleBg_ = LoadGraph("resource/graphics/title/ic_game_title.png");

        // ---- Start/Exitボタン(高さ基準・アスペクト比を保ったまま縮小配置) ----
        const float startWidth = BUTTON_HEIGHT * (START_BUTTON_NATIVE_WIDTH / START_BUTTON_NATIVE_HEIGHT);
        const float exitWidth  = BUTTON_HEIGHT * (EXIT_BUTTON_NATIVE_WIDTH / EXIT_BUTTON_NATIVE_HEIGHT);

        const float startX = DXE_WINDOW_WIDTH_F - BUTTON_RIGHT_MARGIN - startWidth;
        const float exitX  = DXE_WINDOW_WIDTH_F - BUTTON_RIGHT_MARGIN - exitWidth;
        const float exitY  = DXE_WINDOW_HEIGHT_F - BUTTON_BOTTOM_MARGIN - BUTTON_HEIGHT;
        const float startY = exitY - BUTTON_GAP - BUTTON_HEIGHT;

        startButton_ = std::make_unique<gmUIImageButton>(
            tnl::Vector2f{ startX, startY }, startWidth, BUTTON_HEIGHT,
            "resource/graphics/title/btn_normal_start.png",
            "resource/graphics/title/btn_hover_start_fixed.png",
            "resource/graphics/title/btn_press_start_fixed.png",
            [this]() {
                if (sceneManager_) {
                    sceneManager_->requestSceneChange(std::make_shared<gmGameScene>());
                }
            });

        exitButton_ = std::make_unique<gmUIImageButton>(
            tnl::Vector2f{ exitX, exitY }, exitWidth, BUTTON_HEIGHT,
            "resource/graphics/title/btn_normal_exit.png",
            "resource/graphics/title/btn_hover_exit_fixed.png",
            "resource/graphics/title/btn_press_exit_fixed.png",
            []() {
                // dxeにアプリケーション終了を任せる
                dxe::ExitApplication();
            });

        // タイトル画面には「カーソルモード」の概念(ゲームプレイ中限定)が無いため、常時反応してよい
        startButton_->setInputEnabled(true);
        exitButton_->setInputEnabled(true);

        // ゲームシーンから戻ってきた場合等を考慮し、カーソルは念のため明示的に表示しておく
        // (gmGameScene::onExit()側で既に表示に戻しているはずだが、二重の保険として)
        dxe::SetVisibleMousePointer(true);
    }

    void gmTitleScene::update()
    {
        const float dt = dxe::GetDeltaTime();

        // 動画→メニューのフェード進行中は、他の判定(動画終了判定・ボタン操作)より優先して
        // フェードの経過だけを進める。phase_の切り替え自体はフェードのコールバック
        // (fadeOutIn()の引数。暗転しきったタイミングで呼ばれる)側で行う。
        if (videoTransitionActive_) {
            videoToMenuFade_->update(dt);
            if (videoToMenuFade_->isFinished()) {
                videoTransitionActive_ = false;
            }
            return;
        }

        switch (phase_) {
        case Phase::Video: {
            const int state = (hMovie_ >= 0) ? GetMovieStateToGraph(hMovie_) : 0;
            if (state != 1 || isSkipInputPressed()) {
                // 動画が自然終了、または操作によるスキップ。暗転フェードを開始する。
                // 動画ハンドルの破棄(DxLibに専用の「停止」関数は無いため、描画・監視を
                // やめた上でDeleteGraph()するだけでよい)は、暗転しきった瞬間(コールバック内)で行う
                // (フェードアウトの最中は最後の動画フレームがそのまま透けて見えても問題ないため)。
                //
                // また、インゲームからタイトル画面へ戻る際に、シーン遷移側のフェード処理と
                // 動画終了時のフェード処理が重複して発生する可能性がある。
                // そのため、動画終了時の処理としてフェード処理を組み込み、
                // シーン遷移フェードと競合しないようにしている。
                videoTransitionActive_ = true;
                videoToMenuFade_->fadeOutIn([this]() {
                    if (hMovie_ >= 0) {
                        DeleteGraph(hMovie_);
                        hMovie_ = -1;
                    }
                    phase_ = Phase::Menu;
                    });
            }
            break;
        }
        case Phase::Menu: {
            if (startButton_) startButton_->update(dt);
            if (exitButton_)  exitButton_->update(dt);
            break;
        }
        }
    }

    void gmTitleScene::draw()
    {
        switch (phase_) {
            case Phase::Video: {
                if (hMovie_ >= 0) {
                    DrawExtendGraph(0, 0, DXE_WINDOW_WIDTH, DXE_WINDOW_HEIGHT, hMovie_, FALSE);
                }
                break;
            }
            case Phase::Menu: {
                if (hTitleBg_ >= 0) {
                    DrawGraph(0, 0, hTitleBg_, TRUE);
                }
                if (startButton_) startButton_->draw();
                if (exitButton_)  exitButton_->draw();
                break;
            }
        }

        // フェードは常に最前面に重ねて描く(動画→タイトル画像、どちらの上にも乗る必要があるため)
        if (videoTransitionActive_) {
            videoToMenuFade_->draw();
        }
    }

    void gmTitleScene::onExit()
    {
        if (hMovie_ >= 0) {
            DeleteGraph(hMovie_);
            hMovie_ = -1;
        }
        if (hTitleBg_ >= 0) {
            DeleteGraph(hTitleBg_);
            hTitleBg_ = -1;
        }
    }

    bool gmTitleScene::isSkipInputPressed() const
    {
        if (tnl::Input::IsMouseTrigger(tnl::Input::eMouseTrigger::IN_LEFT)) return true;
        if (tnl::Input::IsKeyDownTrigger(tnl::Input::eKeys::KB_SPACE))      return true;
        if (tnl::Input::IsKeyDownTrigger(tnl::Input::eKeys::KB_RETURN))    return true;
        if (tnl::Input::IsKeyDownTrigger(tnl::Input::eKeys::KB_ESCAPE))    return true;
        return false;
    }

}
