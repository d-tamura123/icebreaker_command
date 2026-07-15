#pragma once

namespace gm
{
    // マップサイズ（Excel側と完全一致）
    static const int MAP_CHIP_WIDTH = 256;
    static const int MAP_CHIP_HEIGHT = 256;

    // ゲーム世界座標系での1セル(チップ)の大きさ
    static const float CELL_SIZE = 100.0f;

    // マップデータのパス
    static const char* const MAP_FILE_PATH = "resource/map/map.bin";
    
    // 海流マップデータのパス
    static const char* const FLOW_STO_E_PATH = "resource/map/ocean_flow_StoE.bin";
    static const char* const FLOW_STO_N_PATH = "resource/map/ocean_flow_StoN.bin";
    static const char* const FLOW_STO_W_PATH = "resource/map/ocean_flow_StoW.bin";

    // 描画範囲の定義
    static const float RENDER_DISTANCE = 6400.0f;   // カメラからの描画距離
    static const float RENDER_DISTANCE_SQ = RENDER_DISTANCE * RENDER_DISTANCE;

    // 画像ファイルパス
    static const char* const GRAPHICS_FILE_PATH__OCEAN_FLOW_ARROW = "resource/graphics/test/wf_arrow.png";


}
