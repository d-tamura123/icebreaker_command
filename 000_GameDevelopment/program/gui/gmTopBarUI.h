// gmTopBarUI.h
// 「画面上端バー」。
// 背景(帯画像)+ 資金/溶かす経験値の数値表示 + メインメニューボタン。
#pragma once
#include "gmUIObjectBase.h"
#include "gmUIImageButton.h"
#include <dxe.h>
#include <memory>
#include <functional>
#include <string>

namespace gm {

    class gmWallet;

    class gmTopBarUI : public gmUIObjectBase {
    public:
        // arg1... 資金/経験値の参照元
        // arg2... メニューボタンのクリック時コールバック
        gmTopBarUI(std::shared_ptr<gmWallet> wallet, std::function<void()> onMenuClick);

        // arg1... カーソルモード(Alt押下中)かどうか。メニューボタンのホバー/クリック判定の有効/無効に使う
        void setCursorModeActive(bool active) { cursorModeActive_ = active; }

        void update(float dt) override;
        void draw() override;

    private:
        static std::string FormatWithCommas(int value);

        // ---- レイアウト定数(値の意味はコメント参照。座標決めの経緯はhud_spec参照) ----
        static constexpr float BAR_WIDTH  = 1280.0f;    // 画面幅ぴったり(ic_topbar_bg.pngの実サイズに合わせる)
        //static constexpr float BAR_HEIGHT = 56.0f;      // 帯の高さ(ic_topbar_bg.pngの実サイズに合わせる)
        static constexpr float BAR_HEIGHT = 32.0f;      // 帯の高さ(ic_topbar_bg.pngの実サイズに合わせる)

        static constexpr float MENU_BUTTON_SIZE   = 32.0f;
        static constexpr float MENU_BUTTON_MARGIN = 0.0f;   // (BAR_HEIGHT - MENU_BUTTON_SIZE) / 2、右・上ともに同じ余白

        static constexpr float MONEY_ICON_SIZE = 32.0f;
        static constexpr float EXP_ICON_SIZE   = 32.0f;

        static constexpr float LEFT_MARGIN       = 64.0f;   // 左端 〜 資金アイコンまでの固定余白
        static constexpr float ICON_TO_VALUE_GAP = 8.0f;    // アイコン 〜 数値表示欄の余白
        static constexpr float VALUE_SLOT_WIDTH  = 140.0f;  // 数値表示欄の確保幅(8桁程度を想定した目安値。右寄せの基準にする)
        static constexpr float VALUE_TO_NEXT_GAP = 16.0f;   // 資金の数値表示欄 〜 経験値アイコンの余白

        static constexpr int32_t VALUE_FONT_SIZE = 22;      // 資金/経験値の数値フォントサイズ(暫定値。バー高さの4割程度)

        std::weak_ptr<gmWallet> wallet_;

        int hBackground_ = -1;
        int hMoneyIcon_  = -1;
        int hExpIcon_    = -1;

        tnl::Vector2f moneyIconPos_;
        tnl::Vector2f expIconPos_;

        Shared<dxe::FontText> moneyText_;
        Shared<dxe::FontText> expText_;

        std::unique_ptr<gmUIImageButton> menuButton_;

        bool cursorModeActive_ = false;
    };

}
