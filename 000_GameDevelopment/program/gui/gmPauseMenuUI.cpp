// gmPauseMenuUI.cpp
#include "gmPauseMenuUI.h"
#include "gmUIStrings.h"
#include "../util/gmFontCache.h"
#include "../../ResourceConstantHedder.h"
#include <DxLib.h>

namespace gm {

    gmPauseMenuUI::gmPauseMenuUI(
        std::function<void()> onResume,
        std::function<void()> onRestartConfirmed,
        std::function<void()> onReturnToStartConfirmed,
        std::function<void()> onReturnToTitleConfirmed)
        : gmUIObjectBase({ 0.0f, 0.0f }) // 画面中央基準で毎フレーム計算するため、position_自体は使わない
        , onResume_(std::move(onResume))
        , onRestartConfirmed_(std::move(onRestartConfirmed))
        , onReturnToStartConfirmed_(std::move(onReturnToStartConfirmed))
        , onReturnToTitleConfirmed_(std::move(onReturnToTitleConfirmed))
    {
        hPanel_ = LoadGraph("resource/graphics/menu/menu_panel_320x478.png");

        panelTopLeft_ = {
            DXE_WINDOW_WIDTH_F * 0.5f - PANEL_WIDTH * 0.5f,
            DXE_WINDOW_HEIGHT_F * 0.5f - PANEL_HEIGHT * 0.5f
        };

        Shared<dxe::FontTextResouce> titleFontResource = gmFontCache::GetOrCreate(
            TITLE_FONT_SIZE, FONT_NAME_SAWARABI_GOTHIC, FILE_PATH_TTF_SAWARABIGOTHIC_REGULAR);
        Shared<dxe::FontTextResouce> buttonFontResource = gmFontCache::GetOrCreate(
            BUTTON_FONT_SIZE, FONT_NAME_SAWARABI_GOTHIC, FILE_PATH_TTF_SAWARABIGOTHIC_REGULAR);

        titleText_ = dxe::FontText::Create(titleFontResource);
        titleText_->setString(UIStrings::PAUSE_MENU_TITLE);
        titleText_->setLocation(dxe::eRectOrigin::CENTER);
        titleText_->setColor(dxe::Colors::White);
        titleText_->setEdgeColor(dxe::Colors::Black);
        titleText_->setPosition({ panelTopLeft_.x + PANEL_WIDTH * 0.5f, panelTopLeft_.y + BUTTON_TOP_MARGIN * 0.5f });

        const float buttonX = panelTopLeft_.x + PANEL_WIDTH * 0.5f - BUTTON_WIDTH * 0.5f;
        const float row0Y = panelTopLeft_.y + BUTTON_TOP_MARGIN;
        const float row1Y = row0Y + (BUTTON_HEIGHT + BUTTON_GAP) * 1.0f;
        const float row2Y = row0Y + (BUTTON_HEIGHT + BUTTON_GAP) * 2.0f;
        const float row3Y = row0Y + (BUTTON_HEIGHT + BUTTON_GAP) * 3.0f;

        auto makeMenuButton = [&](float y, std::function<void()> onClick) {
            return std::make_unique<gmUIImageButton>(
                tnl::Vector2f{ buttonX, y }, BUTTON_WIDTH, BUTTON_HEIGHT,
                "resource/graphics/menu/btn_panel_normal_260x48.png",
                "resource/graphics/menu/btn_panel_hover_260x48.png",
                "resource/graphics/menu/btn_panel_press_260x48.png",
                std::move(onClick));
        };

        resumeButton_ = makeMenuButton(row0Y, [this]() {
            if (onResume_) onResume_();
        });

        restartButton_ = makeMenuButton(row1Y, [this]() {
            confirmDialog_.show(
                UIStrings::CONFIRM_RESTART,
                [this]() { if (onRestartConfirmed_) onRestartConfirmed_(); });
        });

        returnToStartButton_ = makeMenuButton(row2Y, [this]() {
            confirmDialog_.show(
                UIStrings::CONFIRM_RETURN_TO_START,
                [this]() { if (onReturnToStartConfirmed_) onReturnToStartConfirmed_(); });
        });

        returnToTitleButton_ = makeMenuButton(row3Y, [this]() {
            confirmDialog_.show(
                UIStrings::CONFIRM_RETURN_TO_TITLE,
                [this]() { if (onReturnToTitleConfirmed_) onReturnToTitleConfirmed_(); });
        });

        auto makeButtonLabel = [&](const char* label, float y) {
            Shared<dxe::FontText> text = dxe::FontText::Create(buttonFontResource);
            text->setString(label);
            text->setLocation(dxe::eRectOrigin::CENTER);
            text->setColor(dxe::Colors::White);
            text->setEdgeColor(dxe::Colors::Black);
            text->setPosition({ buttonX + BUTTON_WIDTH * 0.5f, y + BUTTON_HEIGHT * 0.5f });
            return text;
        };

        resumeLabel_        = makeButtonLabel(UIStrings::PAUSE_MENU_RESUME, row0Y);
        restartLabel_       = makeButtonLabel(UIStrings::PAUSE_MENU_RESTART, row1Y);
        returnToStartLabel_ = makeButtonLabel(UIStrings::PAUSE_MENU_RETURN_TO_START, row2Y);
        returnToTitleLabel_ = makeButtonLabel(UIStrings::PAUSE_MENU_RETURN_TO_TITLE, row3Y);
    }

    void gmPauseMenuUI::update(float dt)
    {
        // 確認ダイアログ表示中は、背後の4ボタンは反応させない(二重操作防止)。
        // ダイアログ自体の更新はここで行う。
        confirmDialog_.update(dt);

        if (confirmDialog_.isVisible()) {
            return;
        }

        if (resumeButton_)         { resumeButton_->setInputEnabled(true);         resumeButton_->update(dt); }
        if (restartButton_)        { restartButton_->setInputEnabled(true);        restartButton_->update(dt); }
        if (returnToStartButton_)  { returnToStartButton_->setInputEnabled(true);  returnToStartButton_->update(dt); }
        if (returnToTitleButton_)  { returnToTitleButton_->setInputEnabled(true);  returnToTitleButton_->update(dt); }
    }

    void gmPauseMenuUI::draw()
    {
        if (hPanel_ >= 0) {
            DrawGraph(static_cast<int>(panelTopLeft_.x), static_cast<int>(panelTopLeft_.y), hPanel_, TRUE);
        }
        titleText_->draw();

        if (resumeButton_)        resumeButton_->draw();
        if (restartButton_)       restartButton_->draw();
        if (returnToStartButton_) returnToStartButton_->draw();
        if (returnToTitleButton_) returnToTitleButton_->draw();

        resumeLabel_->draw();
        restartLabel_->draw();
        returnToStartLabel_->draw();
        returnToTitleLabel_->draw();

        // 確認ダイアログは最前面(4ボタンの上)に描く
        confirmDialog_.draw();
    }

}
