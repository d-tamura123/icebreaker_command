#include "gmCollider.h"


namespace gm {

    gmWorldShape gmCollider::toWorldShape(const tnl::Vector3& ownerPos, const tnl::Quaternion& ownerRot) const
    {
        gmWorldShape s;
        s.type = type;

        // ローカルオフセットは「所有者の向き」に合わせて回してからワールド座標に足す。
        // (船が回転すれば、船に取り付けたコライダーのオフセット位置も一緒に回るようにするため)
        tnl::Vector3 worldOffset = tnl::Vector3::TransformCoord(localOffset, ownerRot);
        tnl::Vector3 worldCenter = ownerPos + worldOffset;

        // コライダー自身が持つローカル回転と、所有者本体の回転を合成する
        tnl::Quaternion worldRot = localRotation * ownerRot;

        s.pos = worldCenter;
        s.rot = worldRot;

        switch (type) {
        case ColliderShapeType::Sphere:
            s.radius = radius;
            break;

        case ColliderShapeType::Box:
            // gmCollider側は「中心からの半分の長さ(halfExtents)」で持っているが、
            // 判定関数側は「全長」で受け取る規約になっているため、ここで2倍する。
            // (単位の取り違えは過去に何度もハマったポイントなので、変換はここに集約している)
            s.size = halfExtents * 2.0f;
            break;

        case ColliderShapeType::Ellipsoid:
            s.radii = ellipsoidRadii;
            break;

        case ColliderShapeType::Capsule: {
            // カプセルの軸は「ローカルY軸」という決め打ちにしている。
            // 別の向きのカプセルが欲しい場合は localRotation 側で調整する。
            tnl::Vector3 axis = tnl::Vector3::TransformCoord(tnl::Vector3(0.0f, 1.0f, 0.0f), worldRot);
            float halfLen = capsuleHeight * 0.5f;
            s.capsuleStart = worldCenter - axis * halfLen;
            s.capsuleEnd = worldCenter + axis * halfLen;
            s.radius = radius;
            break;
        }
        }

        return s;
    }
}
