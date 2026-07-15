#pragma once
#include <vector>
#include <dxe.h>
#include "../gmGameConfig.h"
#include "gmMapLoader.h"
#include "gmOceanFlowLoader.h"
#include "gmFlowField.h"

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

        bool LoadMap(const char* path);
        bool LoadOceanFlow(const char* path);

        // --- 島リスト ---
        const std::vector<IslandInfo>& GetIslands() const { return islands; }

        // --- 地形 ---
        uint8_t GetTile(int x, int y) const;
        bool IsLand(int x, int y) const;

        // --- プレイヤー開始地点（bit1） ---
        // セル座標
        tnl::Vector2i GetPlayerStartCell() const;
        // 世界座標
        tnl::Vector2f GetPlayerStartWorld() const;

        // --- NPC交易船スポーン（bit2） ---
        // セル座標のリスト
        const std::vector<tnl::Vector2i>& GetNpcTradeSpawnCells() const;
        // 世界座標のリスト
        std::vector<tnl::Vector2f> GetNpcTradeSpawnWorld() const;

        // --- 海流 ---
        tnl::Vector2f GetFlow(int x, int y) const;
        tnl::Vector2f SampleFlowFloat(float fx, float fy) const;

    private:
        uint8_t map[MAP_CHIP_HEIGHT][MAP_CHIP_WIDTH];
        Vector2D oceanFlow[MAP_CHIP_HEIGHT][MAP_CHIP_WIDTH];

        gmMapLoader mapLoader;
        gmOceanFlowLoader oceanFlowLoader;
        gmFlowField flowField;

        std::vector<IslandInfo> islands;

        // --- 追加保持データ ---
        tnl::Vector2i playerStartCell;
        std::vector<tnl::Vector2i> npcTradeSpawnCells;

        // --- 内部処理 ---
        void AnalyzeMapFlags();

    };
}
