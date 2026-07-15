#include "gmMapLoader.h"
#include <DxLib.h>

namespace gm
{
    bool gmMapLoader::Load(const char* filePath, uint8_t outMap[MAP_CHIP_HEIGHT][MAP_CHIP_WIDTH])
    {
        int fh = FileRead_open(filePath);
        if (fh == 0)
        {
            return false; // ì«Ç›çûÇ›é∏îs
        }

        // map.bin ÇÕ 256Å~256 ÇÃ 1byte îzóÒ
        const int MAP_SIZE = MAP_CHIP_WIDTH * MAP_CHIP_HEIGHT;

        // DXLib ÇÃ FileRead_read ÇÕ memcpy Ç∆ìØÇ∂ãììÆ
        FileRead_read(outMap, MAP_SIZE, fh);

        FileRead_close(fh);
        return true;
    }
}
