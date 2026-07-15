#pragma once
#include <cstdint>
#include "../gmGameConfig.h"

namespace gm
{
    // map.bin 専用ローダー
    class gmMapLoader
    {
    public:
        // outMap は uint8_t[MAP_H][MAP_W] を渡す
        bool Load(const char* filePath, uint8_t outMap[MAP_CHIP_HEIGHT][MAP_CHIP_WIDTH]);
    };
}
