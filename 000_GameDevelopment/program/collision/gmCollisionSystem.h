#pragma once
#include <vector>
#include <memory>
#include "gmCollider.h"


namespace gm {

    class gmObjectBase;

    // ------------------------------------------------------------
    // 衝突の「検出」だけを担当するマネージャー。
    // 検出後の応答(ダメージ処理・撃破処理等)は各オブジェクト自身の
    // onCollisionEnter() に委ねる(このクラスはあくまで検出役)。
    //
    // 登録はweak_ptrで保持する。氷山のkill()/erase()のときと同じ理由で、
    // 死んだオブジェクトの登録解除を毎回手動で行う必要が無いようにするため。
    // ------------------------------------------------------------
    class gmCollisionSystem {
    public:
        void registerObject(const std::weak_ptr<gmObjectBase>& obj);

        // 登録済みオブジェクトを総当たりで判定する
        // (ブロードフェーズ無しの単純実装。将来的に負荷が問題になったら
        //  海流可視化と同じグリッド分割の考え方で絞り込みを検討する)
        void update();

        // デバッグ表示(コライダーGizmo)用の読み取り専用アクセス
        const std::vector<std::weak_ptr<gmObjectBase>>& getObjects() const { return objects_; }

    private:
        // コライダー1個 対 コライダー1個 の判定(形状の組み合わせに応じて振り分ける)
        bool testPair(
            const gmCollider& colA, const tnl::Vector3& posA, const tnl::Quaternion& rotA,
            const gmCollider& colB, const tnl::Vector3& posB, const tnl::Quaternion& rotB) const;

        std::vector<std::weak_ptr<gmObjectBase>> objects_;
    };
}
