#if 0

//---------------------------------------------------------------------------------------------------------------
//
//
// テクスチャへの描画サンプル
//
//
//---------------------------------------------------------------------------------------------------------------

#include <time.h>
#include <string>
#include <numbers>
#include <functional>
#include <dxe.h>
#include "../ResourceConstantHedder.h"
#include "gm_main.h"

Shared<dxe::Camera> camera = nullptr;
Shared<dxe::Mesh> mesh = nullptr;
Shared<dxe::Texture> texture = nullptr;

int test_gfx = 0;
int test_count = 0;

//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {
	srand(time(0));

	SetFogEnable(true);
	SetFogColor(32, 32, 32);
	SetFogStartEnd(500.0f, 1500.0f);
	ChangeLightTypeDir(VGet(0.0f, -1.0f, 1.0f));
	SetBackgroundColor(32, 32, 32);



	camera = std::make_shared<dxe::Camera>(DXE_WINDOW_WIDTH_F, DXE_WINDOW_HEIGHT_F);


	// 描画領域を生成
	test_gfx = MakeScreen(200, 200, TRUE);
	// DxLib のグラフィックハンドルからテクスチャを生成
	texture = dxe::Texture::CreateFromDxLibGraphHdl(test_gfx);

	mesh = dxe::Mesh::CreatePlaneMV({ 200, 200, 0 }, 20, 20);
	mesh->setTexture(texture);


	mesh->mulRotation(tnl::Quaternion::RotationAxis({ 0, 1, 0 }, tnl::ToRadian(-45)));
}


//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {


	test_count++;

	// 描画対象をテクスチャに変更
	SetDrawScreen(test_gfx);

	// テクスチャに対する描画処理
	ClearDrawScreen();
	DrawBox(0, 0, 200, 200, GetColor(128, 32, 32), TRUE);
	DrawFormatString(0, 0, -1, "abcd = %d", test_count);

	// 描画対象をバックバッファへ戻す
	SetDrawScreen(DX_SCREEN_BACK);



	camera->update();

	dxe::DrawGridGround(camera, 50, 20);

	mesh->render(camera);

	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });

}

//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {

}

#endif