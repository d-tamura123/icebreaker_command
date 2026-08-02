// gmProjectileManager.h
#pragma once
#include <dxe.h>
#include <vector>
#include <memory>
#include "../object/gmProjectile.h"
#include "../object/gmSplitProjectile.h"

namespace gm {

    class gmCollisionSystem;
    class gmSpriteAnimRegistry;

    // ------------------------------------------------------------
    // 発射された全弾(gmProjectileBaseを継承する火炎砲弾・割り砲弾)の
    // 生成・毎フレーム更新・描画・後片付けを一元管理する。
    // gmVFXManagerやgmCollisionSystemと同様のパターン
    // (登録して、毎フレームupdate/renderを呼ぶだけ)。
    // ------------------------------------------------------------
    class gmProjectileManager {
    public:
        gmProjectileManager(
            const std::shared_ptr<gmCollisionSystem>& collisionSystem,
            const std::shared_ptr<gmSpriteAnimRegistry>& spriteRegistry);

        // 通常弾(火炎砲弾)を1発発射する
        void fire(const tnl::Vector3& startPos, const tnl::Vector3& targetPos);

        // 割り砲弾を1発発射する
        void fireSplit(const tnl::Vector3& startPos, const tnl::Vector3& targetPos);

        void update(float deltaTime);
        void render(const Shared<dxe::Camera>& camera);

    private:
        std::shared_ptr<gmCollisionSystem>    collisionSystem_;
        std::shared_ptr<gmSpriteAnimRegistry> spriteRegistry_;
        std::vector<std::shared_ptr<gmProjectileBase>> projectiles_; // 発射中の全弾(火炎砲弾・割り砲弾が混在する)

        int nextId_ = 0; // 弾に付与する識別IDの通し番号(発射のたびに加算する)
    };
}
