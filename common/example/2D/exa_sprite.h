#if 0

//---------------------------------------------------------------------------------------------------------------
//
// dxe::Sprite サンプル
// 
// 単一画像リソースを用いたスプライト描画の基本例です。
// カメラを利用した描画、原点位置の違いによる見た目の違い、回転・スケーリング・透明度設定など、
// スプライトの基本的な操作を確認できます。
//
//---------------------------------------------------------------------------------------------------------------

#include <time.h>
#include <string>
#include <dxe.h>
#include "../ResourceConstantHedder.h"
#include "gm_main.h"


//------------------------------------------------------------------------------------------------------------
// グローバル変数
//------------------------------------------------------------------------------------------------------------

// 2D カメラ（画面全体の表示制御に使う）
Shared<dxe::Camera> camera;

// 同じリソース画像を用いた 3つのスプライトインスタンス
Shared<dxe::Sprite> spriteA;
Shared<dxe::Sprite> spriteB;
Shared<dxe::Sprite> spriteC;

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

	// スプライトのベースとなる画像リソースを作成（1つの画像から複数スプライト生成可能）
	Shared<dxe::SpriteResouce> resource = dxe::SpriteResouce::Create(FILE_PATH_PNG_CURSOR);

	// スプライトA（デフォルト原点：中心）
	spriteA = dxe::Sprite::Create(resource);
	spriteA->setPosition({ -400, 0 });							// 左側に配置
	spriteA->setDebugLine(true, dxe::Colors::MediumVioletRed);	// 原点と枠の可視化（デバッグ用）

	// スプライトB（原点：中央下部）
	spriteB = dxe::Sprite::Create(resource);
	spriteB->setPosition({ 0, 0 });								// 中央に配置
	spriteB->setLocation(dxe::eRectOrigin::CENTER_BOTTOM);		// 原点をスプライト下部中央に変更
	spriteB->setDebugLine(true, dxe::Colors::MediumVioletRed);

	// スプライトC（原点：左上＋透明度設定）
	spriteC = dxe::Sprite::Create(resource);
	spriteC->setPosition({ 400, 0 });							// 右側に配置
	spriteC->setDebugLine(true, dxe::Colors::MediumVioletRed);
	spriteC->setLocation(dxe::eRectOrigin::LEFT_TOP);			// 原点を左上に
	spriteC->setBlendMode(dxe::Sprite::eBlendMode::Alpha);		// 半透明描画を有効に
	spriteC->setAlpha(0.5f);									// 不透明度50%


}

// 回転角用のカウンタ（sin波を使ったスケールアニメーションに使用）
float rot_count = 0;

//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {

	// 回転角カウント更新（sin波計算用）
	rot_count += tnl::ToRadian(1.0f);

	// 各スプライトの回転とスケーリングを設定（縦方向に周期的に変化）
	const float scale_y = 1.0f + sin(rot_count) * 0.3f;

	spriteA->addRotation(tnl::ToRadian(0.2f));         // 毎フレーム少しずつ回転
	spriteA->setScaleXY({ 1, scale_y });               // Yスケールを周期変化

	spriteB->addRotation(tnl::ToRadian(0.2f));
	spriteB->setScaleXY({ 1, scale_y });

	spriteC->addRotation(tnl::ToRadian(0.2f));
	spriteC->setScaleXY({ 1, scale_y });

	// スプライトの描画（Z順などは指定していないので順番に依存）
	spriteA->draw(camera);
	spriteB->draw(camera);
	spriteC->draw(camera);


	// 補助描画（背景グリッドとFPS表示）
	dxe::DrawGrid(50, 30, dxe::Colors::Gray, camera);
	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });
}


//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {
	/* DxLib_End() はシステム側で終了処理が行われるので必要ありません */
}


#endif


