#include "gmMapManager.h"
#include <vector>
#include <queue>
#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>   // std::min, std::max

namespace gm
{
    gmMapManager::gmMapManager()
        : map_{}                    // C26495対策: 生配列は明示的にゼロ初期化しないと未初期化警告が出るため
        , oceanFlow_{}              // 同上
        , flowField_(oceanFlow_)
        , playerStartCell_(0, 0)
    {
    }

    bool gmMapManager::LoadMap(const char* path)
    {
        if (!mapLoader_.Load(path, map_))
            return false;

        AnalyzeMapFlags();
        return true;
    }

    bool gmMapManager::LoadOceanFlow(const char* path)
    {
        return oceanFlowLoader_.Load(path, oceanFlow_);
    }

    // ------------------------------------------------------------
    // ROUTE_FILE_PATH_PREFIX + "1.bin", "2.bin", ... の順に読み込みを試み、
    // 読み込みに失敗した時点で走査を打ち切る。
    // 「何本の航路が存在するか」を別ファイルやマジックナンバーで管理せず、
    // 連番で読めるだけ読む方式にすることで、Excel側でRoute_3を追加しても
    // この関数の変更が不要になる(拡張時の手戻りを避ける狙い)。
    // ------------------------------------------------------------
    bool gmMapManager::LoadRoutes()
    {
        routes_.clear();

        int routeIndex = 1;
        for (;;)
        {
            char filePath[256];
            sprintf_s(filePath, "%s%d.bin", ROUTE_FILE_PATH_PREFIX, routeIndex);

            RouteInfo info;
            if (!routeLoader_.Load(filePath, info.waypointCells))
            {
                break; // このインデックスの航路ファイルが存在しない＝ここで走査終了
            }

            routes_.push_back(info);
            ++routeIndex;
        }

        return !routes_.empty();
    }



    uint8_t gmMapManager::GetTile(int x, int y) const
    {
        if (x < 0 || x >= MAP_CHIP_WIDTH ||
            y < 0 || y >= MAP_CHIP_HEIGHT)
        {
            return 0;
        }
        return map_[y][x];
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
        return playerStartCell_;
    }

    tnl::Vector2f gmMapManager::GetPlayerStartWorld() const
    {
        // Note:
        // 2D->3D変換ロジック
        // 2D座標でY座標が小さいほど北になるためplayerStartCell.yにマイナスする
        return tnl::Vector2f(
            playerStartCell_.x * CELL_SIZE,
            -playerStartCell_.y * CELL_SIZE
        );
    }

    // -------------------------
    // NPC交易船スポーン（bit2）
    // TODO: 現在はスポーンに未使用。港街ビジュアル配置用の目印として残置。
    // -------------------------
    const std::vector<tnl::Vector2i>& gmMapManager::GetNpcTradeSpawnCells() const
    {
        return npcTradeSpawnCells_;
    }

    std::vector<tnl::Vector2f> gmMapManager::GetNpcTradeSpawnWorld() const
    {
        std::vector<tnl::Vector2f> result;
        result.reserve(npcTradeSpawnCells_.size());

        for (auto& c : npcTradeSpawnCells_)
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
    // 航路
    // -------------------------
    const gmMapManager::RouteInfo& gmMapManager::GetRoute(size_t routeIndex) const
    {
        static const RouteInfo emptyRoute; // 範囲外アクセス時に返す空のルート(waypointCellsが空)

        if (routeIndex >= routes_.size())
        {
            return emptyRoute;
        }
        return routes_[routeIndex];
    }

    std::vector<tnl::Vector2f> gmMapManager::GetRouteWorldPoints(size_t routeIndex) const
    {
        std::vector<tnl::Vector2f> result;

        if (routeIndex >= routes_.size())
        {
            return result; // 範囲外は空配列を返す(呼び出し元でのクラッシュを避ける)
        }

        const std::vector<tnl::Vector2i>& cells = routes_[routeIndex].waypointCells;
        result.reserve(cells.size());

        for (const auto& c : cells)
        {
            // Note:
            // 2D->3D変換ロジック(他のセル→ワールド変換と同じ規則)
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

        const Vector2D& v = oceanFlow_[y][x];
        return tnl::Vector2f(v.x, v.y);
    }

    tnl::Vector2f gmMapManager::SampleFlowFloat(float fx, float fy) const
    {
        return flowField_.SampleFloat(fx, fy);
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
        npcTradeSpawnCells_.clear();
        islands_.clear();

        // 島探索で処理済みセルを記録するフラグ 
        // インデックスは y * MAP_CHIP_WIDTH + x で1次元化してアクセスする。
        //
        // Note: 要素型はboolではなくuint8_tを使う。
        // std::vector<bool>はビット詰めのための特殊仕様を持ち
        // (operator[]がbool&ではなく専用のproxyオブジェクトを返す等)、
        // 他のvector<T>と挙動が異なる罠として知られているため、あえて避けている
        // (この関数はマップ読み込み時に1回しか呼ばれないため、ビット詰めによる
        // メモリ節約のメリットよりも、素直な挙動のvectorである方を優先する)。
        std::vector<uint8_t> visited(static_cast<size_t>(MAP_CHIP_HEIGHT) * MAP_CHIP_WIDTH, 0);

        for (int y = 0; y < MAP_CHIP_HEIGHT; ++y)
        {
            for (int x = 0; x < MAP_CHIP_WIDTH; ++x)
            {
                uint8_t v = map_[y][x];

                if (v & 2)  // bit1: プレイヤー開始地点
                {
                    playerStartCell_ = tnl::Vector2i(x, y);
                }

                if (v & 4)  // bit2: NPC交易船スポーン(TODO: 港街ビジュアル配置用の目印として利用予定)
                {
                    npcTradeSpawnCells_.emplace_back(x, y);
                }

                // 島（bit0）
                const size_t visitedIndex = static_cast<size_t>(y) * MAP_CHIP_WIDTH + x;
                if ((v & 1) && !visited[visitedIndex]) {

                    // flood fill で島領域を抽出
                    IslandInfo info;
                    info.cellMin = { x, y };
                    info.cellMax = { x, y };

                    std::queue<tnl::Vector2i> q;
                    q.push({ x, y });
                    visited[visitedIndex] = true;

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

                            const size_t neighborIndex = static_cast<size_t>(ny) * MAP_CHIP_WIDTH + nx;
                            if (!visited[neighborIndex] && (map_[ny][nx] & 1)) {
                                visited[neighborIndex] = true;
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

                    islands_.push_back(info);
                }
            }
        }
    }
}
