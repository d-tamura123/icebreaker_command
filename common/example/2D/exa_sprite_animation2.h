#if 0

//---------------------------------------------------------------------------------------------------------------
//
//
// dxe::Sprite アニメーション機能サンプル
// 
// 簡易的なゲームキャラクタクラスにスプライトを包含して
// 複数のアニメーションを登録し使用するサンプルです
//
//---------------------------------------------------------------------------------------------------------------

#include <time.h>
#include <string>
#include <dxe.h>
#include "../ResourceConstantHedder.h"
#include "gm_main.h"


// 簡易的ゲームキャラクラス
class Character {
public:
	Character();
	~Character() = default;

	// 更新
	// メインループで毎回実行
	void update();

	// 描画
	// arg1... 2D カメラインスタンス
	// メインループで毎回実行
	void draw(const Shared<dxe::Camera>& camera);

	TNL_PROPERTY(tnl::Vector2f, Position, position_);

private:

	// アニメーションの左右を描画の
	// 左右反転で行うための姿勢定数
	enum class ePosture {
		Right,		// 右向き
		Left		// 左向き
	};

	// キャラクタの行動制御ステートマシン
	Shared<tnl::StateMachine> state_flow_;

	// スプライト ( アニメーション機能を使用 )
	Shared<dxe::Sprite> sprite_;

	// 入力オブジェクト
	Shared<dxe::Input> input_;

	// 座標 ( 描画時にこの座標をスプライトの座標に設定して描画 )
	tnl::Vector2f position_;

	// 複数アニメーションの切り替え用
	// アニメーションを作成して各変数に制御インデックスを保存して
	// どのアニメーションを再生するかなどを制御 ( ハンドルのようなもの )
	int32_t anim_idx_idle_ = 0;
	int32_t anim_idx_walk_ = 0;
	int32_t anim_idx_run_ = 0;
	int32_t anim_idx_attack_ = 0;
	int32_t anim_idx_jump_ = 0;

	// 移動制御用変数
	float move_vel_walk_x_ = 2.0f;
	float move_vel_run_x_ = 5.0f;
	float move_vel_jump_x_ = 3.0f;
	float move_vel_jump_y_ = -20.0f;
	float move_vel_x_ = 0;
	float move_vel_y_ = 0;

	// 左右の描画反転制御用
	ePosture posture_ = ePosture::Right;

	// 待機シーケンス
	void seqIdle(void);
	// 歩きシーケンス
	void seqWalk(void);
	// 走りシーケンス
	void seqRun(void);
	// ジャンプシーケンス
	void seqJump(void);
	// 攻撃シーケンス
	void seqAttack(void);
};


//-----------------------------------------------------------------------------------------------------------------------------
// コンストラクタ
// アニメーションスプライトの作成
Character::Character() {

	// 行動制御用ステートマシンを作成
	// ※ 使い方は example -> util -> tnl_state_machine.h を参照
	state_flow_ = TNL_STATE_MACHINE_CREATE(&Character::seqIdle, this);

	// アニメーションを利用するスプライトの生成に必要なリソースの作成
	// ※ リソースを共有して別のスプライトを作成することも可能です
	Shared<dxe::SpriteResouce> resource = dxe::SpriteResouce::CreateAnimation();

	// 各アニメーション情報を追加してアクセス用ハンドルを作成
	// arg1... スプライトシートのファイルパス
	// arg2... 総コマ数
	// arg3... 横方向のコマ数
	// arg4... 縦方向のコマ数
	// arg5... １コマの横サイズ
	// arg6... １コマの縦サイズ
	// arg7... １回あたりの再生時間
	// arg8... 再生モード ( tnl::SeekUnit::ePlayMode )
	anim_idx_idle_ = resource->addAnimation(FILE_PATH_PNG_C2_ANIM_IDLE, 4, 4, 1, 64, 64, 0.75f, tnl::SeekUnit::ePlayMode::REFLECTION);
	anim_idx_walk_ = resource->addAnimation(FILE_PATH_PNG_C2_ANIM_WALK, 11, 8, 2, 64, 64, 0.75f, tnl::SeekUnit::ePlayMode::REPEAT);
	anim_idx_run_ = resource->addAnimation(FILE_PATH_PNG_C2_ANIM_RUN, 20, 8, 3, 64, 64, 0.75f, tnl::SeekUnit::ePlayMode::REPEAT);
	anim_idx_jump_ = resource->addAnimation(FILE_PATH_PNG_C2_ANIM_JUMP, 3, 3, 1, 64, 64, 0.6f, tnl::SeekUnit::ePlayMode::SINGLE);
	anim_idx_attack_ = resource->addAnimation(FILE_PATH_PNG_SAMP_ATTACK, 5, 5, 1, 50, 50, 0.3f, tnl::SeekUnit::ePlayMode::SINGLE);

	// スプライトの作成
	// arg1... 作成リソース
	sprite_ = dxe::Sprite::CreateAnimation(resource);
	sprite_->setPosition({ 0, 0 });
	sprite_->setScaleXY({ 2, 2 });
	sprite_->setDebugLine(true, dxe::Colors::MediumVioletRed);

	// 足元を座標の基点として設定したいのでロケーション設定を「中央下部」に
	sprite_->setLocation(dxe::eRectOrigin::CENTER_BOTTOM);

	// 入力制御オブジェクトの作成
	input_ = dxe::Input::Create(0);
}


