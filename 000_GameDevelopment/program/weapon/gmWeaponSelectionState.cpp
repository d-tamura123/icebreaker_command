// gmWeaponSelectionState.cpp
#include "gmWeaponSelectionState.h"
#include "../gmGameConfig.h"
#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>

namespace gm {

    void gmWeaponSelectionState::update(float dt)
    {
        for (float& remaining : recastRemainingSec_) {
            remaining = std::max(0.0f, remaining - dt);
        }
        recoveryRemainingSec_ = std::max(0.0f, recoveryRemainingSec_ - dt);
    }

    void gmWeaponSelectionState::notifyFired()
    {
        recastRemainingSec_[static_cast<int>(selectedWeapon_)] = GetRecastDurationSec(selectedWeapon_);
    }

    float gmWeaponSelectionState::getRecastRemainingSec(gmWeaponType type) const
    {
        return recastRemainingSec_[static_cast<int>(type)];
    }

    float gmWeaponSelectionState::getRecastRemainingRatio(gmWeaponType type) const
    {
        const float duration = GetRecastDurationSec(type);
        if (duration <= 0.0f) return 0.0f;
        return std::clamp(getRecastRemainingSec(type) / duration, 0.0f, 1.0f);
    }

    void gmWeaponSelectionState::notifyRecoveryUsed()
    {
        recoveryRemainingSec_ = RECOVERY_COOLDOWN_SEC;
    }

    float gmWeaponSelectionState::getRecoveryRemainingRatio() const
    {
        if (RECOVERY_COOLDOWN_SEC <= 0.0f) return 0.0f;
        return std::clamp(recoveryRemainingSec_ / RECOVERY_COOLDOWN_SEC, 0.0f, 1.0f);
    }

    float gmWeaponSelectionState::GetRecastDurationSec(gmWeaponType type)
    {
        switch (type) {
            case gmWeaponType::MeltBullet:   return WEAPON_MELT_BULLET_RECAST_SEC;
            case gmWeaponType::BreakBullet:  return WEAPON_BREAK_BULLET_RECAST_SEC;
            case gmWeaponType::Flamethrower: return WEAPON_FLAMETHROWER_RECAST_SEC;
        }
        return 0.0f;
    }

}
