// gmRudderHUD.h
// フェーズ1.3「操作系HUD」のうち、舵角の表示部分。
//
// gmShip::RUDDER_LEVELS(5段階: MAX左/半分左/中央/半分右/MAX右)を横一列のラベルとして並べ、
// 現在の目標舵角(gmShip::getDynamics().targetRudder)に一致する段階だけ、白背景で反転表示する。
// その下に、実際の舵角(getDynamics().rudder。目標へ毎フレーム漸近する連続値)の位置を指す
// 三角ポインタを表示する。
//
// 常時表示ではなく、目標舵角が中央(0)でない間だけ表示する
// (Q/Eキーで段階操作中、またはA/Dキー押しっぱなし中のいずれでも、targetRudderが0以外になるため、
//  入力元(Q/E or A/D)を個別に見る必要はない)。
//
// 暫定実装として、画像は使わずDxLibの矩形塗りつぶし(DrawBox)・三角形描画(DrawTriangle)+
// dxe::FontText(縁取り込み)だけで組んでいる。
#pragma once
#include "gmUIObjectBase.h"
#include <dxe.h>
#include <memory>
#include <vector>

namespace gm {

    class gmPlayerShip;

    class gmRudderHUD : public gmUIObjectBase {
    public:
        // arg1... 追従対象のプレイヤー船
        explicit gmRudderHUD(std::shared_ptr<gmPlayerShip> playerShip);

        void update(float dt) override;
        void draw() override;

    private:
        // ---- レイアウト定数(配置は1.4実装まで仮置き。座標決めの経緯はhud_spec参照) ----
        static constexpr float PANEL_CENTER_X = 640.0f; // 画面中央(DXE_WINDOW_WIDTH / 2)
        static constexpr float PANEL_TOP_Y    = 460.0f; // 暫定の高さ。1.4(武器選択HUD)実装時に調整する

        static constexpr float COLUMN_WIDTH  = 50.0f; // ラベル1個あたりの幅
        static constexpr float LABEL_HEIGHT  = 22.0f;

        static constexpr float POINTER_GAP  = 4.0f;  // ラベル行の下端 〜 三角ポインタまでの余白
        static constexpr float POINTER_SIZE = 10.0f; // 三角ポインタの一辺の目安サイズ

        static constexpr int32_t LABEL_FONT_SIZE = 16;

        std::weak_ptr<gmPlayerShip> playerShip_;

        // gmShip::RUDDER_LEVELS(添字0=MAX左 〜 添字4=MAX右)と1対1対応するラベル。
        // 配列の並び順がそのまま画面上の左→右の並びになる(速度HUDと違い反転不要)。
        static constexpr const char* RUDDER_LABELS[5] = {
            "MAX", "1/2", "0", "1/2", "MAX"
        };

        // ラベル5個ぶんのFontText(内容は固定。選択状態が変わるたびに色だけ差し替える)
        std::vector<Shared<dxe::FontText>> labelTexts_;

        tnl::Vector2f panelTopLeft_; // 一番左のラベル(添字0=MAX左)の左上座標
        bool visible_ = false;
    };

}
