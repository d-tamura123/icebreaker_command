#if 0

//---------------------------------------------------------------------------------------------------------------
//
//
// 3D で砲身を付けた戦車のクラスを作るサンプル
// このサンプルでは複数のメッシュを持つクラスを作りそれぞれのメッシュは
// 本体クラスのパーツとして扱い、個別に動かしたり回転させる方法のサンプルです
//
//---------------------------------------------------------------------------------------------------------------
#include <time.h>
#include <string>
#include <dxe.h>
#include "../ResourceConstantHedder.h"
#include "gm_main.h"


class Tank {
public:
	enum class eParts {
		Body,
		GunBarrel,
		Max
	};

	Tank();

	// トランスフォーム情報の更新
	void transformUpdate();

	// 描画
	void draw(const Shared<dxe::Camera>& camera);

	// タンクとしての座標と回転
	tnl::Vector3 position_;
	tnl::Quaternion rotation_;

	// 各パーツのオフセット情報
	tnl::Vector3 pts_ofs_pos_[static_cast<int>(eParts::Max)];
	tnl::Quaternion pts_ofs_rot_[static_cast<int>(eParts::Max)];

	// パーツを個別に回転させるための保持情報
	float ballrel_rot_y_ = 0;
	float ballrel_rot_x_ = 0;	// 砲身はローカルな x 軸回転をさせる

	// パーツメッシュ
	Shared<dxe::Mesh> pts_meshs_[static_cast<int>(eParts::Max)];

	// パーツのサイズ情報定数
	const tnl::Vector3 BODY_SIZE = { 100, 50, 200 };
	const float GUN_BARREL_RADIUS = 10;
	const float GUN_BARREL_LENGTH = 200;
};


Tank::Tank() {
	pts_ofs_pos_[static_cast<int>(eParts::Body)] = tnl::Vector3{ 0, 0, 0 };
	pts_ofs_pos_[static_cast<int>(eParts::GunBarrel)] =
		tnl::Vector3{
			0,
			BODY_SIZE.y * 0.5f + GUN_BARREL_RADIUS,
			GUN_BARREL_LENGTH * 0.5f
	};

	pts_meshs_[static_cast<int>(eParts::Body)] = dxe::Mesh::CreateBoxMV(
		BODY_SIZE
		, dxe::Texture::CreateFromFile(FILE_PATH_BMP_BOX_LEFT)
		, dxe::Texture::CreateFromFile(FILE_PATH_BMP_BOX_RIGHT)
		, dxe::Texture::CreateFromFile(FILE_PATH_BMP_BOX_UP)
		, dxe::Texture::CreateFromFile(FILE_PATH_BMP_BOX_DOWN)
		, dxe::Texture::CreateFromFile(FILE_PATH_BMP_BOX_BACK)
		, dxe::Texture::CreateFromFile(FILE_PATH_BMP_BOX_FORWORD)
	);
	pts_meshs_[static_cast<int>(eParts::GunBarrel)] = dxe::Mesh::CreateCylinderMV(
		GUN_BARREL_RADIUS,
		GUN_BARREL_LENGTH
	);
	pts_meshs_[static_cast<int>(eParts::GunBarrel)]->setTexture(
		dxe::Texture::CreateFromFile(FILE_PATH_JPG_TEST)
	);
}


