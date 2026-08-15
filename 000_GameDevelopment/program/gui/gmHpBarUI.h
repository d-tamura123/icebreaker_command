// gmHpBarUI.h
// 「プレイヤーHPバー」。
//
// レイヤー構成(奥から手前):
//   1. 背景(グレー単色塗り)
//   2. 現在HP分の塗り(hp_bar_fill_h18.png。現在HP比率ぶんだけ左から切り出して等倍描画。
//      168x18の原寸のまま使うため拡大縮小はしない)
//   3. 追従バー(淡黄白)。ダメージ・回復どちらでも、直前のHP位置から現在HPへの差分区間を
//      ハイライトする形で、点滅等の目に負担がかかる演出を使わずに変化の体感を伝える。
//   4. 外枠(黒)
#pragma once
#include "gmUIObjectBase.h"
#include <dxe.h>
#include <memory>

namespace gm {

    // 前方宣言
    class gmPlayerShip;

    class gmHpBarUI : public gmUIObjectBase {
    public:
        // arg1... 追従対象のプレイヤー船
        explicit gmHpBarUI(std::shared_ptr<gmPlayerShip> playerShip);

        void update(float dt) override;
        void draw() override;

    private:
        // ---- レイアウト定数 ----
        static constexpr float BAR_LEFT_MARGIN  = 20.0f;                    // 画面左端からの余白
        static constexpr float BAR_TOP_MARGIN   = 12.0f;                    // 画面上端バー(高さ32px。gmTopBarUI::BAR_HEIGHT)からの余白
        static constexpr float BAR_POSITION_Y   = 32.0f + BAR_TOP_MARGIN;   // 上端バーのすぐ下

        static constexpr float BAR_WIDTH        = 168.0f;                   // hp_bar_fill_h18.pngの実サイズに合わせる(拡縮しない)
        static constexpr float BAR_HEIGHT       = 18.0f;

        // 追従バーが現在HPへ追いつくまでのおおよその秒数(暫定値。テストで調整)
        static constexpr float TRAIL_CATCHUP_SECONDS = 0.6f;

        std::weak_ptr<gmPlayerShip> playerShip_;

        int hFillTexture_ = -1;

        // 追従バーの表示比率(0.0〜1.0)。現在HP比率へ、TRAIL_CATCHUP_SECONDSかけて漸近する。
        float trailingHpRatio_ = 1.0f;
    };

}
