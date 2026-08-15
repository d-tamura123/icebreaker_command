// gmHpBarUI.cpp
#include "gmHpBarUI.h"
#include "../object/gmPlayerShip.h"
#include <DxLib.h>
#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>
#include <cmath>

namespace gm {

    gmHpBarUI::gmHpBarUI(std::shared_ptr<gmPlayerShip> playerShip)
        : gmUIObjectBase({ BAR_LEFT_MARGIN, BAR_POSITION_Y })
        , playerShip_(std::move(playerShip))
    {
        hFillTexture_ = LoadGraph("resource/graphics/hud/hp_bar_fill_h18.png");

        // 初期表示がゼロから追従してしまわないよう、開始時点の比率で追従バーを初期化しておく
        if (auto ship = playerShip_.lock()) {
            trailingHpRatio_ = (ship->getMaxHp() > 0.0f) ? (ship->getHp() / ship->getMaxHp()) : 0.0f;
        }
    }

    void gmHpBarUI::update(float dt)
    {
        auto ship = playerShip_.lock();
        if (!ship) return;

        const float currentRatio = (ship->getMaxHp() > 0.0f)
            ? std::clamp(ship->getHp() / ship->getMaxHp(), 0.0f, 1.0f)
            : 0.0f;

        // 追従バーを、現在HP比率へ指数関数的に漸近させる(氷山のdisplayScale_と同じ考え方)。
        // TRAIL_CATCHUP_SECONDS前後でおおよそ追いつく速さになるよう、フレームレートに依存しない
        // 減衰係数を毎フレーム計算する(dtが小さいフレームほど、この係数は1.0に近づき変化が小さくなる)。
        const float decay = expf(-dt / TRAIL_CATCHUP_SECONDS);
        trailingHpRatio_ = currentRatio + (trailingHpRatio_ - currentRatio) * decay;
    }

    void gmHpBarUI::draw()
    {
        auto ship = playerShip_.lock();
        if (!ship) return;

        const float currentRatio = (ship->getMaxHp() > 0.0f)
            ? std::clamp(ship->getHp() / ship->getMaxHp(), 0.0f, 1.0f)
            : 0.0f;

        const int left = static_cast<int>(position_.x);
        const int top = static_cast<int>(position_.y);
        const int right = static_cast<int>(position_.x + BAR_WIDTH);
        const int bottom = static_cast<int>(position_.y + BAR_HEIGHT);

        // ---- 1. 背景(グレー単色塗り) ----
        DrawBox(left, top, right, bottom, GetColor(60, 60, 60), TRUE);

        // ---- 2. 現在HP分の塗り(hp_bar_fill_h18.pngを左から現在HP比率分だけ切り出して等倍描画) ----
        if (hFillTexture_ >= 0 && currentRatio > 0.0f) {
            const int fillWidth = static_cast<int>(BAR_WIDTH * currentRatio);
            DrawRectGraph(left, top, 0, 0, fillWidth, static_cast<int>(BAR_HEIGHT), hFillTexture_, TRUE, FALSE);
        }

        // ---- 3. 追従バー(淡黄白)。ダメージ・回復どちらでも、直前のHP位置から現在HPへの
        //      「差分」区間をハイライトする形で、点滅を使わずに変化の体感を伝える。
        //   - ダメージ時(trailingHpRatio_ > currentRatio): 差分区間は現在HP塗りより右側
        //     (灰色の背景の上)に位置し、削られた分がじわっと消えていくように見える。
        //   - 回復時(trailingHpRatio_ < currentRatio): 差分区間は現在HP塗りの内側に位置するため、
        //     現在HP塗り(2番)より後に描画することで、回復した分が一時的にハイライトされて見える。
        const float loRatio = std::min(trailingHpRatio_, currentRatio);
        const float hiRatio = std::max(trailingHpRatio_, currentRatio);
        if (hiRatio - loRatio > 1e-4f) {
            const int segLeft = left + static_cast<int>(BAR_WIDTH * loRatio);
            const int segRight = left + static_cast<int>(BAR_WIDTH * hiRatio);
            DrawBox(segLeft, top, segRight, bottom, GetColor(0xE8, 0xE0, 0xC0), TRUE);
        }

        // ---- 4. 外枠 ----
        DrawBox(left, top, right, bottom, dxe::Colors::Black, FALSE);
    }

}
