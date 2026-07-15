#if 0

//---------------------------------------------------------------------------------------------------------------
//
//
// プログラマブルメッシュ生成法　サンプル
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

//-------------------------------------------------------------------------------------------------------
//
// 座標と姿勢で制御するカメラ
//
class TransformCamera : public dxe::Camera {
public:

	TransformCamera() {};
	TransformCamera(float screen_w, float screen_h) : dxe::Camera(screen_w, screen_h) {}

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

void TransformCamera::update() {

	//
	// 姿勢パラメータからターゲット座標とアッパーベクトルを計算
	//

	target_ = position_ + tnl::Vector3::TransformCoord({ 0, 0, 1 }, rot_);
	up_ = tnl::Vector3::TransformCoord({ 0, 1, 0 }, rot_);
	dxe::Camera::update();
}



Shared<TransformCamera> camera = nullptr;


std::vector<Shared<dxe::Mesh>> mesh_tbl;
Shared<dxe::Mesh> mesh;
//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {
	srand(time(0));

	SetFogEnable(true);
	SetFogColor(32, 32, 32);
	SetFogStartEnd(500.0f, 1500.0f);
	ChangeLightTypeDir(VGet(0.0f, -1.0f, 1.0f));
	SetBackgroundColor(32, 32, 32);


	camera = std::make_shared<TransformCamera>(DXE_WINDOW_WIDTH_F, DXE_WINDOW_HEIGHT_F);
	camera->setPosition({ 370, 350, -540 });
	camera->rot_ = tnl::Quaternion(0.189f, -0.248f, 0.049f, 0.948f);

	auto texture = dxe::Texture::CreateFromFile("resource/graphics/example/test.jpg");

	tnl::Vector3 st = { -400, 0, 300 };

	// 球
	mesh = dxe::Mesh::CreateFromFileMV(FILE_PATH_MV1_SPHERE, 100);
	mesh->setPosition(st);
	mesh->setTexture(texture);
	mesh->setDebugLine(dxe::Mesh::fDebugLine::FLG_AABB | dxe::Mesh::fDebugLine::FLG_BD_SPHERE);
	mesh_tbl.emplace_back(mesh);
	st.x += 200;

	auto s = mesh->getBoundingSphereRadius();

	// 半球
	mesh = dxe::Mesh::CreateFromFileMV(FILE_PATH_MV1_DOME, 100);
	mesh->setPosition(st);
	mesh->setTexture(texture);
	mesh->setDebugLine(dxe::Mesh::fDebugLine::FLG_AABB | dxe::Mesh::fDebugLine::FLG_BD_SPHERE);
	mesh_tbl.emplace_back(mesh);
	st.x += 200;

	auto s1 = mesh->getBoundingBoxSize();
	auto s2 = mesh->getBoundingSphereRadius();

	// 四角形
	mesh = dxe::Mesh::CreateFromFileMV(FILE_PATH_MV1_PLANE, 100);
	mesh->setPosition(st);
	mesh->setTexture(texture);
	mesh->setDebugLine(dxe::Mesh::fDebugLine::FLG_AABB | dxe::Mesh::fDebugLine::FLG_BD_SPHERE);
	mesh_tbl.emplace_back(mesh);
	st.x += 200;

	// ディスク
	mesh = dxe::Mesh::CreateFromFileMV(FILE_PATH_MV1_DISK, 100);
	mesh->setPosition(st);
	mesh->setTexture(texture);
	mesh->setDebugLine(dxe::Mesh::fDebugLine::FLG_AABB | dxe::Mesh::fDebugLine::FLG_BD_SPHERE);
	mesh_tbl.emplace_back(mesh);
	st.x += 200;

	// 四角形( くり抜き )
	mesh = dxe::Mesh::CreateFromFileMV(FILE_PATH_MV1_HOLLOW_OUT_DISK_PLANE, 100);
	mesh->setPosition(st);
	mesh->setTexture(texture);
	mesh->setDebugLine(dxe::Mesh::fDebugLine::FLG_AABB | dxe::Mesh::fDebugLine::FLG_BD_SPHERE);
	mesh_tbl.emplace_back(mesh);
	st.x = -400;
	st.z -= 200;

