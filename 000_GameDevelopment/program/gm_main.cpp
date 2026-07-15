#include <time.h>
#include <dxe.h>
#include "gm_main.h"

#include "./scene/gmSceneManager.h"
//#include "./scene/scenes/gmTitleScene.h"
#include "./scene/scenes/gmGameScene.h"

std::shared_ptr<gm::gmSceneManager> g_sceneManager;

//------------------------------------------------------------
// ゲーム開始時に一度だけ呼ばれる
//------------------------------------------------------------
void gameStart()
{
    srand(time(0));

    // SceneManager 生成（内部で GameContext も生成される）
    g_sceneManager = std::make_shared<gm::gmSceneManager>();

    // 最初のシーンはタイトル
    //auto titleScene = std::make_shared<gm::gmTitleScene>();
    auto gameScene = std::make_shared<gm::gmGameScene>();
    g_sceneManager->setInitialScene(gameScene);
}

//------------------------------------------------------------
// 毎フレーム呼ばれる
//------------------------------------------------------------
void gameMain(float deltaTime)
{
    g_sceneManager->update();
    g_sceneManager->draw();

    dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });
}

//------------------------------------------------------------
// ゲーム終了時に一度だけ呼ばれる
//------------------------------------------------------------
void gameEnd()
{
    // 特に破棄処理は不要
}