//-----------------------------------------------------------------------------------------------------------------------------
// 更新処理
// シーケンス機能のアップデート処理
// 各行動の結果で得られた移動量で座標更新
void Character::update() {

	// シーケンス機能の更新処理
	state_flow_->update(dxe::GetDeltaTime());

	// move_vel_x_ の値が 0 の近似でなければ
	// 横方向への移動量で左右の姿勢を更新
	if (!tnl::IsAlmostEqualRelative(move_vel_x_, 0)) {
		posture_ = (move_vel_x_ >= 0) ? ePosture::Right : ePosture::Left;
	}
	// 左右の姿勢変数でスプライトの描画反転の設定を行う
	if (ePosture::Right == posture_) {
		if (sprite_->isImageReverseX()) sprite_->setImageReverse(dxe::Sprite::fImageReverse::None);
	}
	if (ePosture::Left == posture_) {
		if (!sprite_->isImageReverseX()) sprite_->setImageReverse(dxe::Sprite::fImageReverse::X);
	}
	// スプライトのアニメーション更新処理
	sprite_->updateAnimation();

	// キャラクタとしての座標更新
	// ※ スプライトの座標更新ではない事に注意
	//    スプライトの座標を描画時にキャラクタ座標を元に更新
	position_.x += move_vel_x_;
	position_.y += move_vel_y_;
}

//-----------------------------------------------------------------------------------------------------------------------------
// 描画
void Character::draw(const Shared<dxe::Camera>& camera) {
	// スプライトの座標更新
	// キャラクタの座標と同じ座標をセット
	sprite_->setPosition(position_);

	// スプライトの描画
	sprite_->draw(camera);
}

//-----------------------------------------------------------------------------------------------------------------------------
// 待機シーケンス
void Character::seqIdle() {

	// シーケンス初期化処理
	if (state_flow_->isStart()) {
		// 該当アニメーション設定
		sprite_->changeAnimation(anim_idx_idle_);
	}

	// X 方向への移動量を 0 でリセット
	move_vel_x_ = 0;

	// 横移動に対応したキーの入力チェック
	if (tnl::Input::IsKeyDown(eKeys::KB_A, eKeys::KB_D)) {
		// 加えて「走る」キーが押されていれば「走る」処理へ
		if (tnl::Input::IsKeyDown(eKeys::KB_RCONTROL)) {
			state_flow_->change(TNL_STATE_BIND(&Character::seqRun, this));
		}
		// 移動キーのみの入力なら「歩く」へ
		else {
			state_flow_->change(TNL_STATE_BIND(&Character::seqWalk, this));
		}
	}

	//// 攻撃キーが押されていれば「攻撃」へ
	if (tnl::Input::IsKeyDownTrigger(eKeys::KB_RETURN)) {
		state_flow_->change(TNL_STATE_BIND(&Character::seqAttack, this));
	}

	// ジャンプキーが押されていれば優先してジャンプへ
	if (tnl::Input::IsKeyDownTrigger(eKeys::KB_W)) {
		state_flow_->change(TNL_STATE_BIND(&Character::seqJump, this));
	}

	DrawFormatString(0, 0, -1, "sequence Idle");
	DrawFormatString(0, 20, -1, "animation frame : %d", sprite_->getAnimPlayingFrameIndex());

	return;
}

