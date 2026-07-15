#if 0

//---------------------------------------------------------------------------------------------------------------
//
//
// [ 入門 ] 衝突判定サンプル マウスカーソルと矩形
//
//
//---------------------------------------------------------------------------------------------------------------


#include <time.h>
#include <string>
#include <dxe.h>
#include "gm_main.h"


// ボックスのサイズ
int size_w = 200;
int size_h = 200;

// ボックスの中心座標
tnl::Vector2f box_pos = { 600, 300 };


//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {
	srand(time(0));
	SetFontSize(30);
}


//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {

	tnl::Vector3 mouse_pos = tnl::Input::GetMousePosition();

	DrawFormatString(0, 0, -1, "マウスカーソルと矩形の衝突判定");


	if (tnl::IsIntersectPointRect(mouse_pos.x, mouse_pos.y, box_pos.x, box_pos.y, size_w, size_h)) {
		DrawFormatString(0, 30, -1, "hit");
	}

	dxe::DrawBox(box_pos, size_w, size_h, false);


	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });
}


//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {
}


#endif