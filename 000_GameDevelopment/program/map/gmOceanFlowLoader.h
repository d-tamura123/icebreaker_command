#pragma once
#include <cstdint>
#include "../gmGameConfig.h"

namespace gm
{
    // ocean_flow_*.bin 専用ローダー
    // 1セル = float x + float y = 8byte
    // 256×256 = 512KB の生バイナリをそのまま読み込む
    struct Vector2D
    {
        float x;
        float y;
    };

    class gmOceanFlowLoader
    {
    public:
        // outFlow は Vector2D[MAP_CHIP_HEIGHT][MAP_CHIP_WIDTH] を渡す
        bool Load(const char* filePath, Vector2D outFlow[MAP_CHIP_HEIGHT][MAP_CHIP_WIDTH]);
    };
}
