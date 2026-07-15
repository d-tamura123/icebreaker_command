#include "gmOceanFlowLoader.h"
#include <DxLib.h>

namespace gm
{
    bool gmOceanFlowLoader::Load(const char* filePath,
        Vector2D outFlow[MAP_CHIP_HEIGHT][MAP_CHIP_WIDTH])
    {
        int fh = FileRead_open(filePath);
        if (fh == 0)
        {
            return false; // ì«Ç›çûÇ›é∏îs
        }

        // ocean_flow.bin ÇÕ 256Å~256 ÇÃ Vector2DÅifloatÅ~2ÅjÇ»ÇÃÇ≈ 512KB
        const int FLOW_SIZE = MAP_CHIP_WIDTH * MAP_CHIP_HEIGHT * sizeof(Vector2D);
        
        // DXLib ÇÃ FileRead_read ÇÕ memcpy Ç∆ìØÇ∂ãììÆ
        FileRead_read(outFlow, FLOW_SIZE, fh);

        FileRead_close(fh);
        return true;
    }
}
