// gmSpeedHUD.h
// フェーズ1.3「操作系HUD」のうち、速度段階の表示部分(WoWS風。舵角は別クラスで対応予定)。
//
// gmShip::SPEED_LEVELS(7段階)の各段階をラベルとして縦一列に並べ、現在の目標速度段階
// (gmShip::getSpeedIndex())に一致する行だけ、白背景で反転表示する。
// 加えて、実際の現在速度(gmShip::getDynamics().speed)を「kts」風の演出用数値に変換して
// 選択行の右側に表示する(この数値変換はゲームロジックとは無関係な表示専用の演出)。
//
// 暫定実装として、画像は使わずDxLibの矩形塗りつぶし(DrawBox)+dxe::FontText(縁取り込み)
// だけで組んでいる。
#pragma once
#include "gmUIObjectBase.h"
#include <dxe.h>
#include <memory>
#include <vector>

namespace gm {

    class gmPlayerShip;

    class gmSpeedHUD : public gmUIObjectBase {
    public:
        // arg1... 追従対象のプレイヤー船
        // Note:
        // 　引数1つのコンストラクタは暗黙変換されやすいため、
        // 　意図しない shared_ptr→gmSpeedHUD の変換を防ぐ目的で explicit を付けている。
        explicit gmSpeedHUD(std::shared_ptr<gmPlayerShip> playerShip);

        void update(float dt) override;
        void draw() override;

    private:
        // ---- レイアウト定数 ----
        static constexpr float PANEL_LEFT_MARGIN   = 20.0f; // 画面左端からの余白
        static constexpr float PANEL_BOTTOM_MARGIN = 20.0f; // 画面下端からの余白
        static constexpr float ROW_WIDTH  = 64.0f;
        static constexpr float ROW_HEIGHT = 22.0f;
        static constexpr float SPEED_VALUE_GAP = 12.0f; // 選択行の右端 〜 実速度数値までの余白

        static constexpr int32_t LABEL_FONT_SIZE = 16;

        // 演出用: dynamics_.speed(-1.0〜1.0の抽象値)を「kts」風の数値に変換する係数。
        // ゲームロジックには一切影響しない、表示専用の作り物の値(こだわりなし、暫定)。
        static constexpr float SPEED_DISPLAY_KTS_SCALE = 35.0f;

        std::weak_ptr<gmPlayerShip> playerShip_;

        // gmShip::SPEED_LEVELS(添字0=後退フル 〜 添字6=前進フル)と1対1対応するラベル。
        // 表示は上から前進フル→停止→後退フル の順(添字の降順)に並べる。
        static constexpr const char* SPEED_LABELS[7] = {
            "FULL", "1/2", "STOP", "1/4", "1/2", "3/4", "FULL"
        };

        // ラベル7個ぶんのFontText(内容は固定。選択状態が変わるたびに色だけ差し替える)
        std::vector<Shared<dxe::FontText>> labelTexts_;

        // 実速度の数値表示(内容が毎フレーム変わる)
        Shared<dxe::FontText> speedValueText_;

        tnl::Vector2f panelTopLeft_; // 一番上の行(添字6=FULL)の左上座標
    };

}
