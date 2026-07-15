#pragma once
#include <memory>
#include <dxe.h>

#include "gmGameConfig.h"
//#include "../input/gmInputManager.h"
//#include "../sound/gmSoundManager.h"
#include "./map/gmMapManager.h"
#include "./scene/gmFadeTransitionEffect.h"

namespace gm
{
    // ゲーム全体で共有する依存をまとめたコンテナ
    struct gmGameContext
    {
        // 入力管理
        //std::shared_ptr<gmInputManager> input;

        // サウンド管理
        //std::shared_ptr<gmSoundManager> sound;

        // マップ管理（map.bin / ocean_flow.bin）
        std::shared_ptr<gmMapManager> map;

        // カメラ（dxe::Camera）
        std::shared_ptr<dxe::Camera> camera;

        // フェードイン/アウト
        std::shared_ptr<gmFadeTransitionEffect> fade;

        // ゲーム設定（CELL_SIZE など）
        // これは static const の集まりなのでインスタンス不要
        // gmGameConfig は include するだけで OK
    };
}
