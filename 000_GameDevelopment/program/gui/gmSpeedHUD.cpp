// gmSpeedHUD.cpp
#include "gmSpeedHUD.h"
#include "../object/gmPlayerShip.h"
#include "../util/gmFontCache.h"
#include "../../ResourceConstantHedder.h"
#include <DxLib.h>
#include <cstdio>
#include <cmath>

namespace gm {

    gmSpeedHUD::gmSpeedHUD(std::shared_ptr<gmPlayerShip> playerShip)
        : gmUIObjectBase({ PANEL_LEFT_MARGIN, DXE_WINDOW_HEIGHT_F - PANEL_BOTTOM_MARGIN - ROW_HEIGHT * 7.0f })
        , playerShip_(std::move(playerShip))
    {
        panelTopLeft_ = position_;

        // ラベルは金額/経験値のような「目立たせたい数字」ではなく実用的な計器表示のため、
        // SawarabiGothicで統一する(フォントの使い分け方針はhud_spec参照)。
        Shared<dxe::FontTextResouce> labelFontResource = gmFontCache::GetOrCreate(
            LABEL_FONT_SIZE, FONT_NAME_CORPORATE_LOGO_ROUNDED_VER3, FILE_PATH_OTF_CORPORATE_LOGO_ROUNDED_BOLD_VER3);

        labelTexts_.reserve(7);
        for (int row = 0; row < 7; ++row) {
            // rowは表示順(0が一番上=添字6/FULL)。ラベル文字列自体はSPEED_LABELS[levelIndex]を使う。
            const int levelIndex = 6 - row;

            Shared<dxe::FontText> text = dxe::FontText::Create(labelFontResource);
            text->setString(SPEED_LABELS[levelIndex]);
            text->setLocation(dxe::eRectOrigin::CENTER);
            text->setPosition({
                panelTopLeft_.x + ROW_WIDTH * 0.5f,
                panelTopLeft_.y + ROW_HEIGHT * static_cast<float>(row) + ROW_HEIGHT * 0.5f
            });
            // 色は選択状態に応じてupdate()で毎フレーム設定するため、ここでは初期値(未選択色)だけ入れておく
            text->setColor(dxe::Colors::White);
            text->setEdgeColor(dxe::Colors::Black);

            labelTexts_.push_back(text);
        }

        speedValueText_ = dxe::FontText::Create(labelFontResource);
        speedValueText_->setLocation(dxe::eRectOrigin::LEFT_CENTER);
        speedValueText_->setColor(dxe::Colors::White);
        speedValueText_->setEdgeColor(dxe::Colors::Black);
    }

    void gmSpeedHUD::update(float dt)
    {
        auto playerShip = playerShip_.lock();
        if (!playerShip) return;

        const int currentSpeedIndex = playerShip->getSpeedIndex();

        for (int row = 0; row < 7; ++row) {
            const int levelIndex = 6 - row;
            const bool selected = (levelIndex == currentSpeedIndex);

            // 選択行は白背景に反転(黒文字+白縁)、非選択行は白文字+黒縁(draw()側で背景を塗る)
            if (selected) {
                labelTexts_[row]->setColor(dxe::Colors::Black);
                labelTexts_[row]->setEdgeColor(dxe::Colors::White);
            }
            else {
                labelTexts_[row]->setColor(dxe::Colors::White);
                labelTexts_[row]->setEdgeColor(dxe::Colors::Black);
            }
        }

        // ---- 実速度(演出用kts表示)。選択行と同じ高さ、パネルの右側に表示する ----
        const float currentSpeed = playerShip->getDynamics().speed;
        const float displayValue = std::fabs(currentSpeed) * SPEED_DISPLAY_KTS_SCALE;

        char buf[32];
        std::snprintf(buf, sizeof(buf), "< %.1fkt", displayValue);
        speedValueText_->setString(buf);

        const int selectedRow = 6 - currentSpeedIndex;
        speedValueText_->setPosition({
            panelTopLeft_.x + ROW_WIDTH + SPEED_VALUE_GAP,
            panelTopLeft_.y + ROW_HEIGHT * static_cast<float>(selectedRow) + ROW_HEIGHT * 0.5f
        });
    }

    void gmSpeedHUD::draw()
    {
        auto playerShip = playerShip_.lock();
        if (!playerShip) return;

        const int currentSpeedIndex = playerShip->getSpeedIndex();

        for (int row = 0; row < 7; ++row) {
            const int levelIndex = 6 - row;
            const bool selected = (levelIndex == currentSpeedIndex);

            const int top    = static_cast<int>(panelTopLeft_.y + ROW_HEIGHT * static_cast<float>(row));
            const int bottom = static_cast<int>(top + ROW_HEIGHT);
            const int left   = static_cast<int>(panelTopLeft_.x);
            const int right  = static_cast<int>(left + ROW_WIDTH);

            if (selected) {
                DrawBox(left, top, right, bottom, dxe::Colors::White, TRUE);
            }

            labelTexts_[row]->draw();
        }

        speedValueText_->draw();
    }

}
