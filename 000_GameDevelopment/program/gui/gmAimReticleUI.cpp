// gmAimReticleUI.cpp
#include "gmAimReticleUI.h"
#include "../util/gmFontCache.h"
#include "../../ResourceConstantHedder.h"
#include <DxLib.h>
#include <cstdio>

namespace gm {

    gmAimReticleUI::gmAimReticleUI()
        : gmUIObjectBase({ 0.0f, 0.0f }) // 画面中央基準で毎フレーム計算するため、position_自体は使わない
    {
        hScaleImage_ = LoadGraph("resource/graphics/hud/aim_scale_prototype.png");

        Shared<dxe::FontTextResouce> fontResource = gmFontCache::GetOrCreate(
            DISTANCE_TEXT_FONT_SIZE, FONT_NAME_SAWARABI_GOTHIC, FILE_PATH_TTF_SAWARABIGOTHIC_REGULAR);

        distanceText_ = dxe::FontText::Create(fontResource);
        distanceText_->setLocation(dxe::eRectOrigin::RIGHT_TOP);
        distanceText_->setColor(dxe::Colors::White);
        distanceText_->setEdgeColor(dxe::Colors::Black);
    }

    void gmAimReticleUI::update(float dt)
    {
        if (!visible_) return;

        // 距離の単位は演出用の"m"表記(world単位をそのまま数値化しているだけで、
        // 実距離との対応は無い。速度HUDの"kt"表記と同じ扱い)。
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.0fm", aimTargetDistance_);
        distanceText_->setString(buf);

        const float centerX = DXE_WINDOW_WIDTH_F * 0.5f;
        const float centerY = DXE_WINDOW_HEIGHT_F * 0.5f;
        const float imageRight  = centerX + SCALE_IMAGE_WIDTH * 0.5f;
        const float imageBottom = centerY + SCALE_IMAGE_HEIGHT * 0.5f;

        distanceText_->setPosition({
            imageRight - DISTANCE_TEXT_MARGIN_X,
            imageBottom + DISTANCE_TEXT_MARGIN_Y
        });
    }

    void gmAimReticleUI::draw()
    {
        if (!visible_) return;

        const float centerX = DXE_WINDOW_WIDTH_F * 0.5f;
        const float centerY = DXE_WINDOW_HEIGHT_F * 0.5f;

        // ---- 目盛りメーター(半透明。画像自体が中心軸をぴったり合わせて作られているため、
        //      画像全体を画面中央へ置くだけで位置補正は不要) ----
        if (hScaleImage_ >= 0) {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, SCALE_IMAGE_ALPHA);

            const int left = static_cast<int>(centerX - SCALE_IMAGE_WIDTH * 0.5f);
            const int top  = static_cast<int>(centerY - SCALE_IMAGE_HEIGHT * 0.5f);
            DrawExtendGraph(
                left, top,
                left + static_cast<int>(SCALE_IMAGE_WIDTH), top + static_cast<int>(SCALE_IMAGE_HEIGHT),
                hScaleImage_, TRUE);

            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }

        // ---- 照準(画面中央、3x3ドット。半透明。画像ではなく図形描画) ----
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, RETICLE_ALPHA);

        const int reticleLeft = static_cast<int>(centerX - RETICLE_DOT_SIZE * 0.5f);
        const int reticleTop  = static_cast<int>(centerY - RETICLE_DOT_SIZE * 0.5f);
        DrawBox(
            reticleLeft, reticleTop,
            reticleLeft + static_cast<int>(RETICLE_DOT_SIZE), reticleTop + static_cast<int>(RETICLE_DOT_SIZE),
            dxe::Colors::White, TRUE);

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // ---- 距離表示(不透明のまま。視認性優先) ----
        distanceText_->draw();
    }

}
