#pragma once
#include "gmShip.h"

namespace gm {

    class gmPlayerShip : public gmShip {
    public:
        gmPlayerShip(const std::string& id, const tnl::Vector3& pos)
            : gmShip(id, pos) {
        }

        void update(float deltaTime) override;

    private:
        void handleInput();
    };

}
