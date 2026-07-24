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
            return false; // 読み込み失敗
        }

        // ocean_flow.bin は 256×256 の Vector2D（float×2）なので 512KB
        const int FLOW_SIZE = MAP_CHIP_WIDTH * MAP_CHIP_HEIGHT * sizeof(Vector2D);
        
        // DXLib の FileRead_read は memcpy と同じ挙動
        FileRead_read(outFlow, FLOW_SIZE, fh);

        FileRead_close(fh);

        // ------------------------------------------------------------
        // 3D空間は+Z方向が北、2Dのマップデータは行番号(y)が大きいほど南で、
        // 北と南の向きが逆になっている。
        // そのままだと海流の向きが意図と反対になってしまうため、
        // 読み込み時にY成分の符号を反転して3D空間の向きに合わせる。
        // ------------------------------------------------------------
        for (int y = 0; y < MAP_CHIP_HEIGHT; ++y)
        {
            for (int x = 0; x < MAP_CHIP_WIDTH; ++x)
            {
                outFlow[y][x].y = -outFlow[y][x].y;
            }
        }

        return true;
    }
}
