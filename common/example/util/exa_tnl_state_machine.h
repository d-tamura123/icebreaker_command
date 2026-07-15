#if 0

//---------------------------------------------------------------------------------------------------------------
//
// ステートマシンクラス ( tnl::StateMachine ) 使用法サンプル
//
// 【ステートマシンとは】
//   「現在の状態（State）」に応じて、毎フレーム実行される処理を切り替える仕組み。
//   例えば「タイトル画面 → ゲーム中 → リザルト画面」という流れを、
//   条件に応じてスムーズに遷移させられる。
//   実装上は、現在の状態に対応する関数（update処理など）を登録しておき、
//   change() を呼び出すことで次の状態へ移行する。
//
// 【このサンプルの用途】
//   - ゲームの「シーン切り替え」を例にしている。
//   - タイトル画面とゲーム画面で別々の update 関数を持ち、
//     Enterキーを押すとタイトルからゲーム画面に遷移する。
//   - 別の用途として、同じクラス内で処理を分けたり、
//     UI状態やキャラクターAIの行動分岐などにも使える。
//
// 【使用フロー】
//   1. StateMachine を生成し、最初に実行する関数を登録する。
//   2. 毎フレーム update() を呼び出すことで、登録された関数が実行される。
//   3. change() を呼ぶと、次のフレームから別の関数に切り替わる。
//   4. immediatelyChange() を使えば、その場で即座に切り替わる。
//
//---------------------------------------------------------------------------------------------------------------

#include <time.h>
#include <string>
#include <dxe.h>
#include "../ResourceConstantHedder.h"
#include "gm_main.h"

//----------------------------------------------
// タイトル画面クラス
//----------------------------------------------
class SceneTitle {
public:
    void update();
};
void SceneTitle::update() {
    // 画面中央に「Scene Title」と描画
    DrawFormatString(DXE_WINDOW_WIDTH_HALF, DXE_WINDOW_HEIGHT_HALF, -1, "Scene Title");
}

//----------------------------------------------
// ゲーム画面クラス
//----------------------------------------------
class SceneInGame {
public:
    void update();
};
void SceneInGame::update() {
    // 画面中央に「Scene In Game」と描画
    DrawFormatString(DXE_WINDOW_WIDTH_HALF, DXE_WINDOW_HEIGHT_HALF, -1, "Scene In Game");
}

//----------------------------------------------
// ゲーム全体を管理するクラス
//----------------------------------------------
class GameManager {
public:
    GameManager() {
        // タイトル画面を生成
        scene_title_ = new SceneTitle();

        // StateMachineを生成し、最初はタイトル画面の update を登録
        scene_flow_ = TNL_STATE_MACHINE_CREATE(&SceneTitle::update, scene_title_);
    }

    SceneTitle* scene_title_ = nullptr;
    SceneInGame* scene_in_game_ = nullptr;

    // シーン遷移を制御するステートマシン
    Shared<tnl::StateMachine> scene_flow_;

    void update();
};

void GameManager::update() {

    // Enterキーが押されたらシーンをゲーム画面に切り替える
    if (tnl::Input::IsKeyDownTrigger(eKeys::KB_RETURN)) {
        // ゲーム画面がまだ作られていない場合のみ生成
        if (!scene_in_game_) {
            scene_in_game_ = new SceneInGame();

            // タイトル画面は不要になるので削除
            TNL_SAFE_DELETE(scene_title_);

            // 次のフレームからゲーム画面の update を実行するように変更
            scene_flow_->change(TNL_STATE_BIND(&SceneInGame::update, scene_in_game_));
        }
    }

    // 現在のシーン処理を実行
    scene_flow_->update(dxe::GetDeltaTime());
}

// ゲーム管理クラスのグローバルインスタンス
GameManager* game_manager = nullptr;

//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に1回だけ実行される初期化処理
void gameStart() {
    /* DxLib_Init() はシステム側で行われているので不要 */
    srand(time(0));

    // GameManager生成（初期シーンはタイトル）
    game_manager = new GameManager();
}

//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されるメインループ
void gameMain(float delta_time) {
    // 現在のシーンを更新
    game_manager->update();

    // 画面左上に操作説明を描画
    DrawFormatString(0, 0, -1, "Enter : シーンの切り替えを実行");

    // FPS表示
    dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });
}

//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に1回だけ実行される終了処理
void gameEnd() {
    /* DxLib_End() はシステム側で終了処理が行われるので不要 */
}

#endif