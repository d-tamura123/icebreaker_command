// gmTitleScene.h
// タイトルシーン。
//
// 流れ: オープニング動画再生(何か操作があればスキップ) → 暗転フェード → タイトル画像+メニュー(Start/Exit)。
// Startでゲームシーンへ、Exitでアプリケーション終了(dxe::ExitApplication())。
//
// オープニング動画は、アプリケーション全体を通して初回のみ再生する(static変数で管理)。
// 将来ポーズメニュー等からタイトルへ戻ってくる場合(未実装)は、動画を再生済みである前提のため、
// いきなりメニュー表示から始まる。
//
#pragma once
#include "../gmSceneBase.h"
#include "../gmFadeTransitionEffect.h"
#include "../../gui/gmUIImageButton.h"
#include <dxe.h>
#include <memory>

namespace gm {

    class gmTitleScene : public gmSceneBase {
    public:
        gmTitleScene() = default;
        ~gmTitleScene() override = default;

        void onEnter(std::shared_ptr<gmSceneManager> manager) override;
        void update() override;
        void draw() override;
        void onExit() override;

    private:
        enum class Phase {
            Video, // オープニング動画再生中(何か操作があればスキップ)。動画→メニューの暗転フェードもこの間に含む
            Menu,  // タイトル画像 + メニュー(Start/Exit)
        };

        // 動画再生中、スキップ操作(クリックまたは主要キー)があったかどうか
        bool isSkipInputPressed() const;

        // アプリケーション全体で、オープニング動画を既に1回再生したかどうか
        static bool s_hasPlayedIntroVideo;

        Phase phase_ = Phase::Video;

        int hMovie_   = -1;
        int hTitleBg_ = -1;

        // 動画終了/スキップ時、暗転してからメニューへ切り替えるためのフェード。
        // シーン切り替え(gmSceneManager)で使っているものと同じクラスをシーン内で流用する。
        std::shared_ptr<gmFadeTransitionEffect> videoToMenuFade_;
        bool videoTransitionActive_ = false;

        std::unique_ptr<gmUIImageButton> startButton_;
        std::unique_ptr<gmUIImageButton> exitButton_;

        // ---- レイアウト定数(暫定配置。座標決めの経緯はhud_spec参照) ----
        // ボタンは高さ基準(BUTTON_HEIGHT)でアスペクト比を保ったまま縮小して配置する。
        static constexpr float BUTTON_HEIGHT = 42.0f;

        static constexpr float START_BUTTON_NATIVE_WIDTH  = 758.0f; // btn_*_start.pngの実サイズ
        static constexpr float START_BUTTON_NATIVE_HEIGHT = 153.0f;
        static constexpr float EXIT_BUTTON_NATIVE_WIDTH   = 624.0f; // btn_*_exit.pngの実サイズ
        static constexpr float EXIT_BUTTON_NATIVE_HEIGHT  = 142.0f;

        static constexpr float BUTTON_RIGHT_MARGIN  = 40.0f; // 画面右端からの余白
        static constexpr float BUTTON_BOTTOM_MARGIN = 40.0f; // 画面下端(Exit側)からの余白
        static constexpr float BUTTON_GAP           = 16.0f; // Start〜Exitの間隔
    };

}
