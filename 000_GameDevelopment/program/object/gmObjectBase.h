#pragma once
#include <dxe.h>
#include <vector>
#include "../collision/gmCollider.h"
#include "../collision/gmCollisionCategory.h"

namespace gm {

    class gmObjectBase {
    public:
        gmObjectBase(const std::string& instanceID, const tnl::Vector3& position);
        virtual ~gmObjectBase() = default;

        // 座標
        const tnl::Vector3& getPosition() const;
        void setPosition(const tnl::Vector3& position);

        // スケール
        const tnl::Vector3& getScale() const;
        void setScale(const tnl::Vector3& scale);

        // 回転(Euler角、ラジアン)
        // 用途: 分裂で生まれた氷山など、生成直後から親の向きを引き継ぎたい場合に使う
        const tnl::Vector3& getRotation() const { return rotation_; }
        void setRotation(const tnl::Vector3& rotation) { rotation_ = rotation; }

        // 識別ID
        const std::string& getId() const { return instanceID_; }

        // 生存フラグ
        bool isAlive() const { return alive_; }
        void kill() { alive_ = false; }

        // 更新・描画
        virtual void update(float deltaTime);
        virtual void draw();
        virtual void render(const Shared<dxe::Camera>& camera);

        // Euler角(rotation_)からクォータニオンへ変換して取得する
        // Note:
        //  gmMeshBase::render()内の変換と共通のロジック。
        //  コライダー判定などで使用
        tnl::Quaternion getRotationQuaternion() const;

        // ---- コライダー ----
        // 1つのオブジェクトに複数のコライダーを持たせられる(複合コライダー対応)
        void addCollider(const gmCollider& collider) { colliders_.push_back(collider); }
        const std::vector<gmCollider>& getColliders() const { return colliders_; }

        // ---- 衝突カテゴリ ----
        void setCollisionCategory(gmCollisionCategory category) { category_ = category; }
        gmCollisionCategory getCollisionCategory() const { return category_; }

        // 衝突が検出されたときに呼ばれる(デフォルトは何もしない。必要な派生クラスだけoverrideする)
        virtual void onCollisionEnter(gmObjectBase* other) {}


        // ---- 移動抑止用の位置スナップショット ----
        // 「移動前の位置を退避しておき、衝突していたら丸ごと元に戻す」方式。
        // 各update()の先頭でsnapshotPosition()を呼び、
        // onCollisionEnter()内でresolveCollisionOrRevert()を呼ぶことで移動抑止する。
        void snapshotPosition() { lastSafePosition_ = position_; }
        void revertToLastSafePosition() { position_ = lastSafePosition_; }

        // ---- 衝突時の移動抑止(距離改善判定つき) ----
        // 衝突相手(other)から見て、今回の移動(=現在のposition_)が直前の安全な位置
        // (lastSafePosition_)より相手から離れる方向であれば、その移動をそのまま許可する
        // (=何もしない)。逆に相手へ近づく方向であれば、revertToLastSafePosition()と同様に
        // 移動前の位置へ差し戻す。
        //
        // 経緯: 単純にrevertToLastSafePosition()だけを使うと、lastSafePosition_自体が
        // 既に衝突状態で記録されてしまっていた場合(分裂直後の氷山フラグメントが船と重なった
        // 位置で生成された等)、「差し戻し→また衝突→差し戻し」のループでスタックし続ける
        // ことがあった。かといって押し出し(瞬間移動)で解決しようとすると、稀に発動した瞬間、
        // プレイヤーから見て唐突なワープに見えてしまい、手触りを損ねた(実機検証で判明)。
        // 「重なった状態から抜け出そうとする移動は常に受け入れる」という、距離の改善有無だけを
        // 見るこの方式であれば、瞬間移動を一切発生させずに、原理的にスタックを防げる
        // (プレイヤー自身の操作で自然に離れていく)。
        //
        // arg1... 衝突相手(nullptrの場合は無条件にrevertToLastSafePosition()する)
        void resolveCollisionOrRevert(const gmObjectBase* other);

    protected:
        // インスタンス識別用
        std::string instanceID_;

        // 位置・トランスフォーム
        tnl::Vector3 position_  = {0.0f, 0.0f, 0.0f };
        tnl::Vector3 scale_     = {1.0f, 1.0f, 1.0f };
        tnl::Vector3 rotation_  = { 0.0f, 0.0f, 0.0f };

        // 生存フラグ
        bool alive_ = true;

        // コライダー・衝突カテゴリ
        std::vector<gmCollider> colliders_;
        gmCollisionCategory category_ = gmCollisionCategory::None;

        // 移動抑止用: snapshotPosition()で退避した「移動前の安全な位置」
        tnl::Vector3 lastSafePosition_ = { 0.0f, 0.0f, 0.0f };
    };
}
