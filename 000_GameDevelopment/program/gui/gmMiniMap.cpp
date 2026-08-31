#include "gmMiniMap.h"
#include "../map/gmMapManager.h"
#include "../object/gmPlayerShip.h"
#include "../object/gmTradeShip.h"
#include "../object/gmIceberg.h"
#include "../spawner/gmIcebergManager.h"
#include "../spawner/gmTradeShipManager.h"
#include "../util/gmRouteCenterlineUtil.h"
#include "../gmGameConfig.h"
#include <DxLib.h>

namespace gm {

    static constexpr int MINI_SIZE = 256;
    static constexpr float MINIMAP_CELL_SCALE = 1.0f;

    // 交易船航路の中心線サンプリング間隔(world単位)。3Dのリボン表示(gmRouteVisualizer)と
    // 同じROUTE_RIBBON_SAMPLE_STEPを流用し、見た目の航路形状を一致させる。
    static constexpr float MINIMAP_ROUTE_SAMPLE_STEP = ROUTE_RIBBON_SAMPLE_STEP;

    // 交易船マーカーの色(明度高めで、目がチカチカしない程度の黄色味)
    static constexpr unsigned char TRADE_SHIP_COLOR_R = 250;
    static constexpr unsigned char TRADE_SHIP_COLOR_G = 205;
    static constexpr unsigned char TRADE_SHIP_COLOR_B = 90;

    // 航路ラインの色(かなり白っぽいゴールド系)
    static constexpr unsigned char ROUTE_LINE_COLOR_R = 235;
    static constexpr unsigned char ROUTE_LINE_COLOR_G = 225;
    static constexpr unsigned char ROUTE_LINE_COLOR_B = 190;

    // プレイヤー船マーカーの色(明度高めで、目がチカチカしない程度の赤)
    static constexpr unsigned char PLAYER_SHIP_COLOR_R = 230;
    static constexpr unsigned char PLAYER_SHIP_COLOR_G = 90;
    static constexpr unsigned char PLAYER_SHIP_COLOR_B = 80;

    // 流氷マーカーの半径(ティアごと。大/中/小の実際の相対サイズ関係と一致させる)
    static constexpr int ICEBERG_MARKER_RADIUS_SMALL = 1;
    static constexpr int ICEBERG_MARKER_RADIUS_MEDIUM = 2;
    static constexpr int ICEBERG_MARKER_RADIUS_LARGE = 3;

    // 流氷マーカーの色(ティアごと)。背景(海)が彩度高めの濃い青のため、
    // 小さいほど見落としやすい=最大コントラストの白系、大きいほど(サイズ自体で目立つので)
    // 氷らしい寒色系の中で彩度を上げていく、という方針。
    static constexpr unsigned char ICEBERG_COLOR_SMALL_R = 248; // ほぼ純白
    static constexpr unsigned char ICEBERG_COLOR_SMALL_G = 251;
    static constexpr unsigned char ICEBERG_COLOR_SMALL_B = 255;

    static constexpr unsigned char ICEBERG_COLOR_MEDIUM_R = 200; // 薄い氷色(従来の色をそのまま維持)
    static constexpr unsigned char ICEBERG_COLOR_MEDIUM_G = 235;
    static constexpr unsigned char ICEBERG_COLOR_MEDIUM_B = 255;

    static constexpr unsigned char ICEBERG_COLOR_LARGE_R = 110; // 彩度を上げたターコイズ寄りの氷色
    static constexpr unsigned char ICEBERG_COLOR_LARGE_G = 225;
    static constexpr unsigned char ICEBERG_COLOR_LARGE_B = 215;

    // 大サイズだけ、白縁取りを通常より太くして特別感を出す(通常は+1px)
    static constexpr int ICEBERG_LARGE_BORDER_WIDTH = 2;


    gmMiniMap::gmMiniMap(
        const tnl::Vector2f& pos,
        std::shared_ptr<gmMapManager> map,
        std::shared_ptr<gmPlayerShip> player,
        std::shared_ptr<gmIcebergManager> icebergManager,
        std::shared_ptr<gmTradeShipManager> tradeShipManager
    )
        : gmUIObjectBase(pos)
        , map_(std::move(map))
        , player_(std::move(player))
        , icebergManager_(std::move(icebergManager))
        , tradeShipManager_(std::move(tradeShipManager))
    {
        // DXLib の LoadGraph で読み込み
        hBackground_ = LoadGraph("resource/graphics/minimap/mini_bkground.png");
        hIsland_ = LoadGraph("resource/graphics/island/lawn.png");
    }

    void gmMiniMap::update(float dt)
    {
        // 必要なら点滅やアニメーション
    }

