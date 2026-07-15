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
    };
}
