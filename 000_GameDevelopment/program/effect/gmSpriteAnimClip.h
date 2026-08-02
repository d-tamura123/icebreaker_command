// gmSpriteAnimClip.h
#pragma once
#include <string>

namespace gm {

    // ------------------------------------------------------------
    // スプライトシートアニメーション1件分のメタデータ。
    // tktk_sprite_metadata.csv の1行に対応する。
    // ------------------------------------------------------------
    struct gmSpriteAnimClip {
        std::string name;           // CSVのfile列と一致する名前(例: "tktk_Fire_10")
        int   cellSize = 192;       // 1コマの一辺のピクセル数
        int   cols = 1;             // 横のコマ数
        int   rows = 1;             // 縦のコマ数
        int   frameCount = 1;       // 実際に使うコマ数(末尾の空白コマを除いた数)
        float fps = 15.0f;          // 再生速度

        // ---- 持続系エフェクト(火炎放射等)用。個別に手動設定する ----
        // どちらも-1のままなら「ループしないワンショット再生」として扱う。
        int loopStart = -1;         // ループ区間の開始コマ(このコマからループが始まる)
        int loopEnd = -1;           // ループ区間の終了コマ(この次のコマからアウトロ)
    };
}
