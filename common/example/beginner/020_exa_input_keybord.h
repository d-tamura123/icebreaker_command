#if 0

//---------------------------------------------------------------------------------------------------------------
//
//
// [ 入門 ] キーボード入力を検知する基本的サンプル
// 
//
//---------------------------------------------------------------------------------------------------------------


#include <time.h>
#include <string>
#include <dxe.h>
#include "gm_main.h"

// キーを押しっぱなしでカウントアップし続ける変数
int on_keep_count_up_value = 0;

// キーが押された瞬間だけカウントアップする変数
int on_trigger_count_up_value = 0;

// キーが離された瞬間だけカウントアップする変数
int on_trigger_release_count_up_value = 0;

//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {
	srand(time(0));

	// 描画される文字列のサイズを変更
	SetFontSize(50);
}



//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {

	// 押しっぱなしを検知
	if (tnl::Input::IsKeyDown(eKeys::KB_Z)) {
		on_keep_count_up_value++;
	}

	// 押した瞬間を検知 ( 他の関数でも複数キーに対応 )
	if (tnl::Input::IsKeyDownTrigger(eKeys::KB_X, eKeys::KB_C)) {
		on_trigger_count_up_value++;
	}

	// 離した瞬間を検知
	if (tnl::Input::IsKeyReleaseTrigger(eKeys::KB_V)) {
		on_trigger_release_count_up_value++;
	}


	DrawFormatString(0, 0, -1, "押しっぱなし(Zキー) %d", on_keep_count_up_value);
	DrawFormatString(0, 50, -1, "押した瞬間 (Xキー もしくは Cキー) %d", on_trigger_count_up_value);
	DrawFormatString(0, 100, -1, "離した瞬間 (Vキー) %d", on_trigger_release_count_up_value);


	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });
}


//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {
}


#endif