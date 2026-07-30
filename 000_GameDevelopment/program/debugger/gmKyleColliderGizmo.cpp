#include "gmKyleColliderGizmo.h"
#include "../collision/gmCollisionSystem.h"
#include "../collision/gmCollider.h"
#include "../object/gmObjectBase.h"
#include <DxLib.h>
#include <cmath>

namespace gm {

    void gmKyleColliderGizmo::render(const std::shared_ptr<gmCollisionSystem>& collisionSystem, const Shared<dxe::Camera>& camera)
    {
        if (!collisionSystem) return;

        // --- カメラ行列を DXLib にセット(gmKyleGridViewerと同じ手順) ---
        MATRIX view, proj;
        memcpy(view.m, camera->getViewMatrix().m, sizeof(float) * 16);
        memcpy(proj.m, camera->getProjectionMatrix().m, sizeof(float) * 16);
        SetCameraViewMatrix(view);
        SetupCamera_ProjectionMatrix(proj);

        // --- ワールド行列を Identity に ---
        MATRIX im;
        CreateIdentityMatrix(&im);
        SetTransformToWorld(&im);

        // 登録済みオブジェクトを総当たりで走査し、各コライダーを描画する
        // (判定処理とは無関係。あくまで見た目の確認用)
        for (const auto& weakObj : collisionSystem->getObjects()) {
            auto obj = weakObj.lock();
            if (!obj) continue;

            const unsigned int color = colorForCategory(obj->getCollisionCategory());
            const tnl::Vector3 ownerPos = obj->getPosition();
            const tnl::Quaternion ownerRot = obj->getRotationQuaternion();

            for (const auto& collider : obj->getColliders()) {
                // gmColliderが既に持っている変換ロジック(toWorldShape)をそのまま使う。
                // ここでワールド座標への変換式を重複して書かないようにするため。
                gmWorldShape shape = collider.toWorldShape(ownerPos, ownerRot);

                switch (shape.type) {
                case ColliderShapeType::Sphere:
                    drawSphere(shape.pos, shape.radius, color);
                    break;
                case ColliderShapeType::Box:
                    drawBox(shape.pos, shape.size, shape.rot, color);
                    break;
                case ColliderShapeType::Capsule:
                    drawCapsule(shape.capsuleStart, shape.capsuleEnd, shape.radius, color);
                    break;
                case ColliderShapeType::Ellipsoid:
                    drawEllipsoid(shape.pos, shape.radii, shape.rot, color);
                    break;
                }
            }
        }
    }

    // ------------------------------------------------------------
    // 任意平面上に円を描く共通ヘルパー
    // (center を中心に、right方向・up方向へ回転させながら線分を繋いでいく)
    // ------------------------------------------------------------
    void gmKyleColliderGizmo::drawCircle3D(const tnl::Vector3& center, const tnl::Vector3& right, const tnl::Vector3& up, unsigned int color)
    {
        tnl::Vector3 prev = center + right; // 角度0(cos=1, sin=0)の点から開始

        for (int i = 1; i <= CIRCLE_SEGMENTS; ++i) {
            const float t = (2.0f * tnl::PI) * static_cast<float>(i) / static_cast<float>(CIRCLE_SEGMENTS);
            tnl::Vector3 cur = center + right * std::cos(t) + up * std::sin(t);

            DrawLine3D(VGet(prev.x, prev.y, prev.z), VGet(cur.x, cur.y, cur.z), color);
            prev = cur;
        }
    }

    // ------------------------------------------------------------
    // 球: 3枚の直交する円(XY/YZ/XZ相当)を重ねて描くことで球らしく見せる
    // ------------------------------------------------------------
    void gmKyleColliderGizmo::drawSphere(const tnl::Vector3& pos, float radius, unsigned int color)
    {
        const tnl::Vector3 x{ radius, 0.0f, 0.0f };
        const tnl::Vector3 y{ 0.0f, radius, 0.0f };
        const tnl::Vector3 z{ 0.0f, 0.0f, radius };

        drawCircle3D(pos, x, y, color);
        drawCircle3D(pos, y, z, color);
        drawCircle3D(pos, x, z, color);
    }

    // ------------------------------------------------------------
    // 楕円球: 球と同じ3枚の円だが、各軸の長さがradii(各軸ごとに異なる)になる
    // ------------------------------------------------------------
    void gmKyleColliderGizmo::drawEllipsoid(const tnl::Vector3& pos, const tnl::Vector3& radii, const tnl::Quaternion& rot, unsigned int color)
    {
        const tnl::Vector3 axisX = tnl::Vector3::TransformCoord(tnl::Vector3(radii.x, 0.0f, 0.0f), rot);
        const tnl::Vector3 axisY = tnl::Vector3::TransformCoord(tnl::Vector3(0.0f, radii.y, 0.0f), rot);
        const tnl::Vector3 axisZ = tnl::Vector3::TransformCoord(tnl::Vector3(0.0f, 0.0f, radii.z), rot);

        drawCircle3D(pos, axisX, axisY, color);
        drawCircle3D(pos, axisY, axisZ, color);
        drawCircle3D(pos, axisX, axisZ, color);
    }

