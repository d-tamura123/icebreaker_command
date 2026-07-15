#if 0

//-----------------------------------------------------------------------------------------------------------
//
// スカイボックス利用サンプル
//
// ※ フォグを利用する場合はスカイボックスを描画した後に設定しましょう
// ※ スカイボックスは大きく作るのでカメラの far 設定次第では描画されなくなるので注意
// ※ スカイボックスのマテリアルはエミッシヴにのみ色を入れて他は切りましょう
//-----------------------------------------------------------------------------------------------------------

#include <time.h>
#include <string>
#include <numbers>
#include <functional>
#include <dxe.h>
#include "../ResourceConstantHedder.h"
#include "gm_main.h"

Shared<dxe::Camera> camera = nullptr;
Shared<dxe::Mesh> skybox = nullptr;
dxe::GuiMenuSelector* gui_menu_selector = nullptr;

//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {
	srand(time(0));

	ChangeLightTypeDir(VGet(0.0f, -1.0f, 0.0f));
	SetBackgroundColor(32, 32, 32);

	camera = std::make_shared<dxe::Camera>(DXE_WINDOW_WIDTH_F, DXE_WINDOW_HEIGHT_F);

	skybox = dxe::Mesh::CreateCubeMV(30000, 20, 20);
	skybox->setDefaultLightEnable(false);
	skybox->setTexture(dxe::Texture::CreateFromFile("resource/graphics/example/skybox/skybox_a.png"));
	skybox->loadMaterial(FILE_PATH_BIN_SKYBOX_MATERIAL);

	gui_menu_selector = new dxe::GuiMenuSelector(
		[&](uint32_t select_index) {

			std::string path[5] = {
				"resource/graphics/example/skybox/skybox_a.png",
				"resource/graphics/example/skybox/skybox_b.png",
				"resource/graphics/example/skybox/skybox_c.png",
				"resource/graphics/example/skybox/Magical.png"
			};
			skybox->setTexture(dxe::Texture::CreateFromFile(path[select_index]));


		}, "skybox_a", "skybox_b", "skybox_c", "Magical"
	);
	gui_menu_selector->setPosition({ 330, 0 });
	gui_menu_selector->setLocation(dxe::GuiMenuSelector::eLocation::DOWN);
}


//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {

	//-------------------------------------------------------------------------------
	//
	// update
	//
	//-------------------------------------------------------------------------------

	skybox->mulRotation(tnl::Quaternion::RotationAxis({ 0, 1, 0 }, tnl::ToRadian(0.01f)));

	camera->update();
	gui_menu_selector->update();


	//-------------------------------------------------------------------------------
	//
	// draw
	//
	//-------------------------------------------------------------------------------

	skybox->render(camera);

	DrawGridGround(camera, 50, 20);

	skybox->drawGuiMaterialControlloer();

	gui_menu_selector->draw();

	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });

}

//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {

}

#endif
