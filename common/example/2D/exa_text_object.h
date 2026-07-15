#if 0

//---------------------------------------------------------------------------------------------------------------
//
//
// テキストオブジェクト使用法サンプル
// 
//
//---------------------------------------------------------------------------------------------------------------


#include <time.h>
#include <string>
#include <dxe.h>
#include "../ResourceConstantHedder.h"
#include "gm_main.h"


Shared<dxe::FontText> text1;
Shared<dxe::FontText> text2;

//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {
	srand(time(0));

	// フォントデータの追加
	tnl::AddFont(dxe::GetPathFromFontName(FONT_NAME_CORPORATE_LOGO_ROUNDED_VER3));

	// テキストオブジェクト用リソース
	// arg1... フォントサイズ
	// arg2... フォント名 ( 省略は "ＭＳ Ｐ明朝" )
	// arg3... クオリティ設定 ( DX_FONTTYPE_**** )
	// arg4... フォントの太さ ( 省略 は デフォルトの -1 )
	// arg5... 文字枠の太さ ( 省略は枠無し )
	// ret.... dxe::FontText の std::shared_ptr
	Shared<dxe::FontTextResouce> resource = dxe::FontTextResouce::Create(
		30, FONT_NAME_CORPORATE_LOGO_ROUNDED_VER3, DX_FONTTYPE_ANTIALIASING_EDGE_4X4, 2, 1);

	// テキストオブジェクト作成
	text1 = dxe::FontText::Create(resource);
	text1->setString("Hello World");
	text1->setPosition({ DXE_WINDOW_WIDTH_HALF_F, DXE_WINDOW_HEIGHT_HALF_F });
	text1->setEdgeColor(dxe::Colors::AliceBlue);
	text1->setLocation(dxe::eRectOrigin::CENTER);

	// テキストオブジェクト作成( テキスト１からクローンを作成 )
	text2 = text1->createClone(text1);
	text2->setString("設定を引き継いだクローンテキスト");
	text2->setPosition({ DXE_WINDOW_WIDTH_HALF_F, DXE_WINDOW_HEIGHT_HALF_F + DXE_WINDOW_HEIGHT_QUARTER_F });

}


//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {

	text1->draw();
	text2->draw();

	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });
}


//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {

	// 内部で保持している Shared<dxe::FontTextResouce> のデストラクタで DxLib のハンドルが
	// 解放されます。DxLib の終了処理が走る前( gameEnd() )にShared<dxe::FontTextResouce>が
	// 解放されるようにしてください。
	// サンプルでは Shared<dxe::FontText> がグローバル変数なのでここで参照カウンタを０に設定
	text1.reset();
	text2.reset();
}


#endif