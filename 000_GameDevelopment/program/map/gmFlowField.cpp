#include "gmFlowField.h"
#include <algorithm>

namespace gm
{
    gmFlowField::gmFlowField(const Vector2D(*flow)[MAP_CHIP_WIDTH])
        : m_flow(flow)
    {
    }

    // 整数座標からそのまま取得
    tnl::Vector2f gmFlowField::SampleInt(int x, int y) const
    {
        if (x < 0 || x >= MAP_CHIP_WIDTH ||
            y < 0 || y >= MAP_CHIP_HEIGHT)
        {
            return tnl::Vector2f(0.0f, 0.0f);
        }

        const Vector2D& v = m_flow[y][x];
        return tnl::Vector2f(v.x, v.y);
    }

    // 浮動小数点座標から補間して取得
    tnl::Vector2f gmFlowField::SampleFloat(float fx, float fy) const
    {
        // マップ外ならゼロ
        if (fx < 0.0f || fx >= MAP_CHIP_WIDTH - 1 ||
            fy < 0.0f || fy >= MAP_CHIP_HEIGHT - 1)
        {
            return tnl::Vector2f(0.0f, 0.0f);
        }

        int x0 = static_cast<int>(fx);
        int y0 = static_cast<int>(fy);
        int x1 = x0 + 1;
        int y1 = y0 + 1;

        float tx = fx - x0;
        float ty = fy - y0;

        const Vector2D& v00 = m_flow[y0][x0];
        const Vector2D& v10 = m_flow[y0][x1];
        const Vector2D& v01 = m_flow[y1][x0];
        const Vector2D& v11 = m_flow[y1][x1];

        return Bilinear(v00, v10, v01, v11, tx, ty);
    }

    // バイリニア補間
    tnl::Vector2f gmFlowField::Bilinear(
        const Vector2D& v00,
        const Vector2D& v10,
        const Vector2D& v01,
        const Vector2D& v11,
        float tx, float ty) const
    {
        float x =
            (1 - tx) * (1 - ty) * v00.x +
            (tx) * (1 - ty) * v10.x +
            (1 - tx) * (ty)*v01.x +
            (tx) * (ty)*v11.x;

        float y =
            (1 - tx) * (1 - ty) * v00.y +
            (tx) * (1 - ty) * v10.y +
            (1 - tx) * (ty)*v01.y +
            (tx) * (ty)*v11.y;

        return tnl::Vector2f(x, y);
    }
}
