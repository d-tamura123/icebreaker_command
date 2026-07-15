#include "gmMapManager.h"
#include <vector>
#include <queue>
#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>   // std::min, std::max

namespace gm
{
    gmMapManager::gmMapManager()
        : flowField(oceanFlow)
        , playerStartCell(0, 0)
    {
    }

    bool gmMapManager::LoadMap(const char* path)
    {
        if (!mapLoader.Load(path, map))
            return false;

        AnalyzeMapFlags();
        return true;
    }

    bool gmMapManager::LoadOceanFlow(const char* path)
    {
        return oceanFlowLoader.Load(path, oceanFlow);
    }

    uint8_t gmMapManager::GetTile(int x, int y) const
    {
        if (x < 0 || x >= MAP_CHIP_WIDTH ||
            y < 0 || y >= MAP_CHIP_HEIGHT)
        {
            return 0;
        }
        return map[y][x];
    }

    bool gmMapManager::IsLand(int x, int y) const
    {
        return (GetTile(x, y) & 1) != 0;
    }

    // -------------------------
    // プレイヤー開始地点（bit1）
    // -------------------------
    tnl::Vector2i gmMapManager::GetPlayerStartCell() const
    {
        return playerStartCell;
    }

    tnl::Vector2f gmMapManager::GetPlayerStartWorld() const
    {
        // Note:
        // 2D->3D変換ロジック
        // 2D座標でY座標が小さいほど北になるためplayerStartCell.yにマイナスする
        return tnl::Vector2f(
            playerStartCell.x * CELL_SIZE,
            -playerStartCell.y * CELL_SIZE
        );
    }

    // -------------------------
    // NPC交易船スポーン（bit2）
    // -------------------------
    const std::vector<tnl::Vector2i>& gmMapManager::GetNpcTradeSpawnCells() const
    {
        return npcTradeSpawnCells;
    }

    std::vector<tnl::Vector2f> gmMapManager::GetNpcTradeSpawnWorld() const
    {
        std::vector<tnl::Vector2f> result;
        result.reserve(npcTradeSpawnCells.size());

        for (auto& c : npcTradeSpawnCells)
        {
            // Note:
            // 2D->3D変換ロジック
            // 2D座標でY座標が小さいほど北になるためc.yにマイナスする
            result.emplace_back(
                c.x * CELL_SIZE,
                -c.y * CELL_SIZE
            );
        }
        return result;
    }

    // -------------------------
    // 海流
    // -------------------------
    tnl::Vector2f gmMapManager::GetFlow(int x, int y) const
    {
        if (x < 0 || x >= MAP_CHIP_WIDTH ||
            y < 0 || y >= MAP_CHIP_HEIGHT)
        {
            return tnl::Vector2f(0, 0);
        }

        const Vector2D& v = oceanFlow[y][x];
        return tnl::Vector2f(v.x, v.y);
    }

    tnl::Vector2f gmMapManager::SampleFlowFloat(float fx, float fy) const
    {
        return flowField.SampleFloat(fx, fy);
    }

    // -------------------------
    // ビット解析
    //   【参考:ビットフラグ定義】
    //    -     --------------------------
    //    bit	用途
    //    -     --------------------------
    //    0     島
    //    1     プレイヤー開始地点
    //    2     NPC交易船スポーン
    //    -     --------------------------
    // -------------------------
    void gmMapManager::AnalyzeMapFlags()
    {
        npcTradeSpawnCells.clear();
        islands.clear();

        bool visited[MAP_CHIP_HEIGHT][MAP_CHIP_WIDTH] = {};

        for (int y = 0; y < MAP_CHIP_HEIGHT; ++y)
        {
            for (int x = 0; x < MAP_CHIP_WIDTH; ++x)
            {
                uint8_t v = map[y][x];

                if (v & 2)  // bit1: プレイヤー開始地点
                {
                    playerStartCell = tnl::Vector2i(x, y);
                }

                if (v & 4)  // bit2: NPC交易船スポーン
                {
                    npcTradeSpawnCells.emplace_back(x, y);
                }

                // 島（bit0）
                if ((v & 1) && !visited[y][x]) {

                    // flood fill で島領域を抽出
                    IslandInfo info;
                    info.cellMin = { x, y };
                    info.cellMax = { x, y };

                    std::queue<tnl::Vector2i> q;
                    q.push({ x, y });
                    visited[y][x] = true;

                    while (!q.empty()) {
                        auto c = q.front(); q.pop();

                        // bounding box 更新
                        info.cellMin.x = std::min(info.cellMin.x, c.x);
                        info.cellMin.y = std::min(info.cellMin.y, c.y);
                        info.cellMax.x = std::max(info.cellMax.x, c.x);
                        info.cellMax.y = std::max(info.cellMax.y, c.y);

                        // 4方向探索
                        const int dx[4] = { 1,-1,0,0 };
                        const int dy[4] = { 0,0,1,-1 };

                        for (int i = 0; i < 4; i++) {
                            int nx = c.x + dx[i];
                            int ny = c.y + dy[i];

                            if (nx < 0 || nx >= MAP_CHIP_WIDTH ||
                                ny < 0 || ny >= MAP_CHIP_HEIGHT)
                                continue;

                            if (!visited[ny][nx] && (map[ny][nx] & 1)) {
                                visited[ny][nx] = true;
                                q.push({ nx, ny });
                            }
                        }
                    }

                    // ワールド座標へ変換
                    info.width = (info.cellMax.x - info.cellMin.x + 1) * CELL_SIZE;
                    info.depth = (info.cellMax.y - info.cellMin.y + 1) * CELL_SIZE;

                    // 島の中心のワールド座標（Minの左端から、サイズの半分を進めた位置）
                    // Note:2D_Y座標->3D_Z座標変換では起点もオフセットもマイナスにする
                    info.worldX = info.cellMin.x * CELL_SIZE + (info.width * 0.5f);
                    info.worldZ = -info.cellMin.y * CELL_SIZE - (info.depth * 0.5f);

                    islands.push_back(info);
                }

            }
        }
    }
}
