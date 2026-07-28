#pragma once
#include <dxe.h>

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

        // 識別ID
        const std::string& getId() const { return instanceID_; }

        // 生存フラグ
        bool isAlive() const { return alive_; }
        void kill() { alive_ = false; }

        // 更新・描画
        virtual void update(float deltaTime);
        virtual void draw();
        virtual void render(const Shared<dxe::Camera>& camera);

    protected:
        // インスタンス識別用
        std::string instanceID_;

        // 位置・トランスフォーム
        tnl::Vector3 position_  = {0.0f, 0.0f, 0.0f };
        tnl::Vector3 scale_     = {1.0f, 1.0f, 1.0f };
        tnl::Vector3 rotation_  = { 0.0f, 0.0f, 0.0f };

        // 生存フラグ
        bool alive_ = true;
    };
}
