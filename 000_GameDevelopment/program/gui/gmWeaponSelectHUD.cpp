// gmWeaponSelectHUD.cpp
#include "gmWeaponSelectHUD.h"
#include "../weapon/gmWeaponSelectionState.h"
#include "../util/gmFontCache.h"
#include "../../ResourceConstantHedder.h"
#include <DxLib.h>

namespace gm {

    gmWeaponSelectHUD::gmWeaponSelectHUD(std::shared_ptr<gmWeaponSelectionState> state, std::function<void()> onRecoveryClick)
        : gmUIObjectBase({ 0.0f, 0.0f }) // このクラス自体はpositionを使わない(各アイコンの座標を個別に持つ)
        , state_(std::move(state))
    {
        // ---- レイアウト計算(武器3種をひとまとまりに、リカバリだけ少し離す) ----
        const float groupWidth = ICON_SIZE * 3.0f + ICON_SPACING * 2.0f;
        const float totalWidth = groupWidth + RECOVERY_EXTRA_GAP + ICON_SIZE;
        const float startX = PANEL_CENTER_X - totalWidth * 0.5f;

        meltBulletPos_   = { startX, PANEL_TOP_Y };
        breakBulletPos_  = { meltBulletPos_.x  + ICON_SIZE + ICON_SPACING, PANEL_TOP_Y };
        flamethrowerPos_ = { breakBulletPos_.x + ICON_SIZE + ICON_SPACING, PANEL_TOP_Y };
        recoveryPos_     = { flamethrowerPos_.x + ICON_SIZE + RECOVERY_EXTRA_GAP, PANEL_TOP_Y };

        // ---- アイコンボタン(通常/ホバー/押下は画像側で作り込み済みのため、普通のボタンとして扱うだけでよい) ----
        meltBulletButton_ = std::make_unique<gmUIImageButton>(
            meltBulletPos_, ICON_SIZE,
            "resource/graphics/hud/wpn_melt_bullet_normal_48.png",
            "resource/graphics/hud/wpn_melt_bullet_hover_48.png",
            "resource/graphics/hud/wpn_melt_bullet_pressed_48.png",
            [this]() { if (state_) state_->selectWeapon(gmWeaponType::MeltBullet); });

        breakBulletButton_ = std::make_unique<gmUIImageButton>(
            breakBulletPos_, ICON_SIZE,
            "resource/graphics/hud/wpn_break_bullet_normal_48.png",
            "resource/graphics/hud/wpn_break_bullet_hover_48.png",
            "resource/graphics/hud/wpn_break_bullet_pressed_48.png",
            [this]() { if (state_) state_->selectWeapon(gmWeaponType::BreakBullet); });

        flamethrowerButton_ = std::make_unique<gmUIImageButton>(
            flamethrowerPos_, ICON_SIZE,
            "resource/graphics/hud/wpn_flamethrower_normal_48.png",
            "resource/graphics/hud/wpn_flamethrower_hover_48.png",
            "resource/graphics/hud/wpn_flamethrower_pressed_48.png",
            [this]() { if (state_) state_->selectWeapon(gmWeaponType::Flamethrower); });

        recoveryButton_ = std::make_unique<gmUIImageButton>(
            recoveryPos_, ICON_SIZE,
            "resource/graphics/hud/wpn_hp_recovery_normal_48.png",
            "resource/graphics/hud/wpn_hp_recovery_hover_48.png",
            "resource/graphics/hud/wpn_hp_recovery_pressed_48.png",
            std::move(onRecoveryClick));

        // ---- リキャストゲージ用マスク画像(4アイコン共通で1枚だけ使い回す) ----
        // 元画像は128x128だが、DrawCircleGauge()には拡縮パラメータが無く常に画像の実サイズで
        // 描画されるため、ここで一度だけICON_SIZE(48x48)に縮小した新しいハンドルを作っておく
        // (MakeScreenで縮小先を作り、そこへDrawExtendGraphで描き込むDxLibの定石パターン)。
        const int originalHandle = LoadGraph("resource/graphics/hud/wpn_recast_mask.png");
        const int prevScreen = GetDrawScreen();
        const int resizedHandle = MakeScreen(static_cast<int>(ICON_SIZE), static_cast<int>(ICON_SIZE), TRUE);

        if (originalHandle >= 0 && resizedHandle >= 0) {
            SetDrawScreen(resizedHandle);
            DrawExtendGraph(0, 0, static_cast<int>(ICON_SIZE), static_cast<int>(ICON_SIZE), originalHandle, TRUE);
            SetDrawScreen(prevScreen);

            DeleteGraph(originalHandle); // 縮小済みハンドルだけ残せば十分なので、元の128x128は破棄する
            hRecastMask_ = resizedHandle;
        }
        else {
            hRecastMask_ = originalHandle; // 縮小に失敗した場合は等倍のまま使う(無いよりはまし)
        }

        // ---- ショートカットキー番号ラベル ----
        Shared<dxe::FontTextResouce> labelFontResource = gmFontCache::GetOrCreate(
            KEY_LABEL_FONT_SIZE, FONT_NAME_SAWARABI_GOTHIC, FILE_PATH_TTF_SAWARABIGOTHIC_REGULAR);

        auto makeKeyLabel = [&](const char* label, const tnl::Vector2f& iconTopLeft) {
            Shared<dxe::FontText> text = dxe::FontText::Create(labelFontResource);
            text->setString(label);
            text->setLocation(dxe::eRectOrigin::CENTER_TOP);
            text->setPosition({ iconTopLeft.x + ICON_SIZE * 0.5f, iconTopLeft.y + ICON_SIZE + KEY_LABEL_GAP });
            text->setColor(dxe::Colors::White);
            text->setEdgeColor(dxe::Colors::Black);
            return text;
        };

        keyLabel1_ = makeKeyLabel("1", meltBulletPos_);
        keyLabel2_ = makeKeyLabel("2", breakBulletPos_);
        keyLabel3_ = makeKeyLabel("3", flamethrowerPos_);
        keyLabel5_ = makeKeyLabel("5", recoveryPos_);
    }

