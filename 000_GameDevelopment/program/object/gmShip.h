#pragma once
#include "gmMeshBase.h"
#include "gmWaterPlane.h"
#include <dxe.h>

namespace gm {
    // gmShip.h
    class gmShip : public gm::gmMeshBase {
    public:
        // 運動状態を保持する変数群
        struct ShipDynamics final {
            float speed         = 0.0f;         // 現在速度
            float targetSpeed   = 0.0f;         // 目標速度（W/Sで段階変更）
            float rudder        = 0.0f;         // 現在舵角（-1.0〜1.0）
            float targetRudder  = 0.0f;         // 目標舵角（A/Dで段階変更）
            float yaw           = 0.0f;         // 船の向き
        };

        static constexpr float SPEED_LEVELS[6] = {
           -1.0f, -0.5f, 0.0f, 0.25f, 0.5f, 1.0f
        };

        tnl::Vector3 getForward() const {
            // Y軸回転（船の向き）から forward ベクトルを作る
            float yaw = dynamics_.yaw;
            return {
                sinf(yaw),
                0.0f,
                cosf(yaw)
            };
        }

    public:
        gmShip(const std::string& id, const tnl::Vector3& pos)
            : gmMeshBase(id, pos) {
        }

        void update(float deltaTime) override;                 // 入力・物理・状態更新
        void render(const Shared<dxe::Camera>& camera) override;

        void setWater(const std::shared_ptr<gmWaterPlane>& water);
        std::shared_ptr<gmWaterPlane> getWater() const;

        void setYaw(float yaw);
        float getYaw() const;

        // コライダー自動生成
        void setupDefaultCollider();

        // コライダーを手動指定で設定する(自動計算が信頼できない場合の代替経路)
        // arg1... カプセルの半径
        // arg2... カプセルの胴体長(半球キャップ分を除いた長さ)
        // arg3... 船の中心からの前後方向オフセット(ローカルZ、正で船首側)
        void setupManualCollider(float radius, float length, float forwardOffset = 0.0f);

        // 衝突検出イベント
        // 移動前の位置に丸ごと戻す(移動抑止)
        void onCollisionEnter(gmObjectBase* other) override;

    private:
        void updateEngine(float deltaTime);   // 速度段階・慣性
        void updateRudder(float deltaTime);   // 舵角
        void updateMovement(float deltaTime); // 前進・旋回
        void updateWave(float deltaTime);     // 波同期・傾き
        
    protected:
        ShipDynamics dynamics_;
        int speedIndex_ = 2; // 停止
        float waveTime_ = 0.0f;
        tnl::Vector3 wavePos_ {0, 0, 0};

        std::weak_ptr<gmWaterPlane> water_;

        // ---- コライダー調整用パラメータ ----
        // 実測で船首・船尾がカプセルに覆われていなかったため、
        // バウンディングボックスの前後長にかける倍率で余裕を持たせる。
        // 1.0で「半球キャップ分を差し引いた胴体長」、大きくするほど船首・船尾側に伸びる。
        static constexpr float SHIP_COLLIDER_LENGTH_SCALE = 1.4f;
    };
}