	// 円柱
	mesh = dxe::Mesh::CreateFromFileMV(FILE_PATH_MV1_CYLINDER, 100);
	mesh->setPosition(st);
	mesh->setTexture(texture);
	mesh->setDebugLine(dxe::Mesh::fDebugLine::FLG_AABB | dxe::Mesh::fDebugLine::FLG_BD_SPHERE);
	mesh_tbl.emplace_back(mesh);
	st.x += 200;

	// 円錐
	mesh = dxe::Mesh::CreateFromFileMV(FILE_PATH_MV1_CONE, 100);
	mesh->setPosition(st);
	mesh->setTexture(texture);
	mesh->setDebugLine(dxe::Mesh::fDebugLine::FLG_AABB | dxe::Mesh::fDebugLine::FLG_BD_SPHERE);
	mesh_tbl.emplace_back(mesh);
	st.x += 200;

	// コーンシリンダー
	mesh = dxe::Mesh::CreateFromFileMV(FILE_PATH_MV1_CONE_CYLINDER, 100);
	mesh->setPosition(st);
	mesh->setTexture(texture);
	mesh->setDebugLine(dxe::Mesh::fDebugLine::FLG_AABB | dxe::Mesh::fDebugLine::FLG_BD_SPHERE);
	mesh_tbl.emplace_back(mesh);
	st.x += 200;

	// 樽
	mesh = dxe::Mesh::CreateFromFileMV(FILE_PATH_MV1_BARREL, 100);
	mesh->setPosition(st);
	mesh->setTexture(texture);
	mesh->setDebugLine(dxe::Mesh::fDebugLine::FLG_AABB | dxe::Mesh::fDebugLine::FLG_BD_SPHERE);
	mesh_tbl.emplace_back(mesh);
	st.x += 200;

	// 半樽
	mesh = dxe::Mesh::CreateFromFileMV(FILE_PATH_MV1_HALF_BARREL, 100);
	mesh->setPosition(st);
	mesh->setTexture(texture);
	mesh->setDebugLine(dxe::Mesh::fDebugLine::FLG_AABB | dxe::Mesh::fDebugLine::FLG_BD_SPHERE);
	mesh_tbl.emplace_back(mesh);
	st.x = -400;
	st.z -= 200;

	// 立方体
	mesh = dxe::Mesh::CreateFromFileMV(FILE_PATH_MV1_CUBE, 100);
	mesh->setPosition(st);
	mesh->setTexture(texture);
	mesh->setDebugLine(dxe::Mesh::fDebugLine::FLG_AABB | dxe::Mesh::fDebugLine::FLG_BD_SPHERE);
	mesh_tbl.emplace_back(mesh);
	st.x += 200;

	// トーラス
	mesh = dxe::Mesh::CreateTorusMV(50, 30);
	mesh->setPosition(st);
	mesh->setTexture(texture);
	mesh->setDebugLine(dxe::Mesh::fDebugLine::FLG_AABB | dxe::Mesh::fDebugLine::FLG_BD_SPHERE);
	mesh_tbl.emplace_back(mesh);
	st.x += 200;

	// 正三角形
	mesh = dxe::Mesh::CreateTriangleEquilateralMV(100);
	mesh->setPosition(st);
	mesh->setTexture(texture);
	mesh->setDebugLine(dxe::Mesh::fDebugLine::FLG_AABB | dxe::Mesh::fDebugLine::FLG_BD_SPHERE);
	mesh_tbl.emplace_back(mesh);
	st.x += 200;