    void gmWeaponSelectHUD::setCursorModeActive(bool active)
    {
        if (meltBulletButton_)   meltBulletButton_->setInputEnabled(active);
        if (breakBulletButton_)  breakBulletButton_->setInputEnabled(active);
        if (flamethrowerButton_) flamethrowerButton_->setInputEnabled(active);
        if (recoveryButton_)     recoveryButton_->setInputEnabled(active);
    }

    void gmWeaponSelectHUD::update(float dt)
    {
        if (meltBulletButton_)   meltBulletButton_->update(dt);
        if (breakBulletButton_)  breakBulletButton_->update(dt);
        if (flamethrowerButton_) flamethrowerButton_->update(dt);
        if (recoveryButton_)     recoveryButton_->update(dt);
    }

    void gmWeaponSelectHUD::draw()
    {
        if (!state_) return;

        const gmWeaponType selected = state_->getSelectedWeapon();

        // ---- 溶かす弾 ----
        if (meltBulletButton_) meltBulletButton_->draw();
        drawRecastGauge(meltBulletPos_, state_->getRecastRemainingRatio(gmWeaponType::MeltBullet));
        if (selected == gmWeaponType::MeltBullet) drawSelectionFrame(meltBulletPos_);
        keyLabel1_->draw();

        // ---- 割る弾 ----
        if (breakBulletButton_) breakBulletButton_->draw();
        drawRecastGauge(breakBulletPos_, state_->getRecastRemainingRatio(gmWeaponType::BreakBullet));
        if (selected == gmWeaponType::BreakBullet) drawSelectionFrame(breakBulletPos_);
        keyLabel2_->draw();

        // ---- 火炎放射 ----
        if (flamethrowerButton_) flamethrowerButton_->draw();
        drawRecastGauge(flamethrowerPos_, state_->getRecastRemainingRatio(gmWeaponType::Flamethrower));
        if (selected == gmWeaponType::Flamethrower) drawSelectionFrame(flamethrowerPos_);
        keyLabel3_->draw();

        // ---- リカバリ(選択の概念が無いので選択枠は描かない) ----
        if (recoveryButton_) recoveryButton_->draw();
        drawRecastGauge(recoveryPos_, state_->getRecoveryRemainingRatio());
        keyLabel5_->draw();
    }

    tnl::Vector2f gmWeaponSelectHUD::GetIconCenter(const tnl::Vector2f& topLeft)
    {
        return { topLeft.x + ICON_SIZE * 0.5f, topLeft.y + ICON_SIZE * 0.5f };
    }

    void gmWeaponSelectHUD::drawRecastGauge(const tnl::Vector2f& topLeft, float remainingRatio)
    {
        if (remainingRatio <= 0.0f || hRecastMask_ < 0) return; // 発射可能なら何も描かない

        const tnl::Vector2f center = GetIconCenter(topLeft);

        // 使った直後は無(0%)、時間経過とともに徐々に扇形が描かれていき、
        // 発射可能に近づくにつれて完璧な円(100%)になる。
        // そのため、「残り」ではなく「経過(進捗)」の割合をPercentとして使う。
        const float progressRatio = 1.0f - remainingRatio; // 0.0=使った直後 〜 1.0=発射可能

        const double percent = static_cast<double>(progressRatio) * 100.0;
        DrawCircleGauge(static_cast<int>(center.x), static_cast<int>(center.y), percent, hRecastMask_);
    }

    void gmWeaponSelectHUD::drawSelectionFrame(const tnl::Vector2f& topLeft)
    {
        const int left = static_cast<int>(topLeft.x - SELECTION_FRAME_MARGIN);
        const int top = static_cast<int>(topLeft.y - SELECTION_FRAME_MARGIN);
        const int right = static_cast<int>(topLeft.x + ICON_SIZE + SELECTION_FRAME_MARGIN);
        const int bottom = static_cast<int>(topLeft.y + ICON_SIZE + SELECTION_FRAME_MARGIN);

        // トップバーのアイコンの発光色(#3BD7FC)に合わせた水色。統一感を出すための指定。
        DrawBox(left, top, right, bottom, GetColor(0x3B, 0xD7, 0xFC), FALSE);
    }

}
