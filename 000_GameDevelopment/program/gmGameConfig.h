// gmGameConfig.h
#pragma once
#include <cstdint>      // uint8_t


namespace gm
{
    // マップサイズ（Excel側と完全一致）
    static const int MAP_CHIP_WIDTH     = 256;
    static const int MAP_CHIP_HEIGHT    = 256;

    // ゲーム世界座標系での1セル(チップ)の大きさ
    static const float CELL_SIZE = 100.0f;

    // マップデータのパス
    static const char* const MAP_FILE_PATH = "resource/map/map.bin";

    // 海流マップデータのパス
    static const char* const FLOW_STO_E_PATH = "resource/map/ocean_flow_StoE.bin";
    static const char* const FLOW_STO_N_PATH = "resource/map/ocean_flow_StoN.bin";
    static const char* const FLOW_STO_W_PATH = "resource/map/ocean_flow_StoW.bin";

    // 航路データのパス(プレフィックス)
    // "route_1.bin", "route_2.bin", ... のように1始まりの連番を後ろに付けて読み込む。
    // 何本の航路が存在するかを別途持たず、
    // このプレフィックス+連番で読み込みに失敗するまで走査する方式
    // (gmMapManager::LoadRoutes()参照)。
    static const char* const ROUTE_FILE_PATH_PREFIX = "resource/map/route_";

    // 描画範囲の定義
    static const float RENDER_DISTANCE      = 6400.0f;                              // カメラからの描画距離
    static const float RENDER_DISTANCE_SQ   = RENDER_DISTANCE * RENDER_DISTANCE;    // 距離比較を高速化するための2乗値
                                                                                    // ※ 距離の比較は本来sqrt()が必要だが、
                                                                                    //    両辺を2乗しても大小関係は変わらないため、
                                                                                    //    平方根の計算を省略できる

    // 画像ファイルパス
    static const char* const GRAPHICS_FILE_PATH__OCEAN_FLOW_ARROW       = "resource/graphics/test/wf_arrow.png";

    // VFX(エフェクト)関連パス
    static const char* const VFX_SPRITE_METADATA_CSV_PATH               = "resource/csv/tktk_sprite_metadata.csv";
    static const char* const VFX_EFFECT_GRAPHICS_DIR                    = "resource/graphics/effect/";

    // 武器関連パス
    static const char* const GRAPHICS_FILE_PATH__SPLIT_PROJECTILE_BALL  = "resource/graphics/weapon/split_projectile_ball.png"; // 割り砲弾(鉄球)用テクスチャ。黒単色を想定



    // ------------------------------------------------------------
    // 航路の可視化(NPC交易船が辿る航路を、ゲーム内にリボンメッシュとして描画する)
    // ------------------------------------------------------------
    // 素材画像(横長のグラデーション。縦方向(V)が中央最明→上下端でアルファ0、
    // 横方向(U)は無地でよい。gmRouteVisualizer参照)
    static const char* const GRAPHICS_FILE_PATH__ROUTE_RIBBON = "resource/graphics/route/route_ribbon_gradient.png";

    // 空中リボン(水平・垂直の十字クロス)を、海面(y=0)からどれだけ高い位置に固定するか。
    // 波の高さは追わず常に一定(モノレールのレールのように)。
    static const float ROUTE_RIBBON_ALTITUDE = 25.0f;

    // 空中リボン(水平・垂直とも共通)の帯の幅(world単位)
    static const float ROUTE_RIBBON_WIDTH = 60.0f;

    // 中心線(遠心的Catmull-Romで補間した曲線)をポリライン化する際のサンプリング間隔(world単位)。
    // 小さいほど滑らかになるが、頂点数・ドローコール準備コストが増える。
    static const float ROUTE_RIBBON_SAMPLE_STEP = 25.0f;

    // 描画カリング用にチャンク分割する際の、1チャンクあたりの目安の長さ(world単位)。
    // gmIceberg等と同じ距離カリング(RENDER_DISTANCE_SQ)をチャンク単位で適用する。
    static const float ROUTE_RIBBON_CHUNK_LENGTH = 700.0f;

    // 始点(S)・終点(G)付近でアルファを0まで滑らかにフェードさせる区間の長さ(world単位)
    static const float ROUTE_RIBBON_FADE_LENGTH = 150.0f;

    // UVスクロールでテクスチャが1周する距離(world単位)。値が小さいほど帯の模様が細かく繰り返す。
    static const float ROUTE_RIBBON_UV_REPEAT_LENGTH = 400.0f;

    // UVスクロール速度(1秒あたりに進むUV量。ROUTE_RIBBON_UV_REPEAT_LENGTHに対する割合)。
    // 各航路はS→Gの一方向のみなので、この方向へ流れる=「今どちら向きの航路か」を示す意味を持つ。
    // 0を指定するとスクロールなし(静止した帯)になる。
    static const float ROUTE_RIBBON_SCROLL_SPEED = 0.35f;

    // 海面の影用リボン(位置確認用、水平のみ・空中リボンより控えめな見た目)関連
    static const float   ROUTE_SHADOW_RIBBON_HEIGHT_OFFSET  = 2.0f;   // 水面メッシュとのZファイティング回避用の微小オフセット(海面y=0からの高さ)
    static const float   ROUTE_SHADOW_RIBBON_WIDTH_SCALE    = 0.4f;   // 空中リボン幅に対する、影リボンの幅の比率
    static const uint8_t ROUTE_SHADOW_RIBBON_ALPHA_SCALE    = 90;     // 影リボンの最大アルファ(0〜255。空中リボンより控えめにする)
    static const uint8_t ROUTE_SHADOW_RIBBON_COLOR_SCALE    = 140;    // 影リボンの頂点色に掛ける明度スケール(0〜255。暗めにする)

}
