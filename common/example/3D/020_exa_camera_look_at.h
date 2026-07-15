#if 0

//---------------------------------------------------------------------------------------------------------------
//
// LookAt カメラ
// 
//　※ 基本となる情報のみをもっているカメラです
//　　 3D オブジェクトを画面に描画するにあたり投影変換を行います
// 　　投影変換はシェーダによって処理されますが、頂点座標に対して
// 　　MVP( モデルビュー投影行列 ) が掛け合わされます
// 　　MVP行列は次の３つの行列を掛け合わせることで生成されます
// 　　１．モデル行列　3D オブジェクトの座標(行列)・回転(行列)・スケール(行列) を掛け合わせた行列
// 　　２．ビュー行列　カメラの座標や注視点などから生成される
// 　　３．プロジェクション行列　カメラの視野角・アスペクト比などから生成される
// 
//　LookAt カメラとは投影変換を行うために必要な最小限の情報で構成され
//　ユーザーはカメラの座標と注視点（および上方向ベクトル）を操作して制御するカメラです
// 
//---------------------------------------------------------------------------------------------------------------

#include <time.h>
#include <string>
#include <numbers>
#include <functional>
#include <dxe.h>
#include "gm_main.h"

Shared<dxe::Camera> camera = nullptr;

//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {
	srand(time(0));

	SetFogEnable(true);
	SetFogColor(32, 32, 32);
	SetFogStartEnd(500.0f, 1500.0f);
	ChangeLightTypeDir(VGet(0.0f, -1.0f, 0.0f));
	SetBackgroundColor(32, 32, 32);


	camera = std::make_shared<dxe::Camera>(DXE_WINDOW_WIDTH_F, DXE_WINDOW_HEIGHT_F);

}


//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {

	float move = 3.0f;

	if (tnl::Input::IsKeyDown(eKeys::KB_LEFT)) camera->addPosition({ -move, 0, 0 });
	if (tnl::Input::IsKeyDown(eKeys::KB_RIGHT)) camera->addPosition({ move, 0, 0 });
	if (tnl::Input::IsKeyDown(eKeys::KB_UP)) camera->addPosition({ 0, move, 0 });
	if (tnl::Input::IsKeyDown(eKeys::KB_DOWN)) camera->addPosition({ 0, -move, 0 });

	if (tnl::Input::IsKeyDown(eKeys::KB_A)) camera->addTarget({ -move, 0, 0 });
	if (tnl::Input::IsKeyDown(eKeys::KB_D)) camera->addTarget({ move, 0, 0 });
	if (tnl::Input::IsKeyDown(eKeys::KB_W)) camera->addTarget({ 0, move, 0 });
	if (tnl::Input::IsKeyDown(eKeys::KB_S)) camera->addTarget({ 0, -move, 0 });


	// カメラ情報の更新
	// 描画処理を行う前に毎フレーム実行
	camera->update();

	DrawGridGround(camera, 50, 20);

	DrawString(0, 0, "←↑→↓ で座標移動", -1);
	DrawString(0, 20, "WASD でターゲット移動", -1);

	DrawFormatString(0, 50, -1, "position = %s", camera->getPosition().toString("%.2f").c_str());
	DrawFormatString(0, 70, -1, "target = %s", camera->getTarget().toString("%.2f").c_str());


	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });

}

//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {

}

#endif