    // ------------------------------------------------------------
    // Box(直方体・立方体): 8つの頂点を求め、12本の辺を線で結ぶ
    // ------------------------------------------------------------
    void gmKyleColliderGizmo::drawBox(const tnl::Vector3& pos, const tnl::Vector3& size, const tnl::Quaternion& rot, unsigned int color)
    {
        const tnl::Vector3 half = size * 0.5f;

        const tnl::Vector3 axisX = tnl::Vector3::TransformCoord(tnl::Vector3(half.x, 0.0f, 0.0f), rot);
        const tnl::Vector3 axisY = tnl::Vector3::TransformCoord(tnl::Vector3(0.0f, half.y, 0.0f), rot);
        const tnl::Vector3 axisZ = tnl::Vector3::TransformCoord(tnl::Vector3(0.0f, 0.0f, half.z), rot);

        // 8頂点をsx,sy,szの符号(-1/+1)の全組み合わせで求める
        // corners[0]=(---) corners[1]=(--+) corners[2]=(-+-) corners[3]=(-++)
        // corners[4]=(+--) corners[5]=(+-+) corners[6]=(++-) corners[7]=(+++)
        tnl::Vector3 corners[8];
        int idx = 0;
        for (int sx = -1; sx <= 1; sx += 2) {
            for (int sy = -1; sy <= 1; sy += 2) {
                for (int sz = -1; sz <= 1; sz += 2) {
                    corners[idx++] = pos
                        + axisX * static_cast<float>(sx)
                        + axisY * static_cast<float>(sy)
                        + axisZ * static_cast<float>(sz);
                }
            }
        }

        auto line = [&](int a, int b) {
            DrawLine3D(
                VGet(corners[a].x, corners[a].y, corners[a].z),
                VGet(corners[b].x, corners[b].y, corners[b].z),
                color);
            };

        // sx=-1側の面(4辺)
        line(0, 1); line(1, 3); line(3, 2); line(2, 0);
        // sx=+1側の面(4辺)
        line(4, 5); line(5, 7); line(7, 6); line(6, 4);
        // 2つの面をつなぐ辺(4本)
        line(0, 4); line(1, 5); line(2, 6); line(3, 7);
    }

    // ------------------------------------------------------------
    // カプセル: 両端(始点・終点)に円を描き、円周上の4点を線でつないで
    // 胴体を表現する(半球のふくらみは省略した簡易表示)
    // ------------------------------------------------------------
    void gmKyleColliderGizmo::drawCapsule(const tnl::Vector3& st, const tnl::Vector3& en, float radius, unsigned int color)
    {
        tnl::Vector3 axisVec = en - st;
        const float len = axisVec.length();
        const tnl::Vector3 axisDir = (len > 1e-5f) ? tnl::Vector3::Normalize(axisVec) : tnl::Vector3(0.0f, 1.0f, 0.0f);

        // 軸(axisDir)に垂直な2本の単位ベクトルを、適当な基準ベクトルとの外積から作る
        const tnl::Vector3 arbitrary = (std::abs(axisDir.y) < 0.9f) ? tnl::Vector3(0.0f, 1.0f, 0.0f) : tnl::Vector3(1.0f, 0.0f, 0.0f);
        const tnl::Vector3 rightDir = tnl::Vector3::Normalize(tnl::Vector3::Cross(axisDir, arbitrary));
        const tnl::Vector3 upDir = tnl::Vector3::Normalize(tnl::Vector3::Cross(axisDir, rightDir));

        // 両端に円を描く
        drawCircle3D(st, rightDir * radius, upDir * radius, color);
        drawCircle3D(en, rightDir * radius, upDir * radius, color);

        // 胴体部分: 円周上4点を始点側・終点側で結ぶ縦線
        for (int i = 0; i < 4; ++i) {
            const float t = (tnl::PI * 0.5f) * static_cast<float>(i);
            const tnl::Vector3 offset = rightDir * (radius * std::cos(t)) + upDir * (radius * std::sin(t));

            const tnl::Vector3 a = st + offset;
            const tnl::Vector3 b = en + offset;
            DrawLine3D(VGet(a.x, a.y, a.z), VGet(b.x, b.y, b.z), color);
        }
    }

    // ------------------------------------------------------------
    // 衝突カテゴリごとに色分けする(見た目だけで大まかに種類が分かるように)
    // ------------------------------------------------------------
    unsigned int gmKyleColliderGizmo::colorForCategory(gmCollisionCategory category) const
    {
        switch (category) {
        case gmCollisionCategory::Ship:       return 0xff00ffff; // シアン
        case gmCollisionCategory::Island:     return 0xff00ff00; // 緑
        case gmCollisionCategory::Iceberg:    return 0xffffffff; // 白
        case gmCollisionCategory::Projectile: return 0xffff8000; // オレンジ
        default:                              return 0xff808080; // グレー(None、通常表示されないはず)
        }
    }
}
