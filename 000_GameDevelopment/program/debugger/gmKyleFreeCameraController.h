#pragma once

#include <dxe.h>
namespace gm {
    class gmKyleFreeCameraController {
    public:
        void update(Shared<dxe::Camera>& camera);

    private:
        float yaw_ = 0.0f;
        float pitch_ = 0.0f;
        float dist_ = 300.0f;
        tnl::Vector3 camTarget_;
    };
}