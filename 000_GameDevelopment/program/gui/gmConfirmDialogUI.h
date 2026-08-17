// gmConfirmDialogUI.h
// 汎用のYes/No確認ダイアログ。「本当によろしいですか?」的な破壊的操作の前に挟む、
// 独立した再利用可能クラス。
//
// show()で表示内容(メッセージ・コールバック)を渡して表示を開始する。
// isVisible()の間だけupdate()/draw()の呼び出し元(親UI)が呼べば良い、という運用は必須ではなく、
// 非表示時にupdate()/draw()を呼んでも何もしない(内部でisVisible()を見ている)。
#pragma once
#include "gmUIObjectBase.h"
#include "gmUIImageButton.h"
#include <dxe.h>
#include <memory>
#include <functional>
#include <string>

namespace gm {

    class gmConfirmDialogUI : public gmUIObjectBase {
    public:
        gmConfirmDialogUI();

        // arg1... 確認メッセージ(1行想定。英語ラベル想定)
        // arg2... Yesが押された時のコールバック
        // arg3... Noが押された時のコールバック(省略可。閉じるだけで良い場合はnullptrのままでよい)
        void show(const std::string& message, std::function<void()> onYes, std::function<void()> onNo = nullptr);

        bool isVisible() const { return visible_; }

        void update(float dt) override;
        void draw() override;

    private:
        void handleYesClicked();
        void handleNoClicked();

        // ---- レイアウト定数 ----
        static constexpr float PANEL_WIDTH  = 320.0f; // msgbox_panel_320x150.pngの実サイズ
        static constexpr float PANEL_HEIGHT = 150.0f;

        static constexpr float BUTTON_WIDTH  = 128.0f; // msgbox_btn_*_128x38.pngの実サイズ
        static constexpr float BUTTON_HEIGHT = 38.0f;
        static constexpr float BUTTON_GAP           = 24.0f; // Yes〜No間の余白
        static constexpr float BUTTON_BOTTOM_MARGIN = 20.0f; // パネル下端からの余白

        static constexpr int32_t MESSAGE_FONT_SIZE = 18;

        static constexpr int DIM_OVERLAY_ALPHA = 150; // 背後を暗くする全画面オーバーレイの濃さ(0〜255)

        int hPanel_ = -1;

        std::unique_ptr<gmUIImageButton> yesButton_;
        std::unique_ptr<gmUIImageButton> noButton_;

        Shared<dxe::FontText> messageText_;
        Shared<dxe::FontText> yesLabel_;
        Shared<dxe::FontText> noLabel_;

        bool visible_ = false;

        tnl::Vector2f panelTopLeft_;

        std::function<void()> onYes_;
        std::function<void()> onNo_;
    };

}
