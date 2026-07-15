
#if 0

//-----------------------------------------------------------------------------------------------------------
//
//
// カプセル( 移動している球体 )と 球 の衝突判定と補正サンプル
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



//-------------------------------------------------------------------------------------------------------
//
// 座標と姿勢で制御するカメラ
//
class FpsCamera : public dxe::Camera {
public:

	FpsCamera() {};
	FpsCamera(float screen_w, float screen_h) : dxe::Camera(screen_w, screen_h) {}

	void update();


	inline tnl::Vector3 up() {
		up_ = tnl::Vector3::TransformCoord({ 0, 1, 0 }, rot_);
		return up_;
	}
	inline tnl::Vector3 down() { return -up(); }

	inline tnl::Vector3 forward() override {
		return tnl::Vector3::TransformCoord({ 0, 0, 1 }, rot_);
	}
	inline tnl::Vector3 back() override { return -forward(); }
	inline tnl::Vector3 left() override { return tnl::Vector3::Cross(forward(), up()); }
	inline tnl::Vector3 right() override { return tnl::Vector3::Cross(up(), forward()); }

	tnl::Quaternion rot_;

};

void FpsCamera::update() {

	//
	// 姿勢パラメータからターゲット座標とアッパーベクトルを計算
	//

	target_ = position_ + tnl::Vector3::TransformCoord({ 0, 0, 1 }, rot_);
	up_ = tnl::Vector3::TransformCoord({ 0, 1, 0 }, rot_);
	dxe::Camera::update();
}


Shared<FpsCamera> camera = nullptr;
Shared<dxe::Mesh> mesh = nullptr;

Shared<dxe::Mesh> sphere = nullptr;
float sphere_r = 100;

tnl::Vector3 cap_st = { 100, 100, -100 };
tnl::Vector3 cap_en = { 0, -100, 300 };
float cap_r = 30;

//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {
	srand(time(0));
	ChangeLightTypeDir(VGet(0.0f, -1.0f, 0.0f));
	SetBackgroundColor(32, 32, 32);


	camera = std::make_shared<FpsCamera>(DXE_WINDOW_WIDTH_F, DXE_WINDOW_HEIGHT_F);

	mesh = dxe::Mesh::CreateSphereMV(cap_r, 20, 20);
	mesh->setTexture(dxe::Texture::CreateFromFile("resource/graphics/example/test.jpg"));


	sphere = dxe::Mesh::CreateSphereMV(sphere_r, 20, 20);
	sphere->setTexture(dxe::Texture::CreateFromFile("resource/graphics/example/test.jpg"));
	sphere->setPosition({ 0, 0, 100 });

}


//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {


	tnl::Input::RunIndexKeyDown(
		[&](uint32_t index) {
			tnl::Vector3 v[6] = {
				camera->left(),
				camera->right(),
				camera->forward().xz(),
				camera->back().xz(),
				{0, 1, 0},
				{0, -1, 0}
			};
			camera->addPosition(v[index] * 3.0f);

		}, eKeys::KB_A, eKeys::KB_D, eKeys::KB_W, eKeys::KB_S, eKeys::KB_Z, eKeys::KB_X);

	tnl::Vector3 mvel = tnl::Input::GetMouseVelocity();
	camera->rot_ *= tnl::Quaternion::RotationAxis({ 0, 1, 0 }, tnl::ToRadian(mvel.x * 0.1f));
	camera->rot_ *= tnl::Quaternion::RotationAxis(camera->right(), tnl::ToRadian(mvel.y * 0.1f));

	tnl::Input::RunIndexKeyDown(
		[&](uint32_t index) {
			tnl::Vector3 v[6] = {
				camera->left(),
				camera->right(),
				camera->up(),
				camera->down(),
			};
			cap_en += v[index] * 5;
		}, eKeys::KB_LEFT, eKeys::KB_RIGHT, eKeys::KB_UP, eKeys::KB_DOWN);
	mesh->setPosition(cap_en);


	//-------------------------------------------------------------------------------------------------
	// カプセルと球の衝突判定
	// arg1... カプセルの始点
	// arg2... カプセルの終点
	// arg3... カプセルの半径
	// arg4... 球 の中心座標
	// arg5... 球 の半径
	// arg6... カプセルの始点(t = 0.0f) から終点(t = 1.0f)方向で衝突が起こった地点
	// ret.... [衝突している : true] [衝突していない : false]
	float t = 0;
	if (tnl::IsIntersectCapsuleSphere(cap_st, cap_en, cap_r, sphere->getPosition(), sphere_r, &t)) {
		mesh->setPosition(cap_st + (cap_en - cap_st) * t);
		DrawFormatString(0, 60, -1, "t : %.2f", t);
	}

	camera->update();

	DrawFormatString(0, 0, -1, "WASDZX でカメラ移動");
	DrawFormatString(0, 20, -1, "マウス操作で視線移動");
	DrawFormatString(0, 40, -1, "カーソルキーでカプセルの終点移動");

	sphere->render(camera);
	dxe::DrawCapsule(camera, cap_st, cap_en, cap_r, dxe::Colors::Yellow);


	mesh->render(camera);

	DrawGridGround(camera, 50, 20);

	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });

}

//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {

}

#endif
