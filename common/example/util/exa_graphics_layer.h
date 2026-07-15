#if 0
//-----------------------------------------------------------------------------------------------------------
//
//
// グラフィックスレイヤー利用サンプル
//
//
//-----------------------------------------------------------------------------------------------------------

#include <time.h>
#include <string>
#include <numbers>
#include <functional>
#include <dxe.h>
#include "gm_main.h"


Shared<dxe::Camera> camera = nullptr;
Shared<dxe::Mesh> mesh = nullptr;
Shared<dxe::Mesh> ground = nullptr;

Shared<dxe::GraphicsLayer> layer1 = nullptr;
Shared<dxe::GraphicsLayer> layer2 = nullptr;


//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {
	srand(time(0));

	ChangeLightTypeDir(VGet(0.0f, -1.0f, 0.0f));
	SetBackgroundColor(32, 32, 32);

	camera = std::make_shared<dxe::Camera>(DXE_WINDOW_WIDTH_F, DXE_WINDOW_HEIGHT_F);
	camera->setPosition({ 0, 300, -700 });

	mesh = dxe::Mesh::CreateSphereMV(100, 20, 20);
	mesh->setTexture(dxe::Texture::CreateFromFile("resource/graphics/example/test.jpg"));
	mesh->setPosition({ 0, 100, 0 });

	ground = dxe::Mesh::CreatePlaneMV({ 2000, 2000, 0 }, 20, 20);
	ground->setTexture(dxe::Texture::CreateFromFile("resource/graphics/example/lawn.png"));
	ground->setRotation(tnl::Quaternion::RotationAxis({ 1, 0, 0 }, tnl::ToRadian(90)));


	layer1 = std::make_shared<dxe::GraphicsLayer>(tnl::Vector2i(DXE_WINDOW_WIDTH, DXE_WINDOW_HEIGHT));

	// レイヤー２を生成
	layer2 = std::make_shared<dxe::GraphicsLayer>(tnl::Vector2i(DXE_WINDOW_WIDTH, DXE_WINDOW_HEIGHT));
	// レイヤー２をメイン画面に描画する時のブレンドモードを設定
	layer2->setBlendMode(DX_BLENDMODE_ADD);
	// レイヤー２に対して使用する画面効果を設定 ( ブルームとブラー )
	layer2->setAdoption(dxe::GraphicsLayer::fAdoption::BLOOM | dxe::GraphicsLayer::fAdoption::BLUR);


	// レイヤーの設定を外部ファイルからロードして作成するにはこちら
	//layer2 = std::make_shared<dxe::GraphicsLayer>("resource/dxe_parameters/graphics_layer/graphics_layer_status.bin");


}


//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {

	mesh->mulRotation(tnl::Quaternion::RotationAxis({ 0, 1, 0 }, tnl::ToRadian(1)));

	camera->update();


	// レイヤー1 に描画
	layer1->write([&]() {

		ground->render(camera);
		mesh->render(camera);

		});

	// レイヤー2 に描画
	layer2->write([&]() {

		mesh->render(camera);

		});

	// レイヤー１・２をメイン画面に描画
	layer1->draw();
	layer2->draw();


	// レイヤーのステータスを GUIで調整
	dxe::ImGuiNewFrame();

	layer1->drawImGuiController({ 0, 0 }, false, "layer1");
	layer2->drawImGuiController({ 0, 20 }, true, "layer2");

	dxe::ImGuiRender();

	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });

}

//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {
}

#endif