#if 0

//-----------------------------------------------------------------------------------------------------------
//
//
// 矩形と円の衝突判定と補正サンプル
//
//
//-----------------------------------------------------------------------------------------------------------


#include <time.h>
#include <string>
#include <dxe.h>
#include "gm_main.h"


// 円の座標と半径
tnl::Vector2f posA = { 100, 400 };
float radiusA = 50;

// 矩形の座標とサイズ
tnl::Vector2f posB = { 600, 400 };
tnl::Vector2f sizeB = { 200, 300 };


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
	tnl::Vector2f before_circle = posA;

	// それぞれの移動処理
	if (tnl::Input::IsKeyDown(eKeys::KB_A)) posA.x -= 6;
	if (tnl::Input::IsKeyDown(eKeys::KB_D)) posA.x += 6;
	if (tnl::Input::IsKeyDown(eKeys::KB_W)) posA.y -= 6;
	if (tnl::Input::IsKeyDown(eKeys::KB_S)) posA.y += 6;

	if (tnl::Input::IsKeyDown(eKeys::KB_LEFT)) posB.x -= 5;
	if (tnl::Input::IsKeyDown(eKeys::KB_RIGHT)) posB.x += 5;
	if (tnl::Input::IsKeyDown(eKeys::KB_UP))	posB.y -= 5;
	if (tnl::Input::IsKeyDown(eKeys::KB_DOWN)) posB.y += 5;


	DrawFormatString(0, 0, -1, "WASD で 円 を移動");
	DrawFormatString(0, 30, -1, "カーソルキー で 矩形 を移動");

	if (tnl::IsIntersectRectCircle(posB, sizeB, posA, radiusA)) {
		tnl::CorrectPositionRectCircle(posB, sizeB, before_circle, radiusA, posA);
	}

	DrawCircle(posA.x, posA.y, radiusA, -1, false);

	dxe::DrawBox({ posB.x, posB.y }, sizeB.x, sizeB.y, false, -1);

	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });
}

//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {

}

#endif