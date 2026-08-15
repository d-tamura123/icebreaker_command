// gmAimReticleUI.h
// フェーズ3「エイム時のUI」。エイムモード中(gmPlayerCameraController::isAimMode())のみ表示する。
//
// - 画面中央に、目盛りメーター画像(aim_scale_prototype.png)を半透明で描画する。
//   画像自体が中心軸(縦線・横線の交点)を画像の中心ピクセルに合わせて作られているため、
//   画像全体を画面中央に置くだけで位置補正は不要。
// - 画面中央に、3x3ドットの照準を半透明で描画する(画像ではなく図形描画)。
// - 目盛りメーターの右下あたりに、プレイヤー船〜狙い先までの距離を数値表示する
//   (不透明のまま。視認性を優先)。
#pragma once
#include "gmUIObjectBase.h"
#include <dxe.h>

namespace gm {

    class gmAimReticleUI : public gmUIObjectBase {
    public:
        gmAimReticleUI();

        // arg1... エイムモード中かどうか(この間だけ表示する)
        void setVisible(bool visible) { visible_ = visible; }

        // arg1... プレイヤー船〜狙い先までの距離(world単位)
        void setAimTargetDistance(float distance) { aimTargetDistance_ = distance; }

        void update(float dt) override;
        void draw() override;

    private:
        // ---- 目盛りメーター画像(aim_scale_prototype.png)の実サイズ。拡縮しない ----
        static constexpr float SCALE_IMAGE_WIDTH  = 800.0f;
        static constexpr float SCALE_IMAGE_HEIGHT = 140.0f;
        static constexpr int   SCALE_IMAGE_ALPHA  = 180; // 0(透明)〜255(不透明)

        // ---- 照準(3x3ドット) ----
        static constexpr float RETICLE_DOT_SIZE = 3.0f;
        static constexpr int   RETICLE_ALPHA    = 200;

        // ---- 距離表示(不透明のまま) ----
        static constexpr float DISTANCE_TEXT_MARGIN_X = 8.0f;       // 目盛り画像の右端からの余白
        static constexpr float DISTANCE_TEXT_MARGIN_Y = -40.0f;     // 目盛り画像の下端からの余白 (目視でメーターの下に来るくらいの位置に調整した)
        static constexpr int32_t DISTANCE_TEXT_FONT_SIZE = 14;

        int hScaleImage_ = -1;
        Shared<dxe::FontText> distanceText_;

        bool visible_ = false;
        float aimTargetDistance_ = 0.0f;
    };

}
