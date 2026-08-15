// gmWeaponSelectHUD.h
// 「武器選択HUD」。画面中央下寄りに、溶かす弾/割る弾/火炎放射/リカバリの
// 4アイコンを並べる。アイコン自体(通常/ホバー/押下)は既存のgmUIImageButtonをそのまま流用し
// (状態変化は画像側で作り込み済みのため、プログラム側は普通のボタンとして扱うだけでよい)、
// その上に以下をこのクラス側で描画する:
//   - 選択枠(DrawBox。3種の武器のみ。リカバリは「選択」の概念が無いため対象外)
//   - リキャストゲージ(DrawCircleGauge。暗い半透明円が、リキャスト完了に近づくほど小さくなる)
//   - ショートカットキー番号のラベル(1/2/3/5)
#pragma once
#include "gmUIObjectBase.h"
#include "gmUIImageButton.h"
#include <dxe.h>
#include <memory>
#include <functional>

namespace gm {

    class gmWeaponSelectionState;

    class gmWeaponSelectHUD : public gmUIObjectBase {
    public:
        // arg1... 武器選択・リキャストの状態本体(参照して表示するだけ。選択の変更はボタンのクリック経由)
        // arg2... リカバリボタンのクリック時コールバック(5キー相当の処理を呼ぶ)
        gmWeaponSelectHUD(std::shared_ptr<gmWeaponSelectionState> state, std::function<void()> onRecoveryClick);

        void setCursorModeActive(bool active);

        void update(float dt) override;
        void draw() override;

    private:
        // ---- レイアウト定数(配置は暫定。座標決めの経緯はhud_spec参照) ----
        static constexpr float PANEL_CENTER_X = 640.0f; // 画面中央(DXE_WINDOW_WIDTH / 2)
        static constexpr float PANEL_TOP_Y    = 600.0f; // 暫定の高さ

        static constexpr float ICON_SIZE          = 48.0f; // アイコンの一辺(素材の実サイズに合わせる)
        static constexpr float ICON_SPACING       = 16.0f; // 武器アイコン間の余白
        static constexpr float RECOVERY_EXTRA_GAP = 24.0f; // 武器3種とリカバリの間の追加余白(グループを視覚的に分けるため)

        static constexpr float SELECTION_FRAME_MARGIN = 3.0f; // アイコンの外側へ、選択枠をどれだけ広げるか

        static constexpr float KEY_LABEL_GAP  = 4.0f;  // アイコン下端 〜 キー番号ラベルまでの余白
        static constexpr int32_t KEY_LABEL_FONT_SIZE = 14;

        std::shared_ptr<gmWeaponSelectionState> state_;

        // 表示順: 溶かす弾(1) / 割る弾(2) / 火炎放射(3) / リカバリ(5)
        std::unique_ptr<gmUIImageButton> meltBulletButton_;
        std::unique_ptr<gmUIImageButton> breakBulletButton_;
        std::unique_ptr<gmUIImageButton> flamethrowerButton_;
        std::unique_ptr<gmUIImageButton> recoveryButton_;

        // 各アイコンの左上座標(選択枠・リキャストゲージ・キー番号ラベルの描画にそのまま使う)
        tnl::Vector2f meltBulletPos_;
        tnl::Vector2f breakBulletPos_;
        tnl::Vector2f flamethrowerPos_;
        tnl::Vector2f recoveryPos_;

        Shared<dxe::FontText> keyLabel1_;
        Shared<dxe::FontText> keyLabel2_;
        Shared<dxe::FontText> keyLabel3_;
        Shared<dxe::FontText> keyLabel5_;

        int hRecastMask_ = -1;

        // arg1... アイコンの左上座標
        // ret.... アイコンの中心座標(DrawCircleGauge・選択枠に使う)
        static tnl::Vector2f GetIconCenter(const tnl::Vector2f& topLeft);

        // arg1... アイコンの左上座標
        // arg2... リキャスト進捗(0.0=発射可能 〜 1.0=たった今使った直後)
        void drawRecastGauge(const tnl::Vector2f& topLeft, float remainingRatio);

        // arg1... アイコンの左上座標(選択枠を描く対象)
        void drawSelectionFrame(const tnl::Vector2f& topLeft);
    };

}
