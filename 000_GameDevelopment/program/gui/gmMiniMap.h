#pragma once
#include "gmUIObjectBase.h"
#include <memory>

namespace gm {

    class gmMapManager;
    class gmPlayerShip;
    class gmIcebergManager;
    class gmTradeShipManager;

    class gmMiniMap : public gmUIObjectBase {
    public:
        gmMiniMap(
            const tnl::Vector2f& pos,
            std::shared_ptr<gmMapManager> map,
            std::shared_ptr<gmPlayerShip> player,
            std::shared_ptr<gmIcebergManager> icebergManager,
            std::shared_ptr<gmTradeShipManager> tradeShipManager
        );

        void update(float dt) override;
        void draw() override;

    private:
        void drawBackground();
        void drawIslands();
        void drawIcebergs();
        void drawTradeRoutes();
        void drawTradeShips();
        void drawPlayer();

        // 船を表す三角形マーカーを1つ描画する(プレイヤー船・交易船で共通)。
        // arg1... ワールド座標(3D)
        // arg2... 進行方向(ワールド空間、3D。正規化前でよい)
        // arg3... 塗りつぶし色
        void drawShipTriangleMarker(const tnl::Vector3& worldPos, const tnl::Vector3& worldForward, unsigned int color);

    private:
        std::shared_ptr<gmMapManager> map_;
        std::shared_ptr<gmPlayerShip> player_;
        std::shared_ptr<gmIcebergManager> icebergManager_;
        std::shared_ptr<gmTradeShipManager> tradeShipManager_;

        // DXLib のグラフィックハンドル
        int hBackground_ = -1;
        int hIsland_ = -1;

        static constexpr int MAP_SIZE = 256; // ミニマップの描画サイズ
    };
}
