// gmProjectileManager.cpp
#include "gmProjectileManager.h"
#include "../collision/gmCollisionSystem.h"
#include <algorithm>

namespace gm {

    gmProjectileManager::gmProjectileManager(
        const std::shared_ptr<gmCollisionSystem>& collisionSystem,
        const std::shared_ptr<gmSpriteAnimRegistry>& spriteRegistry)
        : collisionSystem_(collisionSystem)
        , spriteRegistry_(spriteRegistry)
    {
    }

    // ------------------------------------------------------------
    // 火炎砲弾(gmProjectile)を1発生成し、管理リストと衝突システムの両方へ登録する。
    // ------------------------------------------------------------
    void gmProjectileManager::fire(const tnl::Vector3& startPos, const tnl::Vector3& targetPos)
    {
        const std::string id = "projectile_" + std::to_string(nextId_++);
        auto projectile = std::make_shared<gmProjectile>(id, startPos, targetPos, spriteRegistry_);

        projectiles_.push_back(projectile);

        if (collisionSystem_) {
            collisionSystem_->registerObject(projectile);
        }
    }

    // ------------------------------------------------------------
    // 割り砲弾(gmSplitProjectile)を1発生成し、管理リストと衝突システムの両方へ登録する。
    // (fire()と処理内容はほぼ同じで、生成するクラスが異なるだけ)
    // ------------------------------------------------------------
    void gmProjectileManager::fireSplit(const tnl::Vector3& startPos, const tnl::Vector3& targetPos)
    {
        const std::string id = "split_projectile_" + std::to_string(nextId_++);
        auto projectile = std::make_shared<gmSplitProjectile>(id, startPos, targetPos, spriteRegistry_);

        projectiles_.push_back(projectile);

        if (collisionSystem_) {
            collisionSystem_->registerObject(projectile);
        }
    }

    // ------------------------------------------------------------
    // 管理下の全弾を更新し、着弾済み(kill()された)弾をリストから取り除く。
    // ------------------------------------------------------------
    void gmProjectileManager::update(float deltaTime)
    {
        for (auto& projectile : projectiles_) {
            if (projectile->isAlive()) {
                projectile->update(deltaTime);
            }
        }

        // 着弾済み(kill()された)弾をリストから取り除く
        // (衝突システム側はweak_ptr登録のため、ここで取り除けば自動的に向こうからも外れる)
        projectiles_.erase(
            std::remove_if(projectiles_.begin(), projectiles_.end(),
                [](const std::shared_ptr<gmProjectileBase>& projectile) { return !projectile->isAlive(); }),
            projectiles_.end()
        );
    }

    // ------------------------------------------------------------
    // 管理下の全弾を描画する。
    // ------------------------------------------------------------
    void gmProjectileManager::render(const Shared<dxe::Camera>& camera)
    {
        for (auto& projectile : projectiles_) {
            if (projectile->isAlive()) {
                projectile->render(camera);
            }
        }
    }
}
