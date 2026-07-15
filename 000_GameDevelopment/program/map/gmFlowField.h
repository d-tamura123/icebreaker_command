#pragma once
#include <cstdint>
#include "../gmGameConfig.h"
#include <dxe.h>
#include "gmOceanFlowLoader.h" // Vector2D の定義を参照

namespace gm
{
    // 海流ベクトル補間ユーティリティ
    class gmFlowField
    {
    public:
        // oceanFlow は MapManager が保持している 256x256 の生配列
        gmFlowField(const Vector2D(*flow)[MAP_CHIP_WIDTH]);

        // 整数座標（マップ座標）から海流ベクトルを取得
        tnl::Vector2f SampleInt(int x, int y) const;

        // 浮動小数点座標（滑らかな移動用）から補間して取得
        tnl::Vector2f SampleFloat(float fx, float fy) const;

    private:
        const Vector2D(*m_flow)[MAP_CHIP_WIDTH];

        // 内部補助：バイリニア補間
        tnl::Vector2f Bilinear(
            const Vector2D& v00,
            const Vector2D& v10,
            const Vector2D& v01,
            const Vector2D& v11,
            float tx, float ty) const;
    };
}
