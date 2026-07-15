#if 0
//---------------------------------------------------------------------------------------------------------------
//
//
// tnl::Coroutin 利用法サンプル
// 
//
//---------------------------------------------------------------------------------------------------------------
#include <time.h>
#include <string>
#include <dxe.h>
#include "../ResourceConstantHedder.h"
#include "gm_main.h"


class Foo {
public:

	// TNL_COROUTINE は 指定された関数単位で状態を管理するための構造を内部に作成します。
	TNL_COROUTINE(
		funcA,
		funcB
	);

	void funcA();
	void funcB();

};


void Foo::funcA() {
	// コルーチンの開始。
	// [ !注意! ] このマクロ引数には必ず動作中の関数名を指定してください
	TNL_CO_BEGIN(funcA);

	// 赤と青の円を交互に表示し、それを延々と繰り返す ( while(1) )
	while (1) {
		// 1秒間、赤い円を描画しつつ経過を待ち、完了したら次へ進む
		TNL_CO_TIME_YIELD_RETURN(1.0f, dxe::GetDeltaTime(), [&]() {
			DrawCircle(400, 300, 200, dxe::Colors::Red);
			});
		// 1秒間、青い円を描画しつつ経過を待ち、完了したら次へ進む
		TNL_CO_TIME_YIELD_RETURN(1.0f, dxe::GetDeltaTime(), [&]() {
			DrawCircle(400, 300, 200, dxe::Colors::Blue);
			});
	}

	// コルーチンの終了
	TNL_CO_END();
}


void Foo::funcB() {

	// コルーチンの開始。
	// [ !注意! ] このマクロ引数には必ず動作中の関数名を指定してください
	TNL_CO_BEGIN(funcB);

	// 最初に3秒間、黄色い円を表示
	TNL_CO_TIME_YIELD_RETURN(3.0f, dxe::GetDeltaTime(), [&]() {
		DrawCircle(800, 300, 100, dxe::Colors::Yellow);
		});

	// 続いて3秒間、緑の円を表示。ただし時間経過で中断も可能
	TNL_CO_TIME_YIELD_RETURN(3.0f, dxe::GetDeltaTime(), [&]() {

		// このyield処理の最初のフレームだけ "init" を描画
		if (TNL_CO_YIELD_IS_START()) {
			DrawFormatString(770, 400, -1, "init");
		}

		// 毎フレーム 緑の円を描画
		DrawCircle(800, 300, 100, dxe::Colors::Green);

		// 経過時間が 1.0 秒を超えたら早期に中断する
		if (TNL_CO_PROG_TIME() > 1.0f) TNL_CO_BREAK();
		});

	// コルーチンの終了
	TNL_CO_END();
}


// コルーチン機能を付与したインスタンス
Foo f;

//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {
	srand(time(0));
}


//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {

	// 毎フレーム、funcAとfuncBのコルーチン関数を実行（継続・再開）
	f.funcA();
	f.funcB();

	// Enterキーが押されたら、funcB のコルーチンをリセット
	if (tnl::Input::IsKeyDownTrigger(eKeys::KB_RETURN)) {
		TNL_CO_RESET_FROM_INSTANCE(f, funcB);
	}

	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });
}


//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {
}

#endif