#pragma once
#include <dxe.h>


namespace gm {

    // コライダーの形状の種類
    // ※以下の判定処理(gmCollisionSystem::testPair)は、この並び順
    //   (Sphere < Box < Capsule < Ellipsoid)を前提に組んであるので、
    //   並び順を変える場合は判定処理側も合わせて見直すこと。
    enum class ColliderShapeType {
        Sphere,
        Box,
        Capsule,
        Ellipsoid,
    };

    // ------------------------------------------------------------
    // 交差判定関数へそのまま渡せる、ワールド座標系に変換済みの形状データ。
    // gmCollider(所有者からの相対値で持つ設定データ)と、
    // 実際の判定に使うデータを分けておくことで、
    // 「設定」と「毎フレームの計算結果」の役割を混同しないようにしている。
    // ------------------------------------------------------------
    struct gmWorldShape {
        ColliderShapeType type = ColliderShapeType::Sphere;

        tnl::Vector3 pos{ 0.0f, 0.0f, 0.0f };  // Sphere / Box / Ellipsoid の中心
        tnl::Quaternion rot;                    // Box / Ellipsoid の姿勢

        float radius = 0.0f;                    // Sphere / Capsule の半径
        tnl::Vector3 size{ 0.0f, 0.0f, 0.0f };  // Box の全長サイズ(半分の長さではない点に注意)
        tnl::Vector3 radii{ 0.0f, 0.0f, 0.0f }; // Ellipsoid の各軸半径

        tnl::Vector3 capsuleStart{ 0.0f, 0.0f, 0.0f }; // Capsule の始点(ワールド座標)
        tnl::Vector3 capsuleEnd{ 0.0f, 0.0f, 0.0f };   // Capsule の終点(ワールド座標)
    };

    // ------------------------------------------------------------
    // オブジェクトに持たせる、当たり判定の形状定義(Unityのコライダーに相当)。
    // gmObjectBase::colliders_ に複数持たせることで、複合コライダー
    // (例: 細長い島を複数のBoxで表現する等)にも対応できる。
    //
    // 値はすべて「所有者(gmObjectBase)から見た相対値」で持つ。
    // 実際のワールド座標での判定に使う値が欲しい場合は toWorldShape() を呼ぶ。
    // ------------------------------------------------------------
    struct gmCollider {
        ColliderShapeType type = ColliderShapeType::Sphere;

        // 所有者のposition_/回転からの相対オフセット・相対回転
        tnl::Vector3 localOffset{ 0.0f, 0.0f, 0.0f };
        tnl::Quaternion localRotation;

        // ---- 以下、typeに応じて使うパラメータだけ設定すればよい ----
        float radius = 50.0f;                       // Sphere / Capsule の半径
        float capsuleHeight = 0.0f;                  // Capsule の胴体部分の長さ(カプセルの軸はローカルY軸固定)
        tnl::Vector3 halfExtents{ 50.0f, 50.0f, 50.0f };    // Box の中心からの半分の長さ
        tnl::Vector3 ellipsoidRadii{ 50.0f, 50.0f, 50.0f }; // Ellipsoid の各軸半径

        // 所有者のワールド座標・姿勢(クォータニオン)を渡し、
        // 判定関数へそのまま渡せるワールド空間の形状データに変換する
        gmWorldShape toWorldShape(const tnl::Vector3& ownerPos, const tnl::Quaternion& ownerRot) const;
    };
}
