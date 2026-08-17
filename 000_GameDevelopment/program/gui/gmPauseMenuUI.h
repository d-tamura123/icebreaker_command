// gmPauseMenuUI.h
// Escキー/トップバーのメニューボタンで開くポーズメニュー。
//
// 項目: Resume(即時) / Restart・Return to Start・Return to Title(いずれもYes/No確認を挟む)。
// 実際のシーン切り替え・リスポーン処理等は一切知らず、コンストラクタで渡されたコールバックを
// 呼ぶだけに徹する(呼び出し元(gmGameStopUIManager経由でgmGameScene)が実処理を持つ)。
#pragma once
#include "gmUIObjectBase.h"
#include "gmUIImageButton.h"
#include "gmConfirmDialogUI.h"
#include <dxe.h>
#include <memory>
#include <functional>

namespace gm {

    class gmPauseMenuUI : public gmUIObjectBase {
    public:
        // arg1... Resumeが選ばれた時のコールバック
        // arg2... Restartが確認(Yes)された時のコールバック
        // arg3... Return to Startが確認(Yes)された時のコールバック
        // arg4... Return to Titleが確認(Yes)された時のコールバック
        gmPauseMenuUI(
            std::function<void()> onResume,
            std::function<void()> onRestartConfirmed,
            std::function<void()> onReturnToStartConfirmed,
            std::function<void()> onReturnToTitleConfirmed);

        void update(float dt) override;
        void draw() override;

    private:
        // ---- レイアウト定数 ----
        static constexpr float PANEL_WIDTH  = 320.0f; // menu_panel_320x478.pngの実サイズ
        static constexpr float PANEL_HEIGHT = 478.0f;

        static constexpr float BUTTON_WIDTH  = 260.0f; // btn_panel_*_260x48.pngの実サイズ
        static constexpr float BUTTON_HEIGHT = 48.0f;
        static constexpr float BUTTON_GAP        = 16.0f; // ボタン間の余白
        static constexpr float BUTTON_TOP_MARGIN = 70.0f; // パネル上端 〜 最初のボタンまでの余白(タイトルラベル分)

        static constexpr int32_t TITLE_FONT_SIZE  = 22;
        static constexpr int32_t BUTTON_FONT_SIZE = 18;

        int hPanel_ = -1;

        std::unique_ptr<gmUIImageButton> resumeButton_;
        std::unique_ptr<gmUIImageButton> restartButton_;
        std::unique_ptr<gmUIImageButton> returnToStartButton_;
        std::unique_ptr<gmUIImageButton> returnToTitleButton_;

        Shared<dxe::FontText> titleText_;
        Shared<dxe::FontText> resumeLabel_;
        Shared<dxe::FontText> restartLabel_;
        Shared<dxe::FontText> returnToStartLabel_;
        Shared<dxe::FontText> returnToTitleLabel_;

        gmConfirmDialogUI confirmDialog_;

        tnl::Vector2f panelTopLeft_;

        std::function<void()> onResume_;
        std::function<void()> onRestartConfirmed_;
        std::function<void()> onReturnToStartConfirmed_;
        std::function<void()> onReturnToTitleConfirmed_;
    };

}
