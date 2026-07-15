#if 0

//---------------------------------------------------------------------------------------------------------------
//
//
// ブレンドモードサンプル
// 
// ※ 半透明・加算合成・減算合成
// 　
//
//---------------------------------------------------------------------------------------------------------------


#include <time.h>
#include <string>
#include <dxe.h>
#include "gm_main.h"


int back_ground_hdl = 0;
int alpha_image_hdl = 0;
int add_image_hdl = 0;
int sub_image_hdl = 0;

tnl::Vector3 pos[3];

//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {
	srand(time(0));
	back_ground_hdl = LoadGraph("resource/graphics/example/Sifi_town3_6n.png");
	alpha_image_hdl = LoadGraph("resource/graphics/example/stone.png");
	add_image_hdl = LoadGraph("resource/graphics/example/point.jpg");
	sub_image_hdl = LoadGraph("resource/graphics/example/scissors.png");


	pos[0] = { DXE_WINDOW_WIDTH_QUARTER_F, DXE_WINDOW_HEIGHT_HALF_F, 0 };
	pos[1] = { DXE_WINDOW_WIDTH_HALF_F, DXE_WINDOW_HEIGHT_HALF_F, 0 };
	pos[2] = { DXE_WINDOW_WIDTH_HALF_F + DXE_WINDOW_WIDTH_QUARTER_F, DXE_WINDOW_HEIGHT_HALF_F, 0 };

}

//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {

	/* SetDrawBlendMode は一度設定すると、再設定するまでずっとその設定で描画されます */

	// 不透明描画
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
	DrawRotaGraph(DXE_WINDOW_WIDTH / 2, DXE_WINDOW_HEIGHT / 2, 1.0f, 0, back_ground_hdl, true);

	// 半透明描画
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 64);
	DrawRotaGraph(pos[0].x, pos[0].y, 1.0f, 0, alpha_image_hdl, true);

	// 加算半透明描画
	SetDrawBlendMode(DX_BLENDMODE_ADD, 255);
	DrawRotaGraph(pos[1].x, pos[1].y, 1.0f, 0, add_image_hdl, true);

	// 減算半透明描画
	SetDrawBlendMode(DX_BLENDMODE_SUB, 255);
	DrawRotaGraph(pos[2].x, pos[2].y, 1.0f, 0, sub_image_hdl, true);
}

//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {

	// LoadGraph によって確保した画像のメモリを解放
	DeleteGraph( back_ground_hdl );
	DeleteGraph( alpha_image_hdl );
	DeleteGraph( add_image_hdl );
	DeleteGraph( sub_image_hdl );

}

#endif