//-----------------------------------------------------------------------------------------------------------------------------
// 歩くシーケンス
void Character::seqWalk() {

	// シーケンス初期化処理
	if (state_flow_->isStart()) {
		// 該当アニメーション設定
		sprite_->changeAnimation(anim_idx_walk_);
	}

	// 入力状態で左右の移動量を設定
	if (tnl::Input::IsKeyDown(eKeys::KB_A)) {
		move_vel_x_ = -move_vel_walk_x_;
	}
	else if (tnl::Input::IsKeyDown(eKeys::KB_D)) {
		move_vel_x_ = move_vel_walk_x_;
	}
	// 入力がなければ「待機」へ
	else {
		state_flow_->change(TNL_STATE_BIND(&Character::seqIdle, this));
		return;
	}

	// 歩きの最中にも「走る」へ切り替え
	if (tnl::Input::IsKeyDown(eKeys::KB_RCONTROL)) {
		state_flow_->change(TNL_STATE_BIND(&Character::seqRun, this));
	}

	// 攻撃キーが押されていれば「攻撃」へ
	if (tnl::Input::IsKeyDownTrigger(eKeys::KB_RETURN)) {
		state_flow_->change(TNL_STATE_BIND(&Character::seqAttack, this));
	}

	// 優先的にジャンプ処理
	if (tnl::Input::IsKeyDownTrigger(eKeys::KB_W)) {
		state_flow_->change(TNL_STATE_BIND(&Character::seqJump, this));
	}

	DrawFormatString(0, 0, -1, "sequence Walk");
	DrawFormatString(0, 20, -1, "animation frame : %d", sprite_->getAnimPlayingFrameIndex());

	return;
}

//-----------------------------------------------------------------------------------------------------------------------------
// 走るシーケンス
void Character::seqRun() {

	// シーケンス初期化処理
	if (state_flow_->isStart()) {
		// 該当アニメーション設定
		sprite_->changeAnimation(anim_idx_run_);
	}
	// 「走る」入力がなければ「待機」へ
	if (!tnl::Input::IsKeyDown(eKeys::KB_RCONTROL)) {
		state_flow_->change(TNL_STATE_BIND(&Character::seqIdle, this));
		return;
	}

	// 移動処理
	if (tnl::Input::IsKeyDown(eKeys::KB_A)) {
		move_vel_x_ = -move_vel_run_x_;
	}
	else if (tnl::Input::IsKeyDown(eKeys::KB_D)) {
		move_vel_x_ = move_vel_run_x_;
	}
	// 移動処理がなければ「待機」へ
	else {
		state_flow_->change(TNL_STATE_BIND(&Character::seqIdle, this));
		return;
	}

	// 攻撃キーが押されていれば「攻撃」へ
	if (tnl::Input::IsKeyDownTrigger(eKeys::KB_RETURN)) {
		state_flow_->change(TNL_STATE_BIND(&Character::seqAttack, this));
	}

	// 優先的にジャンプ処理
	if (tnl::Input::IsKeyDownTrigger(eKeys::KB_W)) {
		state_flow_->change(TNL_STATE_BIND(&Character::seqJump, this));
	}

	DrawFormatString(0, 0, -1, "sequence Run");
	DrawFormatString(0, 20, -1, "animation frame : %d", sprite_->getAnimPlayingFrameIndex());

	return;
}

