#include "gmCollisionSystem.h"
#include "gmCollisionCategory.h"
#include "gmIntersectEx.h"
#include "../object/gmObjectBase.h"
#include <dxe.h>
#include <algorithm>

namespace gm {

    void gmCollisionSystem::registerObject(const std::weak_ptr<gmObjectBase>& obj)
    {
        objects_.push_back(obj);
    }

    void gmCollisionSystem::update()
    {
        // ------------------------------------------------------------
        // 死んでしまった(shared_ptrの実体が既に破棄された)登録を、
        // ここでまとめて取り除く。氷山のkill()のときと同様、
        // 呼び出し側が毎回手動で解除する必要が無いようにするための処理。
        // ------------------------------------------------------------
        objects_.erase(
            std::remove_if(objects_.begin(), objects_.end(),
                [](const std::weak_ptr<gmObjectBase>& w) { return w.expired(); }),
            objects_.end()
        );

        // ------------------------------------------------------------
        // 総当たり判定(ブロードフェーズ無し)。
        // 現状のオブジェクト数(船・島・流氷合わせて数十個程度)であれば
        // O(n^2)の総当たりでも性能上問題にならないはず。
        //
        // Note: isAlive()もあわせて見ているのは、kill()が呼ばれてから
        // 実際にリストから取り除かれるまで(各エンティティを保持する側の
        // 次回update()まで)最大1フレームのラグがあるため。
        // このラグの間に「死んでいるが登録上はまだ生きている」オブジェクトが
        // 同フレームに新規生成された別オブジェクトと衝突判定されてしまう
        // ケースがある(例: 氷山の分裂で、死にかけの元氷山と生まれたての
        // 破片がほぼ同座標に同居し、破片側が「衝突による位置差し戻し」で
        // 未初期化のlastSafePosition_(0,0,0)へ飛ばされる、など)。
        // ------------------------------------------------------------
        for (size_t i = 0; i < objects_.size(); ++i) {
            auto a = objects_[i].lock();
            if (!a || !a->isAlive()) continue;      // 冒頭コメントの件で、isAliveもあわせてチェックする

            for (size_t j = i + 1; j < objects_.size(); ++j) {
                auto b = objects_[j].lock();
                if (!b || !b->isAlive()) continue;  // 冒頭コメントの件で、isAliveもあわせてチェックする

                // カテゴリの組み合わせ的にそもそも判定不要なら早期スキップ
                if (!CanCollide(a->getCollisionCategory(), b->getCollisionCategory())) {
                    continue;
                }

                const tnl::Vector3 posA = a->getPosition();
                const tnl::Quaternion rotA = a->getRotationQuaternion();
                const tnl::Vector3 posB = b->getPosition();
                const tnl::Quaternion rotB = b->getRotationQuaternion();

                // 複合コライダー対応: どれか1組でも当たっていれば衝突とみなす
                bool hit = false;
                for (const auto& colA : a->getColliders()) {
                    for (const auto& colB : b->getColliders()) {
                        if (testPair(colA, posA, rotA, colB, posB, rotB)) {
                            hit = true;
                            break;
                        }
                    }
                    if (hit) break;
                }

                if (hit) {
                    a->onCollisionEnter(b.get());
                    b->onCollisionEnter(a.get());
                }
            }
        }
    }

    // ------------------------------------------------------------
    // コライダー1個 対 コライダー1個 の判定。
    // 型の組み合わせに応じて、tnl::intersect(既存のライブラリ)か
    // gmIntersectEx(今回追加したOBB-OBB/カプセル-カプセル/楕円球)の
    // どちらかへ振り分けるだけの役割。
    // ------------------------------------------------------------
    bool gmCollisionSystem::testPair(
        const gmCollider& colA, const tnl::Vector3& posA, const tnl::Quaternion& rotA,
        const gmCollider& colB, const tnl::Vector3& posB, const tnl::Quaternion& rotB) const
    {
        gmWorldShape a = colA.toWorldShape(posA, rotA);
        gmWorldShape b = colB.toWorldShape(posB, rotB);

        // ------------------------------------------------------------
        // 型の並び順を、常に「enum値が小さい方をa」に揃えておく。
        // (Sphere=0 < Box=1 < Capsule=2 < Ellipsoid=3 という順序)
        // こうしておくことで、以下のswitch文を型の数(4種類)だけ
        // 書けばよくなり、逆順(B-A)の組み合わせを別途書かずに済む。
        // ------------------------------------------------------------
        if (static_cast<int>(a.type) > static_cast<int>(b.type)) {
            std::swap(a, b);
        }

        using T = ColliderShapeType;

        switch (a.type) {
        case T::Sphere:
            switch (b.type) {
            case T::Sphere:
                return tnl::IsIntersectSphere(a.pos, a.radius, b.pos, b.radius);
            case T::Box:
                return tnl::IsIntersectSphereOBB(a.pos, a.radius, b.pos, b.size, b.rot);
            case T::Capsule:
                return tnl::IsIntersectCapsuleSphere(b.capsuleStart, b.capsuleEnd, b.radius, a.pos, a.radius);
            case T::Ellipsoid:
                return IsIntersectEllipsoidSphere(b.pos, b.radii, b.rot, a.pos, a.radius);
            }
            break;

        case T::Box:
            switch (b.type) {
            case T::Box:
                return IsIntersectOBB(a.pos, a.size, a.rot, b.pos, b.size, b.rot);
            case T::Capsule:
                return tnl::IsIntersectCapsuleOBB(b.capsuleStart, b.capsuleEnd, b.radius, a.pos, a.size, a.rot);
            case T::Ellipsoid:
                return IsIntersectEllipsoidOBB(b.pos, b.radii, b.rot, a.pos, a.size, a.rot);
            default:
                break;
            }
            break;

        case T::Capsule:
            switch (b.type) {
            case T::Capsule:
                return IsIntersectCapsuleCapsule(a.capsuleStart, a.capsuleEnd, a.radius, b.capsuleStart, b.capsuleEnd, b.radius);
            case T::Ellipsoid:
                return IsIntersectEllipsoidCapsule(b.pos, b.radii, b.rot, a.capsuleStart, a.capsuleEnd, a.radius);
            default:
                break;
            }
            break;

        case T::Ellipsoid:
            if (b.type == T::Ellipsoid) {
                return IsIntersectEllipsoidEllipsoid(a.pos, a.radii, a.rot, b.pos, b.radii, b.rot);
            }
            break;
        }

        // ここに来るのは型の組み合わせを網羅し忘れている場合のみ
        return false;
    }
}
