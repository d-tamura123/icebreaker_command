#include <time.h>
#include <DxLib.h>
#include <dxe.h>
#include "gm_main.h"

#include "./scene/gmSceneManager.h"
#include "./scene/scenes/gmTitleScene.h"
#include "./scene/scenes/gmGameScene.h"

std::shared_ptr<gm::gmSceneManager> g_sceneManager;

//------------------------------------------------------------
// ゲーム開始時に一度だけ呼ばれる
//------------------------------------------------------------
void gameStart()
{
    // time_t(64bit環境では long long 相当) から unsigned int への暗黙変換で出ていたため、
    // 明示的にキャストする(乱数シードとしての用途上、下位ビットだけ使われても問題は無い)。
    srand(static_cast<unsigned int>(time(0)));

    // Alt(またはF10)キー押下時、Windows既定のシステムメニュー起動処理(WM_SYSKEYDOWN)を
    // DxLibが素通しすることで、キーを押している間アプリのメッセージループが一時停止してしまう
    // 既知の挙動への対策。
    SetSysCommandOffFlag(TRUE);

    // SceneManager 生成（内部で GameContext も生成される）
    g_sceneManager = std::make_shared<gm::gmSceneManager>();

    // 最初のシーンはタイトル
    auto titleScene = std::make_shared<gm::gmTitleScene>();
    //auto gameScene = std::make_shared<gm::gmGameScene>();
    g_sceneManager->setInitialScene(titleScene);
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