//-----------------------------------------------------------------------------------------------------------------------------
// ジャンプシーケンス
void Character::seqJump() {

	// シーケンス初期化処理
	if (state_flow_->isStart()) {
		// 該当アニメーション設定
		sprite_->changeAnimation(anim_idx_jump_);

		// ジャンプ用 初期 Y軸 移動量設定
		move_vel_y_ = move_vel_jump_y_;
	}

	// ジャンプ中の加速度によってジャンプアニメーションの
	// 再生フレームを任意のフレームで固定する
	// 移動力の 80% 未満と それ以外で「上昇中」「滞空」「下降中」を設定
	if ((fabs(move_vel_y_) / fabs(move_vel_jump_y_)) > 0.2f) {
		if (move_vel_y_ < 0) {
			// 上昇中
			sprite_->jumpAnimSeekFrame(0);
		}
		else {
			// 滞空
			sprite_->jumpAnimSeekFrame(2);
		}
	}
	else {
		// 下降中
		sprite_->jumpAnimSeekFrame(1);
	}

	// ジャンプ中の入力による左右移動への対応

	// 横移動量をリセット
	move_vel_x_ = 0;
	// 左右の入力でジャンプ中の X軸 移動量を設定
	if (tnl::Input::IsKeyDown(eKeys::KB_A)) {
		move_vel_x_ = -move_vel_jump_x_;
	}
	else if (tnl::Input::IsKeyDown(eKeys::KB_D)) {
		move_vel_x_ = move_vel_jump_x_;
	}

	// Y 軸移動量に重力加速度を加算 ( これによりいずれ落下してくる )
	// このサンプルでは現実世界の重力加速度を参考 ( 9.8 )
	move_vel_y_ += 0.98f;

	// 地面( 仮に Y 軸 0 ) に着地したら「待機」へ
	if (position_.y > 0) {
		position_.y = 0;	// 座標補正
		move_vel_y_ = 0;	// Y軸 移動量リセット
		state_flow_->change(TNL_STATE_BIND(&Character::seqIdle, this));
	}

	DrawFormatString(0, 0, -1, "sequence Jump");
	DrawFormatString(0, 20, -1, "animation frame : %d", sprite_->getAnimPlayingFrameIndex());

	return;
}

//-----------------------------------------------------------------------------------------------------------------------------
// 攻撃シーケンス
void Character::seqAttack() {

	// シーケンス初期化処理
	if (state_flow_->isStart()) {
		// 該当アニメーション設定
		sprite_->changeAnimation(anim_idx_attack_);
		// 移動量をリセット
		move_vel_x_ = 0;
	}

	// アニメーションの再生が終了したら「待機」へ
	if (sprite_->getAnimCondition() == tnl::SeekUnit::eCondition::ENDED) {
		state_flow_->change(TNL_STATE_BIND(&Character::seqIdle, this));
	}

	DrawFormatString(0, 0, -1, "sequence Attack");
	DrawFormatString(0, 20, -1, "animation frame : %d", sprite_->getAnimPlayingFrameIndex());

	return;
}


// 2D カメラ
Shared<dxe::Camera> camera;

// 簡易キャラクタユニット
Shared<Character> unit;

//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {
	/* DxLib_Init() はシステム側で行われているので必要ありません */
	srand(time(0));

	// 2D カメラの作成
	// arg1... スクリーンの幅
	// arg2... スクリーンの高さ
	// arg3... カメラタイプを 2D に設定
	camera = std::make_shared<dxe::Camera>(DXE_WINDOW_WIDTH_F, DXE_WINDOW_HEIGHT_F, dxe::Camera::eDimension::Type2D);

	// ユニット作成
	unit = std::make_shared<Character>();

}


//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {

	// カメラの追従移動 ( 線形補間 )
	tnl::Vector2f lp = tnl::Vector2f::UniformLerp(camera->getPosition2D(), unit->getPosition(), 1.0f, 0.05f);
	camera->setPosition2D(lp);

	// 更新
	unit->update();

	// 描画
	unit->draw(camera);


	DrawFormatString(0, 50, -1, "[ 歩く : A, D ]");
	DrawFormatString(0, 70, -1, "[ 走る : Right Ctrl + A, D ]");
	DrawFormatString(0, 90, -1, "[ ジャンプ : W ]");
	DrawFormatString(0, 110, -1, "[ 攻撃 : Enter ]");

	dxe::DrawGrid(50, 30, dxe::Colors::Gray, camera);
	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });
}


//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {
	/* DxLib_End() はシステム側で終了処理が行われるので必要ありません */
}


#endif


