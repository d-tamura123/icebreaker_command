#if 0

//---------------------------------------------------------------------------------------------------------------
//
//
// FPSカメラサンプル
//
// ※ FPS カメラはプレイヤー視点で操作されるカメラで、3Dオブジェクトと同様に座標と姿勢で制御します。
//    プレイヤーは視点を自由に上下左右に動かし、視界を回転させることができるため、主観視点のゲームでよく使用されます。
// 
//    FPS カメラは、カメラの視点がプレイヤーの位置に固定され、カメラの方向をマウスやキーボードによって操作します。
//    これにより、視点を上や下に向けたり、左右に回転させたりすることが可能です。
//
//    3Dオブジェクトの描画には LookAt カメラが保持するビュー行列とプロジェクション行列が必要なので、
//    FPS カメラは LookAt カメラを継承して作成します。
// 
//    また、FPSカメラの視点の動きは、移動（前進・後退・左右のストレイフ）と回転（ピッチとヨー）に分けて制御され、
//    これにより視界を自在に操作できる特徴を持ちます。
//
//---------------------------------------------------------------------------------------------------------------

#include <time.h>
#include <string>
#include <numbers>
#include <functional>
#include <dxe.h>
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
		target_ = position_ + tnl::Vector3::TransformCoord({ 0, 0, 1 }, rot_);
		return tnl::Vector3::Normalize(target_ - position_);
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

//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {
	srand(time(0));
	ChangeLightTypeDir(VGet(0.0f, -1.0f, 0.0f));
	SetBackgroundColor(32, 32, 32);

	camera = std::make_shared<FpsCamera>(DXE_WINDOW_WIDTH_F, DXE_WINDOW_HEIGHT_F);

}


//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {


	tnl::Input::RunIndexKeyDown(
		[&](uint32_t index) {
			tnl::Vector3 v[4] = {
				camera->left(),
				camera->right(),
				camera->forward().xz(),
				camera->back().xz()
			};
			camera->addPosition(v[index] * 3.0f);

		}, eKeys::KB_A, eKeys::KB_D, eKeys::KB_W, eKeys::KB_S);

	tnl::Vector3 mvel = tnl::Input::GetMouseVelocity();
	camera->rot_ *= tnl::Quaternion::RotationAxis({ 0, 1, 0 }, tnl::ToRadian(mvel.x * 0.1f));
	camera->rot_ *= tnl::Quaternion::RotationAxis(camera->right(), tnl::ToRadian(mvel.y * 0.1f));


	camera->update();

	DrawFormatString(0, 0, -1, "WASD で移動");
	DrawFormatString(0, 20, -1, "マウス操作で視線移動");

	DrawGridGround(camera, 50, 20);

	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });

}

//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {

}

#endif