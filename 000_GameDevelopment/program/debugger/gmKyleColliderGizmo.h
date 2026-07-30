#pragma once
#include <dxe.h>
#include <memory>
#include "../collision/gmCollisionCategory.h"

namespace gm {

    class gmCollisionSystem;

    // ------------------------------------------------------------
    // Unityのギズモのような、コライダー形状のワイヤーフレーム表示。
    // gmCollisionSystemに登録済みの全オブジェクトを走査し、
    // 各コライダーの形状(Sphere/Box/Capsule/Ellipsoid)に応じた
    // 線画を描画するだけの、判定には一切関与しないデバッグ専用クラス。
    // ------------------------------------------------------------
    class gmKyleColliderGizmo {
    public:
        gmKyleColliderGizmo() = default;

        void render(const std::shared_ptr<gmCollisionSystem>& collisionSystem, const Shared<dxe::Camera>& camera);

    private:
        // 形状ごとのワイヤーフレーム描画
        void drawSphere(const tnl::Vector3& pos, float radius, unsigned int color);
        void drawBox(const tnl::Vector3& pos, const tnl::Vector3& size, const tnl::Quaternion& rot, unsigned int color);
        void drawCapsule(const tnl::Vector3& st, const tnl::Vector3& en, float radius, unsigned int color);
        void drawEllipsoid(const tnl::Vector3& pos, const tnl::Vector3& radii, const tnl::Quaternion& rot, unsigned int color);

        // 任意の平面(center + right*cosθ + up*sinθ)上に円を描く共通ヘルパー
        // (right/upは既に半径分のスケールがかかっている前提)
        void drawCircle3D(const tnl::Vector3& center, const tnl::Vector3& right, const tnl::Vector3& up, unsigned int color);

        // 衝突カテゴリごとに色を変える(見分けやすくするため)
        unsigned int colorForCategory(gmCollisionCategory category) const;

        static constexpr int CIRCLE_SEGMENTS = 16; // 円を描く際の分割数
    };
}
