#pragma once
#include <vector>
#include <dxe.h>
#include "../gmGameConfig.h"
#include "gmMapLoader.h"
#include "gmOceanFlowLoader.h"
#include "gmFlowField.h"
#include "gmRouteLoader.h"

namespace gm
{
    class gmMapManager
    {
    public:
        gmMapManager();

        //  --- 島の情報 ---
        struct IslandInfo {
            tnl::Vector2i cellMin;   // 島の左上セル
            tnl::Vector2i cellMax;   // 島の右下セル
            float worldX;            // ワールド座標（中心）
            float worldZ;
            float width;             // 島の幅（world）
            float depth;             // 島の奥行き（world）
        };

        // --- 航路の情報（NPC交易船が追従する1本ぶんの経路） ---
        struct RouteInfo {
            // 始点(S)→終点(G)の順に並んだウェイポイントのセル座標。
            // (gmRouteLoaderのエンコード規則を参照)。
            std::vector<tnl::Vector2i> waypointCells;
        };

        // --- ロード関数 ---
        bool LoadMap(const char* path);
        bool LoadOceanFlow(const char* path);

        // 航路データを読み込む。
        // 
        // ROUTE_FILE_PATH_PREFIX + "1.bin", "2.bin", ... の順に読み込みを試み、
        // 読み込みに失敗した時点で走査を打ち切る(＝そのインデックスの航路ファイルが
        // 存在しない、という意味として扱う)。何本の航路が存在するかを別途持たず、
        // この「連番で読めるだけ読む」方式にしているため、Excel側でRoute_3等を
        // 追加しても、この関数・呼び出し元とも変更不要で追従できる。
        // 戻り値: 1本以上の航路を読み込めたらtrue
        bool LoadRoutes();


        // --- 島リスト ---
        const std::vector<IslandInfo>& GetIslands() const { return islands_; }

        // --- 地形 ---
        uint8_t GetTile(int x, int y) const;
        bool IsLand(int x, int y) const;

        // --- プレイヤー開始地点（bit1） ---
        // セル座標
        tnl::Vector2i GetPlayerStartCell() const;
        // 世界座標
        tnl::Vector2f GetPlayerStartWorld() const;

        // --- NPC交易船スポーン（bit2） ---
        
        // NOTE:
        // 現在はNPC交易船のスポーンには使用していない(スポーン地点は航路(RouteInfo)の
        // 両端(S/G)に統一された)。港街のようなビジュアル配置の目印座標として、
        // 将来使う可能性があるためデータ自体とAPIは残置している
        // (TODO: 余力があれば港街オブジェクトの配置に利用する)。

        // セル座標のリスト
        const std::vector<tnl::Vector2i>& GetNpcTradeSpawnCells() const;
        // 世界座標のリスト
        std::vector<tnl::Vector2f> GetNpcTradeSpawnWorld() const;


        // --- 航路 ---
        // 読み込めた航路の本数
        size_t GetRouteCount() const { return routes_.size(); }
        // セル座標のまま返す(routeIndexが範囲外なら空のRouteInfoを返す)
        const RouteInfo& GetRoute(size_t routeIndex) const;
        // ワールド座標へ変換したウェイポイント列を返す(routeIndexが範囲外なら空配列)
        std::vector<tnl::Vector2f> GetRouteWorldPoints(size_t routeIndex) const;

        // --- 海流 ---
        tnl::Vector2f GetFlow(int x, int y) const;
        tnl::Vector2f SampleFlowFloat(float fx, float fy) const;

    private:
        uint8_t map_[MAP_CHIP_HEIGHT][MAP_CHIP_WIDTH];
        Vector2D oceanFlow_[MAP_CHIP_HEIGHT][MAP_CHIP_WIDTH];

        gmMapLoader mapLoader_;
        gmOceanFlowLoader oceanFlowLoader_;
        gmFlowField flowField_;
        gmRouteLoader routeLoader_;

        std::vector<IslandInfo> islands_;
        std::vector<RouteInfo> routes_;

        // --- 追加保持データ ---
        tnl::Vector2i playerStartCell_;
        std::vector<tnl::Vector2i> npcTradeSpawnCells_;

        // --- 内部処理 ---
        void AnalyzeMapFlags();

    };
}
