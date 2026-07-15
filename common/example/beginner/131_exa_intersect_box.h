#if 0

//-----------------------------------------------------------------------------------------------------------
//
//
// 矩形同士の衝突判定と補正サンプル ( 軽量版 )
//
//
//-----------------------------------------------------------------------------------------------------------


#include <time.h>
#include <string>
#include <dxe.h>
#include "gm_main.h"


tnl::Vector2f a_pos = { DXE_WINDOW_WIDTH_HALF_F, 200 };
tnl::Vector2f a_size = { 50, 50 };

tnl::Vector2f b_pos = { DXE_WINDOW_WIDTH_HALF_F, DXE_WINDOW_HEIGHT_HALF_F };
tnl::Vector2f b_size = { 100, 50 };



//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {
	srand(time(0));
}


//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {

	if (tnl::Input::IsKeyDown(eKeys::KB_A)) a_pos.x -= 5.0f;
	if (tnl::Input::IsKeyDown(eKeys::KB_D)) a_pos.x += 5.0f;
	if (tnl::Input::IsKeyDown(eKeys::KB_W)) a_pos.y -= 5.0f;
	if (tnl::Input::IsKeyDown(eKeys::KB_S)) a_pos.y += 5.0f;


	if (tnl::IsIntersectRect(a_pos, a_size, b_pos, b_size)) {

		// 矩形同士の座標補正( 軽量版 )
		// arg1 補正対象の座標 ( この関数によって補正後の座標に更新 )
		// arg2 補正対象のサイズ
		// arg3 不動矩形の座標
		// arg4 不動矩形のサイズ
		// arg5 補正後に間に設ける空間 ( デフォルト 0.0f )
		// tips 補正対象の移動量が大きい場合や押し押されが必要な場合は Easy が付いていない補正関数を推奨
		tnl::eCorrResRect corr = tnl::EasyCorrectPositionRect(a_pos, a_size, b_pos, b_size);

		std::string s;
		switch (corr) {
		case tnl::eCorrResRect::LEFT: s = "左"; break;
		case tnl::eCorrResRect::RIGHT: s = "右"; break;
		case tnl::eCorrResRect::UP: s = "上"; break;
		case tnl::eCorrResRect::DOWN: s = "下"; break;
		}
		DrawFormatString(0, 50, -1, "%s に補正", s.c_str());
	}

	dxe::DrawBox(a_pos, a_size.x, a_size.y, false);
	dxe::DrawBox(b_pos, b_size.x, b_size.y, false);

	DrawFormatString(0, 0, -1, "WASD で矩形の移動");

	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });
}


//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {
}


#endif