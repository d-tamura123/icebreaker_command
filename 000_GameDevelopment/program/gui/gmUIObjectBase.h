#pragma once
#include <dxe.h>

namespace gm {
    class gmUIObjectBase {
    public:
        gmUIObjectBase(const tnl::Vector2f& pos);
        virtual ~gmUIObjectBase() = default;

        virtual void update(float dt) = 0;
        virtual void draw() = 0;

    protected:
        tnl::Vector2f position_;   // スクリーン座標
        float alpha_ = 1.0f;      // 透明度
    };
}