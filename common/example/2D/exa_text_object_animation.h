#if 0

//---------------------------------------------------------------------------------------------------------------
//
//
// テキストオブジェクト アニメーション サンプル
// 
//
//---------------------------------------------------------------------------------------------------------------
#include <time.h>
#include <string>
#include <dxe.h>
#include "../ResourceConstantHedder.h"
#include "gm_main.h"


int se_hdl = 0;
int current_message = 0;
Shared<dxe::FontText> text;

// サンプルテキスト ( 青空文庫より )
std::vector<std::string> test_message = {
	"　吾輩わがはいは猫である。名前はまだ無い。\nどこで生れたかとんと見当けんとうがつかぬ。\n何でも薄暗いじめじめした所でニャーニャー泣いていた事だけは記憶している。",
	"吾輩はここで始めて人間というものを見た。\nしかもあとで聞くとそれは書生という人間中で\n一番獰悪どうあくな種族であったそうだ。\nこの書生というのは時々我々を捕つかまえて煮にて食うという話である。\n",
	"しかしその当時は何という考もなかったから別段恐しいとも思わなかった。\nただ彼の掌てのひらに載せられてスーと持ち上げられた時\n何だかフワフワした感じがあったばかりである。\n掌の上で少し落ちついて書生の顔を見たのが\nいわゆる人間というものの見始みはじめであろう。",
	"この時妙なものだと思った感じが今でも残っている。\n第一毛をもって装飾されべきはずの顔がつるつるしてまるで薬缶やかんだ。\nその後ご猫にもだいぶ逢あったがこんな片輪かたわには\n一度も出会でくわした事がない。",
	"のみならず顔の真中があまりに突起している。\nそうしてその穴の中から時々ぷうぷうと煙けむりを吹く。\nどうも咽むせぽくて実に弱った。\nこれが人間の飲む煙草たばこというものである事は\nようやくこの頃知った。"
};


//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {
	srand(time(0));
	SetFontSize(20);

	// 文字送り SE 
	se_hdl = LoadSoundMem(dxe::GetPathFromFileName(FILE_NAME_MP3_SE_SAMPLE_1));

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

	// アニメーションテキストの作成
	text = dxe::FontText::CreateAnimation(resource);
	text->setString(test_message[0]);

	text->setPosition({ 70, 400 });
	text->setEdgeColor(dxe::Colors::Green);

	// 文字送りされる毎に行うユーザー定義処理
	text->setAnimUpdateCallback([&]() {
		// 文字送り SE 再生
		PlaySoundMem(se_hdl, DX_PLAYTYPE_BACK);
		});

}

//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {


	if (tnl::Input::IsKeyDownTrigger(eKeys::KB_RETURN)) {

		// 待機状態なら再生
		if (text->getAnimCondition() == tnl::SeekUnit::eCondition::STANDBY) {
			text->playAnimation();

			// 再生が完了していたら次のメッセージ
		}
		else if (text->getAnimCondition() == tnl::SeekUnit::eCondition::ENDED) {
			current_message++;
			current_message %= test_message.size();
			text->setString(test_message[current_message]);
			text->restartAnimation();

			// アニメーションを飛ばす
		}
		else {
			text->finishAnimation();
		}

	}


	if (tnl::Input::IsKeyDownTrigger(eKeys::KB_SPACE)) {
		// 一時停止
		text->pauseAnimation();
	}

	// アニメーション更新処理
	text->updateAnimation();

	// 描画
	text->draw();


	dxe::DrawString({ DXE_WINDOW_WIDTH_HALF, DXE_WINDOW_HEIGHT_HALF }, -1, dxe::eRectOrigin::CENTER, "エンターキーで文字再生");

	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });
}


//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {

	// 内部で保持している Shared<dxe::FontTextResouce> のデストラクタで DxLib のハンドルが
	// 解放されます。DxLib の終了処理が走る前( gameEnd() )にShared<dxe::FontTextResouce>が
	// 解放されるようにしてください。
	// サンプルでは Shared<dxe::FontText> がグローバル変数なのでここで参照カウンタを０に設定
	text.reset();

}


#endif