    void gmMiniMap::draw()
    {
        drawBackground();
        drawIslands();
        drawTradeRoutes();
        drawTradeShips();
        drawIcebergs();
        drawPlayer();
    }

    void gmMiniMap::drawBackground()
    {
        DrawExtendGraph(
            static_cast<int>(position_.x),
            static_cast<int>(position_.y),
            static_cast<int>(position_.x + MAP_SIZE),
            static_cast<int>(position_.y + MAP_SIZE),
            hBackground_,
            TRUE
        );
    }

    void gmMiniMap::drawIslands()
    {
        const auto& islands = map_->GetIslands();

        for (const auto& isl : islands)
        {
            // 1. セル座標（0〜255）をベースにミニマップ上の描画位置を決定
            // IslandInfo の実際のメンバ名「cellMin」「cellMax」を使用します
            float ix = position_.x + static_cast<float>(isl.cellMin.x);
            float iy = position_.y + static_cast<float>(isl.cellMin.y);

            // 2. セル単位での幅と高さを計算（1セル = 1ピクセル換算）
            float iw = static_cast<float>(isl.cellMax.x - isl.cellMin.x + 1);
            float ih = static_cast<float>(isl.cellMax.y - isl.cellMin.y + 1);

            DrawExtendGraph(
                static_cast<int>(ix),
                static_cast<int>(iy),
                static_cast<int>(ix + iw),
                static_cast<int>(iy + ih),
                hIsland_,
                TRUE
            );
        }
    }

    void gmMiniMap::drawIcebergs()
    {
        if (!icebergManager_) return;

        // プレイヤーマーカーと同じ変換規則
        // (ワールドX→そのまま、ワールドZ→符号反転して北を上に)
        const float worldToMiniScale = 1.0f / CELL_SIZE;

        for (const auto& iceberg : icebergManager_->getEntities())
        {
            if (!iceberg) continue;

            // サイズ(大/中/小)をミニマップ上でも見分けられるよう、ティアごとに半径・色・縁の
            // 太さを変える(実際の相対サイズ関係と一致させる: 大 > 中 > 小)。
            //
            // 背景(海)が彩度高めの濃い青(実測 R14/G118/B167程度)のため、単純な水色だと
            // 埋もれてしまう懸念があった。方針: 小さいほど見落としやすいので最大コントラストの
            // 白系にし、大きいほど(サイズ自体で目立つので)氷らしい寒色系の中で彩度を上げていく。
            // 大サイズだけ縁取りも太くして、さらに特別感を出す。
            int coreRadius = ICEBERG_MARKER_RADIUS_MEDIUM;
            unsigned int fillColor = GetColor(ICEBERG_COLOR_MEDIUM_R, ICEBERG_COLOR_MEDIUM_G, ICEBERG_COLOR_MEDIUM_B);
            int borderWidth = 1;
            switch (iceberg->getTier()) {
            case gmIceberg::Tier::Small:
                coreRadius = ICEBERG_MARKER_RADIUS_SMALL;
                fillColor = GetColor(ICEBERG_COLOR_SMALL_R, ICEBERG_COLOR_SMALL_G, ICEBERG_COLOR_SMALL_B);
                borderWidth = 1;
                break;
            case gmIceberg::Tier::Medium:
                coreRadius = ICEBERG_MARKER_RADIUS_MEDIUM;
                fillColor = GetColor(ICEBERG_COLOR_MEDIUM_R, ICEBERG_COLOR_MEDIUM_G, ICEBERG_COLOR_MEDIUM_B);
                borderWidth = 1;
                break;
            case gmIceberg::Tier::Large:
                coreRadius = ICEBERG_MARKER_RADIUS_LARGE;
                fillColor = GetColor(ICEBERG_COLOR_LARGE_R, ICEBERG_COLOR_LARGE_G, ICEBERG_COLOR_LARGE_B);
                borderWidth = ICEBERG_LARGE_BORDER_WIDTH; // 大サイズだけ縁取りを太くする
                break;
            }

            tnl::Vector3 pos3D = iceberg->getPosition();

            int px = static_cast<int>(position_.x + pos3D.x * worldToMiniScale);
            int py = static_cast<int>(position_.y - pos3D.z * worldToMiniScale);

            // 白の縁取り + 本体色を重ねて描画
            DrawCircle(px, py, coreRadius + borderWidth, GetColor(255, 255, 255), TRUE);
            DrawCircle(px, py, coreRadius, fillColor, TRUE);
        }
    }

