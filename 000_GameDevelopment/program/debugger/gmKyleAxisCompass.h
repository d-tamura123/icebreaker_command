#pragma once
#include <dxe.h>
namespace gm {
    class gmKyleAxisCompass {
    public:
        gmKyleAxisCompass() = default;

        void draw(const Shared<dxe::Camera>& camera);

    };
}
