// gmWeaponSelectionState.h
// 「武器選択HUD + 入力」の状態本体。
//
// 「現在選択中の武器」「武器ごとのリキャスト残り時間」「リカバリのクールダウン」を一元管理する。
// gmProjectileManager/gmFlameThrowerManagerの発射処理自体には手を入れず、その手前
// (gmGameScene::tryFireProjectileOnClick()等)でこのクラスを参照して発射可否をガードする。
// HUD側(gmWeaponSelectHUD)も同じインスタンスを参照し、選択枠・リキャストゲージの表示に使う。
#pragma once

namespace gm {

    enum class gmWeaponType {
        MeltBullet,   // 溶かす弾(gmProjectileManager::fire())
        BreakBullet,  // 割る弾(gmProjectileManager::fireSplit())
        Flamethrower, // 火炎放射(gmFlameThrowerManager::fire())
    };

    class gmWeaponSelectionState {
    public:
        void update(float dt);

        // 1/2/3キー・武器選択ボタンのクリックで呼ぶ。リキャスト中でも選択自体は可能
        // (選択できるだけで、発射自体はcanFireSelectedWeapon()がfalseの間はできない)。
        void selectWeapon(gmWeaponType type) { selectedWeapon_ = type; }
        gmWeaponType getSelectedWeapon() const { return selectedWeapon_; }

        // 選択中の武器が発射可能(リキャスト完了)かどうか
        bool canFireSelectedWeapon() const { return getRecastRemainingSec(selectedWeapon_) <= 0.0f; }

        // 発射した瞬間に呼ぶ(選択中武器のリキャストを開始する)
        void notifyFired();

        // arg1... 対象武器
        float getRecastRemainingSec(gmWeaponType type) const;

        // arg1... 対象武器
        // ret.... リキャストゲージ用の進捗(0.0=発射可能 〜 1.0=たった今発射した直後)
        float getRecastRemainingRatio(gmWeaponType type) const;

        // ---- リカバリ(5キー)。武器選択・リキャストとは独立したクールダウンを持つ ----
        bool canUseRecovery() const { return recoveryRemainingSec_ <= 0.0f; }
        void notifyRecoveryUsed();
        float getRecoveryRemainingRatio() const;

    private:
        static float GetRecastDurationSec(gmWeaponType type);

        gmWeaponType selectedWeapon_ = gmWeaponType::MeltBullet;

        // 3種の武器に対応するリキャスト残り秒数(添字はstatic_cast<int>(gmWeaponType))
        float recastRemainingSec_[3] = { 0.0f, 0.0f, 0.0f };

        float recoveryRemainingSec_ = 0.0f;
    };

}