    // ------------------------------------------------------------
    // 交易船の航路をベジェ曲線相当の滑らかな中心線(gm::SampleRouteCenterline()。
    // 3Dのリボン表示・交易船の実際の航行と同じロジック)で描画する。
    // ------------------------------------------------------------
    void gmMiniMap::drawTradeRoutes()
    {
        if (!map_) return;

        const float worldToMiniScale = 1.0f / CELL_SIZE;
        const unsigned int lineColor = GetColor(ROUTE_LINE_COLOR_R, ROUTE_LINE_COLOR_G, ROUTE_LINE_COLOR_B);

        const size_t routeCount = map_->GetRouteCount();
        for (size_t i = 0; i < routeCount; ++i)
        {
            std::vector<tnl::Vector2f> waypointsWorld = map_->GetRouteWorldPoints(i);
            if (waypointsWorld.size() < 2) continue;

            std::vector<tnl::Vector2f> centerline = SampleRouteCenterline(waypointsWorld, MINIMAP_ROUTE_SAMPLE_STEP);
            if (centerline.size() < 2) continue;

            for (size_t p = 0; p + 1 < centerline.size(); ++p)
            {
                // Note: gmMapManagerが返すワールド座標は(X, Z)を(x, y)として詰めたもの。
                // 他の描画箇所と同じ変換規則(ワールドZ→符号反転して北を上に)に合わせる。
                const int x1 = static_cast<int>(position_.x + centerline[p].x * worldToMiniScale);
                const int y1 = static_cast<int>(position_.y - centerline[p].y * worldToMiniScale);
                const int x2 = static_cast<int>(position_.x + centerline[p + 1].x * worldToMiniScale);
                const int y2 = static_cast<int>(position_.y - centerline[p + 1].y * worldToMiniScale);

                DrawLine(x1, y1, x2, y2, lineColor);
            }
        }
    }

    void gmMiniMap::drawTradeShips()
    {
        if (!tradeShipManager_) return;

        const unsigned int color = GetColor(TRADE_SHIP_COLOR_R, TRADE_SHIP_COLOR_G, TRADE_SHIP_COLOR_B);

        for (const auto& ship : tradeShipManager_->getEntities())
        {
            if (!ship) continue;
            drawShipTriangleMarker(ship->getPosition(), ship->getForward(), color);
        }
    }


    void gmMiniMap::drawPlayer()
    {
        if (!player_) return;

        const unsigned int color = GetColor(PLAYER_SHIP_COLOR_R, PLAYER_SHIP_COLOR_G, PLAYER_SHIP_COLOR_B);
        drawShipTriangleMarker(player_->getPosition(), player_->getForward(), color);
    }

    // ------------------------------------------------------------
    // 船を表す三角形マーカーを1つ描画する(プレイヤー船・交易船で共通)。
    // 以前はプレイヤー船のみ、この三角形に加えてアイコン画像も重ねて描画していたが、
    // 見た目がごちゃついていたため、三角形のみのシンプルな表現に統一した。
    // ------------------------------------------------------------
    void gmMiniMap::drawShipTriangleMarker(const tnl::Vector3& worldPos, const tnl::Vector3& worldForward, unsigned int color)
    {
        // --- 1.位置を算出 ---
        // 縮尺：ワールド座標をセル（ピクセル）単位に変換
        const float worldToMiniScale = 1.0f / CELL_SIZE;

        // ワールド座標(X, Z) を ミニマップ上の相対ピクセル(X, Y) に変換
        // Note:
        // 2D座標でY座標が小さいほど北、3D座標でZ座標が大きいほど北
        const float px = position_.x + (worldPos.x * worldToMiniScale);
        const float py = position_.y - (worldPos.z * worldToMiniScale);

        // --- 2.向きを表す三角形の頂点を計算 ---
        // 3Dの yaw 回転　と 2Dミニマップ空間の軸を完全に一致させます。
        // Note:
        // ワールド X -> ミニマップ X（そのまま）
        // ワールド Z -> ミニマップ Y（北が上なので反転）
        float fx = worldForward.x;
        float fy = -worldForward.z;

        // ベクトルの正規化
        const float len = std::sqrt(fx * fx + fy * fy);
        if (len > 0.0f) {
            fx /= len;
            fy /= len;
        }

        // 船の右方向ベクトル（90度時計回り）
        const float rx = fy;
        const float ry = -fx;

        // 三角形のサイズ
        const float length = 8.0f;
        const float widthHalf = 4.0f;
        const float backOffset = 3.0f;

        // 各頂点の座標を計算
        const int x1 = static_cast<int>(px + fx * length);
        const int y1 = static_cast<int>(py + fy * length);

        const int x2 = static_cast<int>(px - fx * backOffset + rx * widthHalf);
        const int y2 = static_cast<int>(py - fy * backOffset + ry * widthHalf);

        const int x3 = static_cast<int>(px - fx * backOffset - rx * widthHalf);
        const int y3 = static_cast<int>(py - fy * backOffset - ry * widthHalf);

        DrawTriangle(x1, y1, x2, y2, x3, y3, color, TRUE);
    }

}
