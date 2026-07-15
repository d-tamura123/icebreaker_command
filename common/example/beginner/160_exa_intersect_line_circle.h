#if 0

//-----------------------------------------------------------------------------------------------------------
//
//
// 線分と円の衝突判定と補正サンプル
//
//
//-----------------------------------------------------------------------------------------------------------


#include <time.h>
#include <string>
#include <dxe.h>
#include "gm_main.h"


// 円の座標と半径
tnl::Vector2f circle_pos = { 100, 400 };
float circle_radius = 50;

// 線分の始点と終点
tnl::Vector2f line_s = { 500, 200 };
tnl::Vector2f line_e = { 700, 500 };


//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {
	srand(time(0));
	SetFontSize(30);
}

//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {

	// 円の移動前の座標を保存
	tnl::Vector2f before_circle = circle_pos;

	// 円の移動処理
	if (tnl::Input::IsKeyDown(eKeys::KB_A)) circle_pos.x -= 6;
	if (tnl::Input::IsKeyDown(eKeys::KB_D)) circle_pos.x += 6;
	if (tnl::Input::IsKeyDown(eKeys::KB_W)) circle_pos.y -= 6;
	if (tnl::Input::IsKeyDown(eKeys::KB_S)) circle_pos.y += 6;


	DrawFormatString(0, 0, -1, "WASD で 円 を移動");

	if (tnl::IsIntersectLineCircle(line_s, line_e, circle_pos, circle_radius)) {
		tnl::CorrectPositionLineCircle(line_s, line_e, before_circle, circle_radius, circle_pos);
	}

	DrawCircle(circle_pos.x, circle_pos.y, circle_radius, -1, false);

	DrawLine(line_s.x, line_s.y, line_e.x, line_e.y, -1);

	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });
}

//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {

}

#endif