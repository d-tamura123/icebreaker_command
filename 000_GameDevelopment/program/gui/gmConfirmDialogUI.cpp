// gmConfirmDialogUI.cpp
#include "gmConfirmDialogUI.h"
#include "gmUIStrings.h"
#include "../util/gmFontCache.h"
#include "../../ResourceConstantHedder.h"
#include <DxLib.h>

namespace gm {

    gmConfirmDialogUI::gmConfirmDialogUI()
        : gmUIObjectBase({ 0.0f, 0.0f }) // 画面中央基準で毎フレーム計算するため、position_自体は使わない
    {
        hPanel_ = LoadGraph("resource/graphics/menu/msgbox_panel_320x150.png");

        panelTopLeft_ = {
            DXE_WINDOW_WIDTH_F * 0.5f - PANEL_WIDTH * 0.5f,
            DXE_WINDOW_HEIGHT_F * 0.5f - PANEL_HEIGHT * 0.5f
        };

        Shared<dxe::FontTextResouce> fontResource = gmFontCache::GetOrCreate(
            MESSAGE_FONT_SIZE, FONT_NAME_SAWARABI_GOTHIC, FILE_PATH_TTF_SAWARABIGOTHIC_REGULAR);

        messageText_ = dxe::FontText::Create(fontResource);
        messageText_->setLocation(dxe::eRectOrigin::CENTER);
        messageText_->setColor(dxe::Colors::White);
        messageText_->setEdgeColor(dxe::Colors::Black);
        messageText_->setPosition({
            panelTopLeft_.x + PANEL_WIDTH * 0.5f,
            panelTopLeft_.y + PANEL_HEIGHT * 0.35f
        });

        const float buttonY = panelTopLeft_.y + PANEL_HEIGHT - BUTTON_BOTTOM_MARGIN - BUTTON_HEIGHT;
        const float yesX = panelTopLeft_.x + PANEL_WIDTH * 0.5f - BUTTON_GAP * 0.5f - BUTTON_WIDTH;
        const float noX  = panelTopLeft_.x + PANEL_WIDTH * 0.5f + BUTTON_GAP * 0.5f;

        yesButton_ = std::make_unique<gmUIImageButton>(
            tnl::Vector2f{ yesX, buttonY }, BUTTON_WIDTH, BUTTON_HEIGHT,
            "resource/graphics/menu/msgbox_btn_normal_128x38.png",
            "resource/graphics/menu/msgbox_btn_hover_128x38.png",
            "resource/graphics/menu/msgbox_btn_press_128x38.png",
            [this]() { handleYesClicked(); });

        noButton_ = std::make_unique<gmUIImageButton>(
            tnl::Vector2f{ noX, buttonY }, BUTTON_WIDTH, BUTTON_HEIGHT,
            "resource/graphics/menu/msgbox_btn_normal_128x38.png",
            "resource/graphics/menu/msgbox_btn_hover_128x38.png",
            "resource/graphics/menu/msgbox_btn_press_128x38.png",
            [this]() { handleNoClicked(); });

        auto makeButtonLabel = [&](const char* label, float btnX) {
            Shared<dxe::FontText> text = dxe::FontText::Create(fontResource);
            text->setString(label);
            text->setLocation(dxe::eRectOrigin::CENTER);
            text->setColor(dxe::Colors::White);
            text->setEdgeColor(dxe::Colors::Black);
            text->setPosition({ btnX + BUTTON_WIDTH * 0.5f, buttonY + BUTTON_HEIGHT * 0.5f });
            return text;
        };

        yesLabel_ = makeButtonLabel(UIStrings::CONFIRM_DIALOG_YES, yesX);
        noLabel_ = makeButtonLabel(UIStrings::CONFIRM_DIALOG_NO, noX);
    }

    void gmConfirmDialogUI::show(const std::string& message, std::function<void()> onYes, std::function<void()> onNo)
    {
        messageText_->setString(message);
        onYes_ = std::move(onYes);
        onNo_  = std::move(onNo);
        visible_ = true;
    }

    void gmConfirmDialogUI::update(float dt)
    {
        if (!visible_) return;

        if (yesButton_) { yesButton_->setInputEnabled(true); yesButton_->update(dt); }
        if (noButton_)  { noButton_->setInputEnabled(true);  noButton_->update(dt); }
    }

    void gmConfirmDialogUI::draw()
    {
        if (!visible_) return;

        // 背後を暗くする全画面オーバーレイ(親のパネル等より前に、このダイアログ自体より後ろに描く)
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, DIM_OVERLAY_ALPHA);
        DrawBox(0, 0, DXE_WINDOW_WIDTH, DXE_WINDOW_HEIGHT, dxe::Colors::Black, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        if (hPanel_ >= 0) {
            DrawGraph(static_cast<int>(panelTopLeft_.x), static_cast<int>(panelTopLeft_.y), hPanel_, TRUE);
        }
        messageText_->draw();

        if (yesButton_) yesButton_->draw();
        if (noButton_)  noButton_->draw();
        yesLabel_->draw();
        noLabel_->draw();
    }

    void gmConfirmDialogUI::handleYesClicked()
    {
        visible_ = false;
        if (onYes_) onYes_();
    }

    void gmConfirmDialogUI::handleNoClicked()
    {
        visible_ = false;
        if (onNo_) onNo_();
    }

}