void Tank::transformUpdate() {
	const int idx_body = static_cast<int>(eParts::Body);
	const int idx_barrel = static_cast<int>(eParts::GunBarrel);

	pts_ofs_rot_[idx_barrel] =
		tnl::Quaternion::RotationAxis(
			tnl::Vector3::TransformCoord({ 1, 0, 0 }, rotation_), tnl::ToRadian(90)
		);

	auto body = pts_meshs_[idx_body];
	auto barrel = pts_meshs_[idx_barrel];

	//----------------------------------------------------------------------------------------------
	//
	// 姿勢計算
	// 

	body->setRotation(rotation_ * pts_ofs_rot_[idx_body]);

	// 砲身をワールド Y 軸 で回転させる姿勢の作成
	tnl::Quaternion qtn_bal_rot_y = tnl::Quaternion::RotationAxis({ 0, 1, 0 }, ballrel_rot_y_);

	// 砲身のワールド空間の基本姿勢
	tnl::Quaternion qtn_bal_world_base_rot = rotation_ * pts_ofs_rot_[idx_barrel] * qtn_bal_rot_y;

	// 砲身のローカル X 軸回転させる姿勢の作成
	// ( ワールド基本姿勢から横軸を計算し それを X 回転軸とする )
	tnl::Quaternion qtn_bal_rot_x = tnl::Quaternion::RotationAxis(
		tnl::Vector3::TransformCoord({ 1, 0, 0 }, qtn_bal_world_base_rot), ballrel_rot_x_
	);

	barrel->setRotation(qtn_bal_world_base_rot * qtn_bal_rot_x);

	//----------------------------------------------------------------------------------------------
	//
	// 座標計算
	// 

	body->setPosition(position_ + pts_ofs_pos_[idx_body]);

	// 回転した砲身の座標
	// タンク本体の座標からオフセット分ずらした位置に配置する
	// Y 座標は単純に本体座標に足す
	// XZ 座標は砲身の基本姿勢( 現状寝ている姿勢 )から ローカルな Z方向にずらす必要がある
	float bal_ofs_x = pts_ofs_pos_[idx_barrel].x;
	float bal_ofs_y = pts_ofs_pos_[idx_barrel].y;
	float bal_ofs_z = pts_ofs_pos_[idx_barrel].z;
	barrel->setPosition(
		position_ +
		tnl::Vector3(0, bal_ofs_y, 0) +
		tnl::Vector3::TransformCoord({ 0, 0, bal_ofs_z }, rotation_ * qtn_bal_rot_y * qtn_bal_rot_x)
	);

}


void Tank::draw(const Shared<dxe::Camera>& camera) {
	const int idx_body = static_cast<int>(eParts::Body);
	const int idx_barrel = static_cast<int>(eParts::GunBarrel);
	auto body = pts_meshs_[idx_body];
	auto barrel = pts_meshs_[idx_barrel];
	body->render(camera);
	barrel->render(camera);
}


Shared<Tank> tank;
Shared<dxe::Camera> camera;


//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {
	srand(time(0));

	camera = std::make_shared<dxe::Camera>(DXE_WINDOW_WIDTH_F, DXE_WINDOW_HEIGHT_F);
	tank = std::make_shared<Tank>();
}


//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {

	if (tnl::Input::IsKeyDown(eKeys::KB_A)) {
		tank->rotation_ *= tnl::Quaternion::RotationAxis({ 0, 1, 0 }, tnl::ToRadian(-1));
	}
	if (tnl::Input::IsKeyDown(eKeys::KB_D)) {
		tank->rotation_ *= tnl::Quaternion::RotationAxis({ 0, 1, 0 }, tnl::ToRadian(1));
	}
	if (tnl::Input::IsKeyDown(eKeys::KB_W)) {
		tank->position_ += tnl::Vector3::TransformCoord({ 0, 0, 3 }, tank->rotation_);
	}
	if (tnl::Input::IsKeyDown(eKeys::KB_S)) {
		tank->position_ += tnl::Vector3::TransformCoord({ 0, 0, -3 }, tank->rotation_);
	}

	if (tnl::Input::IsKeyDown(eKeys::KB_LEFT)) {
		tank->ballrel_rot_y_ -= tnl::ToRadian(1);
	}
	if (tnl::Input::IsKeyDown(eKeys::KB_RIGHT)) {
		tank->ballrel_rot_y_ += tnl::ToRadian(1);
	}
	if (tnl::Input::IsKeyDown(eKeys::KB_UP)) {
		tank->ballrel_rot_x_ -= tnl::ToRadian(1);
	}
	if (tnl::Input::IsKeyDown(eKeys::KB_DOWN)) {
		tank->ballrel_rot_x_ += tnl::ToRadian(1);
	}


	tank->transformUpdate();

	camera->update();




	dxe::DrawGridGround(camera, 50, 20);

	tank->draw(camera);


	DrawString(0, 0, "WASD でタンクの移動・回転", -1);
	DrawString(0, 20, "カーソル で砲身の回転", -1);

	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });
}


//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {
}


#endif