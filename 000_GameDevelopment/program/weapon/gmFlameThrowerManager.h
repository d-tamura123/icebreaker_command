// gmFlameThrowerManager.h
#pragma once
#include <dxe.h>
#include <vector>
#include <memory>
#include "../object/gmFlameThrowerAttack.h"

namespace gm {

    class gmCollisionSystem;
    class gmSpriteAnimRegistry;
    class gmShip;

    // ------------------------------------------------------------
    // 火炎放射攻撃(gmFlameThrowerAttack)の発動・毎フレーム更新・描画・
    // 後片付けを一元管理する。gmProjectileManagerと同じパターン
    // (登録して、毎フレームupdate/renderを呼ぶだけ)。
    //
    // 「発射位置・扇の中心方向」をどう決めるか(船のどちら側面から出すか等)は、
    // 弾道兵器(gmProjectileManager::fire)が着弾目標をそのまま受け取るのとは違い、
    // 船の状態(位置・向き)とクリック位置から、このクラス側で計算する。
    // 
    // 発動中、発射位置(発射起点)は船の現在位置に毎フレーム追従する
    // (扇の向き自体は発動時のまま固定。gmFlameThrowerAttack::updateOrigin参照)。
    // そのため発動した船への参照(activeShip_)を保持しておく。
    // ------------------------------------------------------------
    class gmFlameThrowerManager {
    public:
        gmFlameThrowerManager(
            const std::shared_ptr<gmCollisionSystem>& collisionSystem,
            const std::shared_ptr<gmSpriteAnimRegistry>& spriteRegistry);

        // 火炎放射を1回発動する。
        // arg1... 発動する船本体。発動中はこの船の現在位置に発射起点を追従させる
        // arg2... クリックした狙い位置(海面上の座標)。船から見てどちら側面か、
        //         扇の中心方向をどちらへ向けるかの両方をこの位置から決める。
        //
        // 既に発動中の火炎放射がある場合は何もしない(同時多重発動はしない、簡易ロック)。
        // TODO: 武器選択UI実装後、クールタイム等のちゃんとした発射制御に置き換える想定。
        void fire(const std::shared_ptr<gmShip>& ship, const tnl::Vector3& aimTargetPos);

        void update(float deltaTime);
        void render(const Shared<dxe::Camera>& camera);

        // 現在発動中(生存中)の火炎放射があるかどうか
        bool isActive() const;

    private:
        std::shared_ptr<gmCollisionSystem>    collisionSystem_;
        std::shared_ptr<gmSpriteAnimRegistry> spriteRegistry_;
        std::vector<std::shared_ptr<gmFlameThrowerAttack>> attacks_; // 発動中の火炎放射(基本的に同時に0〜1個)

        // 発動中の攻撃が追従する対象の船。発動時に決めた側面(activeSideSign_)を
        // 保ったまま、毎フレームこの船の現在位置から発射起点を再計算する。
        std::weak_ptr<gmShip> activeShip_;
        float activeSideSign_ = 1.0f;               // 発動時にどちら側面から出したか(+1: 右舷, -1: 左舷)

        int nextId_ = 0;

        // ---- 発射位置(ハードポイント)の調整用パラメータ ----
        static constexpr float SIDE_OFFSET = 30.0f; // 船の中心から左右どちらかの側面へ、発射位置をずらす量(world単位)
    };
}
