#if 0

//---------------------------------------------------------------------------------------------------------------
//
//
// CSV から読み込んだ フィールドマップと衝突判定と補正をおこなう
// 　
//
//---------------------------------------------------------------------------------------------------------------


#include <time.h>
#include <string>
#include <dxe.h>
#include "gm_main.h"


class Block {
public:
	Block(const tnl::Vector2f& pos) : pos_(pos) {}

	static constexpr float CHIP_SIZE = 64;

	tnl::Vector2f pos_ = { 0, 0 };
};

const int PLAYER_SIZE = 60;
float gravity = 0;
tnl::Vector2f player_pos = { 300, 300 };

std::vector<std::vector<int>> map_csv;

std::list<Block*> blocks;

//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {
	srand(time(0));

	map_csv = tnl::LoadCsv<int>("resource/csv/example_map_chip.csv");

	float sx = DXE_WINDOW_WIDTH_QUARTER_F;
	float sy = 100;

	for (int i = 0; i < map_csv.size(); ++i) {
		for (int k = 0; k < map_csv[i].size(); ++k) {
			if (0 == map_csv[i][k]) continue;
			tnl::Vector2f pos;
			pos.x = { sx + k * Block::CHIP_SIZE };
			pos.y = { sy + i * Block::CHIP_SIZE };
			blocks.emplace_back(new Block(pos));
		}
	}


}

//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {

	tnl::Vector2f prev_pos = player_pos;

	if (tnl::Input::IsKeyDown(eKeys::KB_A)) {
		player_pos.x -= 10;
	}
	if (tnl::Input::IsKeyDown(eKeys::KB_D)) {
		player_pos.x += 10;
	}
	if (tnl::Input::IsKeyDownTrigger(eKeys::KB_W)) {
		gravity = -20.0f;
	}

	gravity += 0.98;
	player_pos.y += gravity;

	// プレイヤーに近い順にソート
	blocks.sort([](Block* l, Block* r) {
		float ld = (player_pos - l->pos_).length();
		float rd = (player_pos - r->pos_).length();
		return ld < rd;
		});

	// ブロックと衝突判定 & 補正
	for (auto blk : blocks) {

		// 衝突判定
		if (tnl::IsIntersectRect(player_pos, PLAYER_SIZE, PLAYER_SIZE, blk->pos_, Block::CHIP_SIZE, Block::CHIP_SIZE)) {

			// 矩形同士の座標補正
			// arg1.2. 矩形 A, B の移動前の座標
			// arg3.4. 矩形 A, B のサイズ
			// arg5.6. 矩形 A, B の移動後の座標 ( この関数によって補正後の座標に更新 )
			// arg7.8. 横, 縦 方向の補正タイプ
			// ....... [ PWRFL_A : A が 強い ] [ PWRFL_B : B が 強い ] [ BOTH : A と B は押し合う ]
			// arg9... 補正後に A と B の間に設ける空間 ( デフォルト 1.0f )
			// ret.... 矩形 A が B に対してどの方向へ補正されたか
			tnl::eCorrResRect n = tnl::CorrectPositionRect(
				prev_pos
				, blk->pos_
				, { PLAYER_SIZE, PLAYER_SIZE }
				, { (int)Block::CHIP_SIZE, (int)Block::CHIP_SIZE }
				, player_pos
				, blk->pos_
				, tnl::eCorrTypeRect::PWRFL_B
				, tnl::eCorrTypeRect::PWRFL_B, 1.0f);

			// 上下に補正された場合は重力リセット
			if (n == tnl::eCorrResRect::UP || n == tnl::eCorrResRect::DOWN) {
				gravity = 0;
			}
		}

	}

	//-------------------------------------------------------------------
	//
	// 描画
	//

	for (auto blk : blocks) {
		dxe::DrawBox({ blk->pos_.x, blk->pos_.y }, Block::CHIP_SIZE, Block::CHIP_SIZE, false);
	}

	dxe::DrawBox({ player_pos.x, player_pos.y }, PLAYER_SIZE, PLAYER_SIZE, false, dxe::Colors::Red);

	DrawString(0, 0, "W : ジャンプ", -1);
	DrawString(0, 20, "AD : 左右移動", -1);
}

//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {

}

#endif
