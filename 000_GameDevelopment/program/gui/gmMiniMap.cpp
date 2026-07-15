#include "gmMiniMap.h"
#include "../map/gmMapManager.h"
#include "../object/gmPlayerShip.h"
#include <DxLib.h>

namespace gm {

    static constexpr int MINI_SIZE = 256;
    static constexpr float MINIMAP_CELL_SCALE = 1.0f;

    gmMiniMap::gmMiniMap(
        const tnl::Vector2f& pos,
        std::shared_ptr<gmMapManager> map,
        std::shared_ptr<gmPlayerShip> player)
        : gmUIObjectBase(pos)
        , map_(std::move(map))
        , player_(std::move(player))
    {
        // DXLib の LoadGraph で読み込み
        hBackground_ = LoadGraph("resource/graphics/test/minimap/mini_bkground.png");
        hIsland_ = LoadGraph("resource/graphics/test/lawn.png");
        hPlayer_ = LoadGraph("resource/graphics/test/minimap/player_mark.png");
    }

    void gmMiniMap::update(float dt)
    {
        // 必要なら点滅やアニメーション
    }

    void gmMiniMap::draw()
    {
        drawBackground();
        drawIslands();
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

    void gmMiniMap::drawPlayer()
    {
        // --- 1.位置やスケールを算出 --- 
        tnl::Vector3 pos3D = player_->getPosition();

        // 縮尺：ワールド座標をセル（ピクセル）単位に変換 (1.0f / 100.0f)
        float worldToMiniScale = 1.0f / CELL_SIZE; // ゲームのCELL_SIZEに合わせて調整

        // ワールド座標(X, Z) を ミニマップ上の相対ピクセル(X, Y) に変換
        // Note:
        // 2D座標でY座標が小さいほど北、3D座標でZ座標が大きいほど北
        float px = position_.x + (pos3D.x * worldToMiniScale);
        float py = position_.y - (pos3D.z * worldToMiniScale);

        // --- 2.プレイヤーの向きを表すマーカーの描画 ---
        // 船の前方向ベクトルを取得（ワールド空間でのX, Z向き）
        tnl::Vector3 forward3D = player_->getForward();

        // 3Dの yaw 回転　と 2Dミニマップ空間の軸を完全に一致させます。
        // Note:
        // ワールド X -> ミニマップ X（そのまま）
        // ワールド Z -> ミニマップ Y（北が上なので反転）
        float fx = forward3D.x;
        float fy = -forward3D.z;

        // ベクトルの正規化
        float len = std::sqrt(fx * fx + fy * fy);
        if (len > 0.0f) {
            fx /= len;
            fy /= len;
        }

        // --- あとはこの fx, fy を使って三角形の頂点を計算 ---
        // 船の右方向ベクトル（90度時計回り）
        float rx = fy;
        float ry = -fx;

        // 三角形のサイズ
        float length = 18.0f;
        float widthHalf = 9.0f;

        // 各頂点の座標を計算
        int x1 = static_cast<int>(px + fx * length);
        int y1 = static_cast<int>(py + fy * length);

        int x2 = static_cast<int>(px - fx * 6.0f + rx * widthHalf);
        int y2 = static_cast<int>(py - fy * 6.0f + ry * widthHalf);

        int x3 = static_cast<int>(px - fx * 6.0f - rx * widthHalf);
        int y3 = static_cast<int>(py - fy * 6.0f - ry * widthHalf);

        // 三角形を描画
        DrawTriangle(x1, y1, x2, y2, x3, y3, GetColor(24, 24, 24), TRUE);



        // --- 3.プレイヤーアイコンの描画  ---
        // アイコンのサイズ（指定の 16x16 ピクセル）
        float iconSize = 18.0f;
        float halfSize = iconSize / 2.0f; // 8.0f

        // --- 修正後 ---
        // 中心座標 (px, py) から上下左右に 8 ピクセルずつ広げた範囲（16x16）に引き伸ばして描画
        DrawExtendGraph(
            static_cast<int>(px - halfSize),
            static_cast<int>(py - halfSize),
            static_cast<int>(px + halfSize),
            static_cast<int>(py + halfSize),
            hPlayer_,
            TRUE
        );

    }

}
