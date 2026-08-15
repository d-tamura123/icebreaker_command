// gmRudderHUD.cpp
#include "gmRudderHUD.h"
#include "../object/gmPlayerShip.h"
#include "../util/gmFontCache.h"
#include "../../ResourceConstantHedder.h"
#include <DxLib.h>
#include <algorithm>
#include <cmath>

namespace gm {

    gmRudderHUD::gmRudderHUD(std::shared_ptr<gmPlayerShip> playerShip)
        : gmUIObjectBase({
              PANEL_CENTER_X - COLUMN_WIDTH * 5.0f * 0.5f,
              PANEL_TOP_Y
          })
        , playerShip_(std::move(playerShip))
    {
        panelTopLeft_ = position_;

        // 速度HUDと同じくSawarabiGothicで統一する(フォントの使い分け方針はhud_spec参照)。
        // 既にgmSpeedHUD側で同じ(フォント名, サイズ)の組み合わせを使っていれば、
        // gmFontCache経由でリソースが共有され、再生成は発生しない。
        Shared<dxe::FontTextResouce> labelFontResource = gmFontCache::GetOrCreate(
            LABEL_FONT_SIZE, FONT_NAME_CORPORATE_LOGO_ROUNDED_VER3, FILE_PATH_OTF_CORPORATE_LOGO_ROUNDED_BOLD_VER3);

        labelTexts_.reserve(5);
        for (int i = 0; i < 5; ++i) {
            Shared<dxe::FontText> text = dxe::FontText::Create(labelFontResource);
            text->setString(RUDDER_LABELS[i]);
            text->setLocation(dxe::eRectOrigin::CENTER);
            text->setPosition({
                panelTopLeft_.x + COLUMN_WIDTH * static_cast<float>(i) + COLUMN_WIDTH * 0.5f,
                panelTopLeft_.y + LABEL_HEIGHT * 0.5f
            });
            // 色は選択状態に応じてupdate()で毎フレーム設定するため、ここでは初期値(未選択色)だけ入れておく
            text->setColor(dxe::Colors::White);
            text->setEdgeColor(dxe::Colors::Black);

            labelTexts_.push_back(text);
        }
    }

    void gmRudderHUD::update(float dt)
    {
        auto playerShip = playerShip_.lock();
        if (!playerShip) {
            visible_ = false;
            return;
        }

        const float targetRudder = playerShip->getDynamics().targetRudder;

        // 目標舵角が中央(0)でない間だけ表示する(Q/E・A/Dいずれの操作かは問わない)
        visible_ = std::fabs(targetRudder) > 1e-4f;
        if (!visible_) return;

        // 目標舵角に一番近いRUDDER_LEVELSの段階を選択中として扱う
        int nearestIndex = 0;
        float nearestDiff = std::fabs(targetRudder - gmShip::RUDDER_LEVELS[0]);
        for (int i = 1; i < 5; ++i) {
            const float diff = std::fabs(targetRudder - gmShip::RUDDER_LEVELS[i]);
            if (diff < nearestDiff) {
                nearestDiff = diff;
                nearestIndex = i;
            }
        }

        for (int i = 0; i < 5; ++i) {
            const bool selected = (i == nearestIndex);
            if (selected) {
                labelTexts_[i]->setColor(dxe::Colors::Black);
                labelTexts_[i]->setEdgeColor(dxe::Colors::White);
            }
            else {
                labelTexts_[i]->setColor(dxe::Colors::White);
                labelTexts_[i]->setEdgeColor(dxe::Colors::Black);
            }
        }
    }

    void gmRudderHUD::draw()
    {
        if (!visible_) return;

        auto playerShip = playerShip_.lock();
        if (!playerShip) return;

        const float targetRudder = playerShip->getDynamics().targetRudder;

        int nearestIndex = 0;
        float nearestDiff = std::fabs(targetRudder - gmShip::RUDDER_LEVELS[0]);
        for (int i = 1; i < 5; ++i) {
            const float diff = std::fabs(targetRudder - gmShip::RUDDER_LEVELS[i]);
            if (diff < nearestDiff) {
                nearestDiff = diff;
                nearestIndex = i;
            }
        }

        // ---- ラベル行(選択中の段階だけ白背景反転) ----
        for (int i = 0; i < 5; ++i) {
            if (i == nearestIndex) {
                const int left   = static_cast<int>(panelTopLeft_.x + COLUMN_WIDTH * static_cast<float>(i));
                const int right  = static_cast<int>(left + COLUMN_WIDTH);
                const int top    = static_cast<int>(panelTopLeft_.y);
                const int bottom = static_cast<int>(top + LABEL_HEIGHT);
                DrawBox(left, top, right, bottom, dxe::Colors::White, TRUE);
            }
            labelTexts_[i]->draw();
        }

        // ---- 実舵角を指す三角ポインタ ----
        // RUDDER_LEVELS[0](-1.0、一番左の列の中心)〜RUDDER_LEVELS[4](1.0、一番右の列の中心)の範囲に、
        // 現在の実舵角(連続値)を線形補間してx座標を求める。
        const float leftColumnCenterX  = panelTopLeft_.x + COLUMN_WIDTH * 0.5f;
        const float rightColumnCenterX = panelTopLeft_.x + COLUMN_WIDTH * 4.0f + COLUMN_WIDTH * 0.5f;

        const float currentRudder = playerShip->getDynamics().rudder;
        const float t = std::clamp((currentRudder - gmShip::RUDDER_LEVELS[0]) /
                                    (gmShip::RUDDER_LEVELS[4] - gmShip::RUDDER_LEVELS[0]), 0.0f, 1.0f);
        const float pointerX = leftColumnCenterX + t * (rightColumnCenterX - leftColumnCenterX);
        const float pointerTopY = panelTopLeft_.y + LABEL_HEIGHT + POINTER_GAP;

        // 上向きの三角形(ラベル行を指す)。塗りつぶし+黒縁取り(2回描画)で視認性を確保する。
        const int x1 = static_cast<int>(pointerX);
        const int y1 = static_cast<int>(pointerTopY);
        const int x2 = static_cast<int>(pointerX - POINTER_SIZE * 0.5f);
        const int y2 = static_cast<int>(pointerTopY + POINTER_SIZE);
        const int x3 = static_cast<int>(pointerX + POINTER_SIZE * 0.5f);
        const int y3 = y2;

        DrawTriangle(x1, y1, x2, y2, x3, y3, dxe::Colors::White, TRUE);
        DrawTriangle(x1, y1, x2, y2, x3, y3, dxe::Colors::Black, FALSE);
    }

}
