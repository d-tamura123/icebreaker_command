// gmGameConfig.h
#pragma once

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
}
