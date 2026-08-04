// gmFlameThrowerManager.cpp
#include "gmFlameThrowerManager.h"
#include "../collision/gmCollisionSystem.h"
#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>
#include <cmath>

namespace gm {

    gmFlameThrowerManager::gmFlameThrowerManager(
        const std::shared_ptr<gmCollisionSystem>& collisionSystem,
        const std::shared_ptr<gmSpriteAnimRegistry>& spriteRegistry)
        : collisionSystem_(collisionSystem)
        , spriteRegistry_(spriteRegistry)
    {
    }

    // ------------------------------------------------------------
    // 火炎放射を1回発動する。
    //   手順1: 船の向きから「右」ベクトルを求める
    //   手順2: クリック位置が船の右側/左側どちらにあるかで、発射する側面を決める
    //   手順3: 発射位置(側面ハードポイント)・扇の中心方向を確定し、
    //          gmFlameThrowerAttackを生成する
    // ------------------------------------------------------------
    void gmFlameThrowerManager::fire(const tnl::Vector3& shipPos, float shipYaw, const tnl::Vector3& aimTargetPos)
    {
        if (isActive()) {
            // 簡易ロック: 発動中の火炎放射が終わるまで、次の発動は受け付けない
            return;
        }

        // ---- 手順1: 船の「前方」「右」ベクトル ----
        // gmShip::getForward()と同じ規約(角度0のとき+Z方向、角度が増えると+X方向へ回る)。
        const tnl::Vector3 forward(sinf(shipYaw), 0.0f, cosf(shipYaw));
        const tnl::Vector3 right(cosf(shipYaw), 0.0f, -sinf(shipYaw));

        // ---- 手順2: クリック位置から狙い方向と、どちら側面かを求める ----
        tnl::Vector3 toTarget = aimTargetPos - shipPos;
        toTarget.y = 0.0f;
        const float toTargetLenSq = tnl::Vector3::Dot(toTarget, toTarget);
        const tnl::Vector3 aimDir = (toTargetLenSq > 1e-8f) ? tnl::Vector3::Normalize(toTarget) : forward;

        const float sideDot = tnl::Vector3::Dot(aimDir, right);
        const float sideSign = (sideDot >= 0.0f) ? 1.0f : -1.0f; // 右舷なら+1、左舷なら-1

        // ---- 手順3: 発射位置・中心方向を確定して生成 ----
        const tnl::Vector3 originPos = shipPos + right * (sideSign * SIDE_OFFSET);

        const std::string id = "flamethrower_" + std::to_string(nextId_++);
        auto attack = std::make_shared<gmFlameThrowerAttack>(id, originPos, aimDir, spriteRegistry_);

        attacks_.push_back(attack);

        if (collisionSystem_) {
            collisionSystem_->registerObject(attack);
        }
    }

    // ------------------------------------------------------------
    // 管理下の全火炎放射を更新し、終了済み(kill()された)ものをリストから取り除く。
    // ------------------------------------------------------------
    void gmFlameThrowerManager::update(float deltaTime)
    {
        for (auto& attack : attacks_) {
            if (attack->isAlive()) {
                attack->update(deltaTime);
            }
        }

        attacks_.erase(
            std::remove_if(attacks_.begin(), attacks_.end(),
                [](const std::shared_ptr<gmFlameThrowerAttack>& attack) { return !attack->isAlive(); }),
            attacks_.end()
        );
    }

    // ------------------------------------------------------------
    // 管理下の全火炎放射を描画する。
    // ------------------------------------------------------------
    void gmFlameThrowerManager::render(const Shared<dxe::Camera>& camera)
    {
        for (auto& attack : attacks_) {
            if (attack->isAlive()) {
                attack->render(camera);
            }
        }
    }

    bool gmFlameThrowerManager::isActive() const
    {
        return std::any_of(attacks_.begin(), attacks_.end(),
            [](const std::shared_ptr<gmFlameThrowerAttack>& attack) { return attack->isAlive(); });
    }
}
