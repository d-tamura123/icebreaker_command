// gmSoundFileConfig.h
// サウンド(SE/BGM)のファイルパス・キャッシュ用の識別名を集約する定数ファイル。
// gmGameConfig.hと同じ書式(namespace gm直下にstatic const char* const)。
#pragma once

namespace gm {

    // ------------------------------------------------------------
    // BGM
    // ------------------------------------------------------------
    // タイトル画面BGM
    static const char* const BGM_NAME__TITLE      = "title_bgm"; // gmSoundManagerのキャッシュキー(playBgm()等で使う識別名)
    static const char* const SOUND_FILE_PATH__TITLE_BGM = "resource/sound/title/destruction-and-regeneration.mp3";
    
    // インゲーム(ゲームプレイ中)BGM
    static const char* const BGM_NAME__INGAME = "ingame_bgm";
    static const char* const SOUND_FILE_PATH__INGAME_BGM = "resource/sound/bgm/tayutau-sazanami-no-kage-ni.mp3";
}