	// 二等辺三角形
	mesh = dxe::Mesh::CreateTriangleIsoscelesMV({ 100, 150, 0 });
	mesh->setPosition(st);
	mesh->setTexture(texture);
	mesh->setDebugLine(dxe::Mesh::fDebugLine::FLG_AABB | dxe::Mesh::fDebugLine::FLG_BD_SPHERE);
	mesh_tbl.emplace_back(mesh);
	st.x += 200;


	// 直角三角形
	mesh = dxe::Mesh::CreateTriangleRightMV({ 100, 100, 0 });
	mesh->setPosition(st);
	mesh->setTexture(texture);
	mesh->setDebugLine(dxe::Mesh::fDebugLine::FLG_AABB | dxe::Mesh::fDebugLine::FLG_BD_SPHERE);
	mesh_tbl.emplace_back(mesh);
	st.x = -400;
	st.z -= 200;

	// ボックス
	mesh = dxe::Mesh::CreateBoxMV
	({ 100, 70, 60 }
		, dxe::Texture::CreateFromFile(FILE_PATH_BMP_BOX_LEFT)
		, dxe::Texture::CreateFromFile(FILE_PATH_BMP_BOX_RIGHT)
		, dxe::Texture::CreateFromFile(FILE_PATH_BMP_BOX_UP)
		, dxe::Texture::CreateFromFile(FILE_PATH_BMP_BOX_DOWN)
		, dxe::Texture::CreateFromFile(FILE_PATH_BMP_BOX_BACK)
		, dxe::Texture::CreateFromFile(FILE_PATH_BMP_BOX_FORWORD)
	);
	mesh->setPosition(st);
	mesh->setDebugLine(dxe::Mesh::fDebugLine::FLG_AABB | dxe::Mesh::fDebugLine::FLG_BD_SPHERE);
	mesh_tbl.emplace_back(mesh);
	st.x += 200;


	// ディスクリング
	mesh = dxe::Mesh::CreateDiskRingMV(50, 20);
	mesh->setPosition(st);
	mesh->setTexture(texture);
	mesh->setDebugLine(dxe::Mesh::fDebugLine::FLG_AABB | dxe::Mesh::fDebugLine::FLG_BD_SPHERE);
	mesh_tbl.emplace_back(mesh);
	st.x += 200;


}


//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {


	//----------------------------------------------------------------------------------------------------
	//
	// カメラ制御
	//
	tnl::Input::RunIndexKeyDown(
		[&](uint32_t index) {
			tnl::Vector3 v[4] = {
				camera->left(),
				camera->right(),
				tnl::Vector3::up(),
				tnl::Vector3::down()
			};
			camera->addPosition(v[index] * 3.0f);

		}, eKeys::KB_A, eKeys::KB_D, eKeys::KB_W, eKeys::KB_S);

	if (tnl::Input::IsMouseDown(tnl::Input::eMouse::RIGHT)) {
		tnl::Vector3 mvel = tnl::Input::GetMouseVelocity();
		camera->rot_ *= tnl::Quaternion::RotationAxis({ 0, 1, 0 }, tnl::ToRadian(mvel.x * 0.2f));
		camera->rot_ *= tnl::Quaternion::RotationAxis(camera->right(), tnl::ToRadian(mvel.y * 0.2f));
	}
	camera->addPosition(camera->forward().xz() * tnl::Input::GetMouseWheel() * 0.3f);



	camera->update();


	DrawGridGround(camera, 50, 20);

	for (auto mesh : mesh_tbl) {
		mesh->render(camera);
	}



	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
	DrawBox(DXE_WINDOW_WIDTH - 270, 15, DXE_WINDOW_WIDTH, 130, 0, true);
	DrawString(DXE_WINDOW_WIDTH - 260, 20, "右クリック : 向き", -1);
	DrawString(DXE_WINDOW_WIDTH - 260, 40, "ホイール : 前後", -1);
	DrawString(DXE_WINDOW_WIDTH - 260, 60, "A D : 左右移動", -1);
	DrawString(DXE_WINDOW_WIDTH - 260, 80, "W S : 上下移動", -1);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);


	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });

}

//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {

}

#endif