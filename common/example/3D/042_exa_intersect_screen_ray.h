#if 0

//-----------------------------------------------------------------------------------------------------------
//
//
// マウスカーソルからレイを飛ばして３次元空間に存在するオブジェクトと衝突判定をとるサンプル
//
//
//-----------------------------------------------------------------------------------------------------------

#include <time.h>
#include <string>
#include <numbers>
#include <functional>
#include <dxe.h>
#include "../ResourceConstantHedder.h"
#include "gm_main.h"


Shared<dxe::Camera> camera = nullptr;
Shared<dxe::Mesh> box = nullptr;


//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {
	srand(time(0));

	ChangeLightTypeDir(VGet(0.0f, -1.0f, 0.0f));
	SetBackgroundColor(32, 32, 32);

	camera = std::make_shared<dxe::Camera>(DXE_WINDOW_WIDTH_F, DXE_WINDOW_HEIGHT_F);

	box = dxe::Mesh::CreateCubeMV(100);
	box->setTexture(dxe::Texture::CreateFromFile("resource/graphics/example/test.jpg"));
	box->setPosition({ -100, 0, 0 });

}


//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {

	//-------------------------------------------------------------------------------
	//
	// update
	//
	//-------------------------------------------------------------------------------

	tnl::Vector3 ms = tnl::Input::GetMousePosition();

	tnl::Vector3 ray_nml = tnl::Vector3::CreateScreenRay(ms.x, ms.y, DXE_WINDOW_WIDTH_F, DXE_WINDOW_HEIGHT_F, camera->getViewMatrix(), camera->getProjectionMatrix());

	tnl::Vector3 pos = box->getPosition();

	if (tnl::IsIntersectRayAABB(
		camera->getPosition()
		, ray_nml
		, tnl::ToMaxAABB(pos, { 100, 100, 100 })
		, tnl::ToMinAABB(pos, { 100, 100, 100 })))
	{
		DrawFormatString(0, 30, -1, "hit");
	}

	camera->update();


	//-------------------------------------------------------------------------------
	//
	// draw
	//
	//-------------------------------------------------------------------------------

	DrawGridGround(camera, 50, 20);

	box->render(camera);

	DrawString(0, 0, "マウスカーソルをキューブに重ねてください", -1);

	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });

}

//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {

}


#endif