#if 0

//---------------------------------------------------------------------------------------------------------------
//
//
// 2D カメラを適用したスクロール処理
// 
// ※ 2D ゲームでもカメラの概念を適用し、カメラを動かすことでオブジェクト全てを動かします
// 　カメラを使用することでオブジェクトの座標はスクリーン座標ではなく、ゲーム空間座標となります
//
//---------------------------------------------------------------------------------------------------------------


#include <time.h>
#include <string>
#include <dxe.h>
#include "gm_main.h"
#include "../ResourceConstantHedder.h"


const int FIELD_CHIP_NUM_W = 7;
const int FIELD_CHIP_NUM_H = 6;

int field[FIELD_CHIP_NUM_H][FIELD_CHIP_NUM_W] = {
	1, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 1,
	1, 1, 1, 1, 1, 1, 1,
};


//--------------------------------------------------------------------------------------------------
class Chip {
public:
	Chip(const tnl::Vector2f& pos) : pos_(pos) {}
	static constexpr float WIDTH = 64;
	static constexpr float HEIGHT = 64;

	tnl::Vector2f pos_;
	void draw(const Shared<dxe::Camera>& camera);
};

void Chip::draw(const Shared<dxe::Camera>& camera) {
	// ゲーム空間座標からスクリーン座標へ変換
	tnl::Vector2f draw_pos = camera->convertWorldToScreenPosition2D(pos_);
	dxe::DrawBox({ draw_pos.x, draw_pos.y }, WIDTH, HEIGHT, false, 0);
}


//--------------------------------------------------------------------------------------------------
class Player {
public:
	static constexpr float RADIUS = 20;
	tnl::Vector2f pos_ = { 0, 0 };
	void draw(const Shared<dxe::Camera>& camera);
};

void Player::draw(const Shared<dxe::Camera>& camera) {
	// ゲーム空間座標からスクリーン座標へ変換
	tnl::Vector2f draw_pos = camera->convertWorldToScreenPosition2D(pos_);
	DrawCircle(draw_pos.x, draw_pos.y, RADIUS, 0, false);
}

//--------------------------------------------------------------------------------------------------
class BackGround {
public:
	int gfx_hdl_ = 0;
	tnl::Vector2f pos_;
	void draw(const Shared<dxe::Camera>& camera);
};

void BackGround::draw(const Shared<dxe::Camera>& camera) {
	// ゲーム空間座標からスクリーン座標へ変換
	// 背景だけカメラの影響を弱めてスクロール速度を遅くするため arg2 に 0.1f を指定
	tnl::Vector2f draw_pos = camera->convertWorldToScreenPosition2D(pos_, 0.1f);
	DrawRotaGraph(draw_pos.x, draw_pos.y, 1.0f, 0, gfx_hdl_, true);
}


Player player;
BackGround back_ground;
Shared<dxe::Camera> camera;
std::list<Chip*> chips;


//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {
	srand(time(0));

	// 2D 用カメラを作成
	// arg1... 描画領域の幅
	// arg2... 描画領域の高さ
	// arg3... カメラタイプ( 2D or 3D )
	camera = std::make_shared<dxe::Camera>(DXE_WINDOW_WIDTH_F, DXE_WINDOW_HEIGHT_F, dxe::Camera::eDimension::Type2D);

	back_ground.gfx_hdl_ = LoadGraph(FILE_PATH_PNG_SKY01);


	// テスト用マップチップ作成
	float sx = Chip::WIDTH / 2;
	float sy = Chip::HEIGHT / 2;
	for (int i = 0; i < FIELD_CHIP_NUM_H; ++i) {
		for (int k = 0; k < FIELD_CHIP_NUM_W; ++k) {
			if (0 == field[i][k]) continue;
			chips.emplace_back(new Chip({ sx + k * Chip::WIDTH, sy + i * Chip::HEIGHT }));
		}
	}

}

//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {


	if (tnl::Input::IsKeyDown(eKeys::KB_A))	player.pos_.x -= 5;
	if (tnl::Input::IsKeyDown(eKeys::KB_D)) player.pos_.x += 5;
	if (tnl::Input::IsKeyDown(eKeys::KB_W))	player.pos_.y -= 5;
	if (tnl::Input::IsKeyDown(eKeys::KB_S))	player.pos_.y += 5;


	// カメラがプレイヤーを追いかけるように補間移動
	camera->addPosition((tnl::Vector3(player.pos_) - camera->getPosition()) * 0.05f);

	// カメラ情報の更新( 描画処理の前に実行すること )
	camera->update();


	// 以下描画処理
	back_ground.draw(camera);
	player.draw(camera);
	for (auto chip : chips) { chip->draw(camera); }


	//
	// マウスカーソル座標をゲーム空間座標へ変換
	//
	tnl::Vector3 scr_pos = tnl::Input::GetMousePosition();
	dxe::DrawBox({ scr_pos.x, scr_pos.y }, 10, 10, true, dxe::Colors::Red);

	tnl::Vector2f world_pos = camera->convertScreenToWorldPosition2D(tnl::Vector2f(scr_pos));
	DrawFormatString(0, 0, 0, "マウスカーソル座標をゲーム空間座標へ %s", world_pos.toString("%.2f").c_str());


	DrawFormatString(0, 20, 0, "プレイヤ座標( WASD で移動 ) %s", player.pos_.toString("%.2f").c_str());
	DrawFormatString(0, 40, 0, "カメラ座標 %s", camera->getPosition().toString("%.2f").c_str());

}

//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {

}


#endif