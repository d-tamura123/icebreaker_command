#if 0

//---------------------------------------------------------------------------------------------------------------
//
//
// dxe::Sprite アニメーション機能サンプル
// 
// 単一のアニメーションを行うシンプルなサンプルです
//
//---------------------------------------------------------------------------------------------------------------

#include <time.h>
#include <string>
#include <dxe.h>
#include "../ResourceConstantHedder.h"
#include "gm_main.h"

// アニメーションを利用するスプライトの生成に必要なリソースの作成
Shared<dxe::SpriteResouce> resource = dxe::SpriteResouce::CreateAnimation();

// スプライト ( アニメーション機能を使用 )
Shared<dxe::Sprite> sprite;
int32_t anim_idx_ = 0;


// 2D カメラ
Shared<dxe::Camera> camera;

//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {
	/* DxLib_Init() はシステム側で行われているので必要ありません */
	srand(time(0));

	// 各アニメーション情報を追加してアクセス用ハンドルを作成
	// arg1... スプライトシートのファイルパス
	// arg2... 総コマ数
	// arg3... 横方向のコマ数
	// arg4... 縦方向のコマ数
	// arg5... １コマの横サイズ
	// arg6... １コマの縦サイズ
	// arg7... １回あたりの再生時間
	// arg8... 再生モード ( tnl::SeekUnit::ePlayMode )
	// ret.... 複数のアニメーションを登録して切り替える時に指定する個別のインデックス
	//         このサンプルではアニメーションは１つだけなので使用していない
	anim_idx_ = resource->addAnimation(FILE_PATH_PNG_FIRE, 8, 8, 1, 128, 128, 1.0f, tnl::SeekUnit::ePlayMode::REPEAT);

	// スプライトの作成
	// arg1... 作成リソース
	sprite = dxe::Sprite::CreateAnimation(resource);
	sprite->setPosition({ 0, 0 });
	sprite->setScaleXY({ 2, 2 });
	sprite->setDebugLine(true, dxe::Colors::MediumVioletRed);
	sprite->setAnimFrameFadeMode(dxe::Sprite::eAnimFrameFadeMode::Cross);
	sprite->setBlendMode(dxe::Sprite::eBlendMode::Add4);
	sprite->restartAnimation();

	// 2D カメラの作成
	// arg1... スクリーンの幅
	// arg2... スクリーンの高さ
	// arg3... カメラタイプを 2D に設定
	camera = std::make_shared<dxe::Camera>(DXE_WINDOW_WIDTH_F, DXE_WINDOW_HEIGHT_F, dxe::Camera::eDimension::Type2D);

}


//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {

	// アニメーション更新処理
	sprite->updateAnimation();

	// 描画
	sprite->draw(camera);

	dxe::DrawGrid(50, 30, dxe::Colors::Gray, camera);
	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });
}


//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {
	/* DxLib_End() はシステム側で終了処理が行われるので必要ありません */
}


#endif


