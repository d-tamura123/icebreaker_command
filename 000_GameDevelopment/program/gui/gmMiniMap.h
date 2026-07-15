#pragma once
#include "gmUIObjectBase.h"
#include <memory>

namespace gm {

    class gmMapManager;
    class gmPlayerShip;

    class gmMiniMap : public gmUIObjectBase {
    public:
        gmMiniMap(
            const tnl::Vector2f& pos,
            std::shared_ptr<gmMapManager> map,
            std::shared_ptr<gmPlayerShip> player);

        void update(float dt) override;
        void draw() override;

    private:
        void drawBackground();
        void drawIslands();
        void drawPlayer();

    private:
        std::shared_ptr<gmMapManager> map_;
        std::shared_ptr<gmPlayerShip> player_;

        // DXLib のグラフィックハンドル
        int hBackground_ = -1;
        int hIsland_ = -1;
        int hPlayer_ = -1;

        static constexpr int MAP_SIZE = 256; // ミニマップの描画サイズ
    };
}
