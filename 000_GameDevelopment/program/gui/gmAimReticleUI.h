// gmAimReticleUI.h
// 画面中央の照準表示。
//
// - 3x3ドットの照準(図形描画)は、周回モード・エイムモードいずれでも表示する
//
// - 目盛りメーター画像(aim_scale_prototype.png。半透明)と、プレイヤー船〜狙い先までの
//   距離表示は、エイムモード中のみ表示する(周回モード中は情報過多・画面圧迫になるため)。
//   画像自体が中心軸(縦線・横線の交点)を画像の中心ピクセルに合わせて作られているため、
//   画像全体を画面中央に置くだけで位置補正は不要。
#pragma once
#include "gmUIObjectBase.h"
#include <dxe.h>

namespace gm {

    class gmAimReticleUI : public gmUIObjectBase {
    public:
        gmAimReticleUI();

        // arg1... 照準ドットを表示するかどうか(周回・エイムいずれのモードでもtrueでよい。
        //         撃沈中等、狙い先自体が無意味なタイミングだけfalseにする想定)
        void setDotVisible(bool visible) { dotVisible_ = visible; }

        // arg1... 目盛りメーター・距離表示を表示するかどうか(エイムモード中のみtrue)
        void setAimUIVisible(bool visible) { aimUIVisible_ = visible; }


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

        bool dotVisible_ = false;
        bool aimUIVisible_ = false;
        float aimTargetDistance_ = 0.0f;
    };

}
