// gmTopBarUI.cpp
#include "gmTopBarUI.h"
#include "../wallet/gmWallet.h"
#include "../util/gmFontCache.h"
#include "../../ResourceConstantHedder.h"
#include <DxLib.h>

namespace gm {

    gmTopBarUI::gmTopBarUI(std::shared_ptr<gmWallet> wallet, std::function<void()> onMenuClick)
        : gmUIObjectBase({ 0.0f, 0.0f }) // 画面上端にパディング無しでぴったり配置
        , wallet_(std::move(wallet))
    {
        hBackground_ = LoadGraph("resource/graphics/hud/ic_topbar_bg.png");
        hMoneyIcon_  = LoadGraph("resource/graphics/hud/ic_icon_money_48.png");
        hExpIcon_    = LoadGraph("resource/graphics/hud/ic_icon_exp_48.png");

        // ---- 資金/経験値の数値表示用フォント ----
        // 太めのCorporate Logo Rounded Boldで、パッと目に入るように強調する
        //
        // フォントリソースの取得は gmFontCache 経由にすることで、
        // リソースを重複生成せず使い回す。
        // フォント名(FONT_NAME_CORPORATE_LOGO_ROUNDED_VER3)・ファイルパス
        // (FILE_PATH_OTF_CORPORATE_LOGO_ROUNDED_BOLD_VER3)は、いずれもResourceConstantHedder.hに
        // 既に用意されているものをそのまま使う。
        Shared<dxe::FontTextResouce> valueFontResource = gmFontCache::GetOrCreate(
            VALUE_FONT_SIZE, FONT_NAME_CORPORATE_LOGO_ROUNDED_VER3, FILE_PATH_OTF_CORPORATE_LOGO_ROUNDED_BOLD_VER3);


        moneyText_ = dxe::FontText::Create(valueFontResource);
        moneyText_->setLocation(dxe::eRectOrigin::RIGHT_CENTER); // 右寄せ基準にすることで、桁数が変わっても右端(=読み取り位置)がブレない
        moneyText_->setColor(dxe::Colors::White);
        moneyText_->setEdgeColor(dxe::Colors::Black);

        expText_ = dxe::FontText::Create(valueFontResource);
        expText_->setLocation(dxe::eRectOrigin::RIGHT_CENTER);
        expText_->setColor(dxe::Colors::White);
        expText_->setEdgeColor(dxe::Colors::Black);

        // ---- レイアウト computation ----
        const float moneyIconX = LEFT_MARGIN;
        const float moneyValueSlotRight = moneyIconX + MONEY_ICON_SIZE + ICON_TO_VALUE_GAP + VALUE_SLOT_WIDTH;
        const float expIconX = moneyValueSlotRight + VALUE_TO_NEXT_GAP;
        const float expValueSlotRight = expIconX + EXP_ICON_SIZE + ICON_TO_VALUE_GAP + VALUE_SLOT_WIDTH;

        moneyIconPos_ = { moneyIconX, (BAR_HEIGHT - MONEY_ICON_SIZE) * 0.5f };
        expIconPos_   = { expIconX,   (BAR_HEIGHT - EXP_ICON_SIZE) * 0.5f };

        moneyText_->setPosition({ moneyValueSlotRight, BAR_HEIGHT * 0.5f });
        expText_->setPosition({ expValueSlotRight, BAR_HEIGHT * 0.5f });

        const tnl::Vector2f menuBtnPos = {
            BAR_WIDTH - MENU_BUTTON_MARGIN - MENU_BUTTON_SIZE,
            MENU_BUTTON_MARGIN
        };
        menuButton_ = std::make_unique<gmUIImageButton>(
            menuBtnPos, MENU_BUTTON_SIZE, MENU_BUTTON_SIZE,
            "resource/graphics/hud/ic_btn_menu_normal_48.png",
            "resource/graphics/hud/ic_btn_menu_hover_48.png",
            "resource/graphics/hud/ic_btn_menu_pressed_48.png",
            std::move(onMenuClick));
    }

    void gmTopBarUI::update(float dt)
    {
        if (auto wallet = wallet_.lock()) {
            moneyText_->setString(FormatWithCommas(wallet->getFunds()));
            expText_->setString(FormatWithCommas(wallet->getMeltExpAsInt()));
        }

        if (menuButton_) {
            menuButton_->setInputEnabled(cursorModeActive_);
            menuButton_->update(dt);
        }
    }

    void gmTopBarUI::draw()
    {
        DrawExtendGraph(0, 0, static_cast<int>(BAR_WIDTH), static_cast<int>(BAR_HEIGHT), hBackground_, TRUE);

        DrawExtendGraph(
            static_cast<int>(moneyIconPos_.x), static_cast<int>(moneyIconPos_.y),
            static_cast<int>(moneyIconPos_.x + MONEY_ICON_SIZE), static_cast<int>(moneyIconPos_.y + MONEY_ICON_SIZE),
            hMoneyIcon_, TRUE);

        DrawExtendGraph(
            static_cast<int>(expIconPos_.x), static_cast<int>(expIconPos_.y),
            static_cast<int>(expIconPos_.x + EXP_ICON_SIZE), static_cast<int>(expIconPos_.y + EXP_ICON_SIZE),
            hExpIcon_, TRUE);

        moneyText_->draw();
        expText_->draw();

        if (menuButton_) {
            menuButton_->draw();
        }
    }

    // 例: 128450 -> "128,450"。3桁ごとにカンマを挿入するだけの単純な実装。
    std::string gmTopBarUI::FormatWithCommas(int value)
    {
        const bool negative = value < 0;
        long long absValue = negative ? -static_cast<long long>(value) : static_cast<long long>(value);
        std::string digits = std::to_string(absValue);

        std::string result;
        int count = 0;
        for (auto it = digits.rbegin(); it != digits.rend(); ++it) {
            if (count != 0 && count % 3 == 0) {
                result.insert(result.begin(), ',');
            }
            result.insert(result.begin(), *it);
            count++;
        }
        if (negative) {
            result.insert(result.begin(), '-');
        }
        return result;
    }

}
