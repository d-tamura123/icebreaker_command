#pragma once
#include <dxe.h>

namespace gm {

    class gmKyleWorldRuler {
    public:
        gmKyleWorldRuler() = default;

        // origin: 世界座標
        // unit: 目盛り間隔（例：10.0f）
        // length: +方向 / -方向の長さ（例：100.0f）
        void draw(const Shared<dxe::Camera>& camera,
            const tnl::Vector3& origin,
            float unit = 50.0f,
            float length = 500.0f);

    };

}
