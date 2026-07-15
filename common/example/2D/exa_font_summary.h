#if 0

//---------------------------------------------------------------------------------------------------------------
//
//
// フォント一覧
// 
//
//---------------------------------------------------------------------------------------------------------------
#include <time.h>
#include <string>
#include <dxe.h>
#include "../ResourceConstantHedder.h"
#include "gm_main.h"

int32_t page = 0;
std::string sample_str = "あいう, カキク, 月火水, ABC, abc, 123, !?$#";
std::vector<Shared<dxe::FontText>> sml_texts;
std::vector<Shared<dxe::FontText>> reg_texts;
std::vector<Shared<dxe::FontText>> big_texts;
std::vector<std::string> names;
#define UPCONV( up, low ) for (int i = 0; i < low.size(); i++) { up[i] = (low[i] == '.' || low[i] == '\'' || low[i] == '/' || low[i] == '-' || low[i] == ' ') ? '_' : up[i] = std::toupper(low[i]); }


//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {
	srand(time(0));
	SetFontSize(20);

	for (int i = 0; i < FONT_NAME_TABLE.size(); ++i) {
		// フォントデータの追加

		std::string test = dxe::GetPathFromFontName(FONT_NAME_TABLE[i]);
		tnl::AddFont(dxe::GetPathFromFontName(FONT_NAME_TABLE[i]));

		std::string name = FONT_NAME_TABLE[i];
		std::string up_name = name;
		UPCONV(up_name, name);
		up_name = "FONT_NAME_" + up_name;
		names.emplace_back(up_name);

		Shared<dxe::FontTextResouce> resource1 = dxe::FontTextResouce::Create(
			10, FONT_NAME_TABLE[i], DX_FONTTYPE_ANTIALIASING_EDGE_4X4, -1, 1);

		auto text = dxe::FontText::Create(resource1);
		text->setString(sample_str);
		text->setPosition({ 10, float(100 + (i % 4) * 160) });
		text->setEdgeColor(dxe::Colors::BlueViolet);
		sml_texts.emplace_back(text);

		Shared<dxe::FontTextResouce> resource2 = dxe::FontTextResouce::Create(
			20, FONT_NAME_TABLE[i], DX_FONTTYPE_ANTIALIASING_EDGE_4X4, -1, 1);

		text = dxe::FontText::Create(resource2);
		text->setString(sample_str);
		text->setPosition({ 10, float(115 + (i % 4) * 160) });
		text->setEdgeColor(dxe::Colors::BlueViolet);
		reg_texts.emplace_back(text);

		Shared<dxe::FontTextResouce> resource3 = dxe::FontTextResouce::Create(
			40, FONT_NAME_TABLE[i], DX_FONTTYPE_ANTIALIASING_EDGE_4X4, -1, 1);

		text = dxe::FontText::Create(resource3);
		text->setString(sample_str);
		text->setPosition({ 10, float(140 + (i % 4) * 160) });
		text->setEdgeColor(dxe::Colors::BlueViolet);
		big_texts.emplace_back(text);
	}



}

//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {

	if (tnl::Input::IsKeyDownTrigger(eKeys::KB_RIGHT)) page++;
	if (tnl::Input::IsKeyDownTrigger(eKeys::KB_LEFT)) page--;
	page = std::clamp(page, 0, int32_t((sml_texts.size() / 4)));


	for (int i = (page * 4); i < (page * 4) + 4; ++i) {
		if (i >= sml_texts.size()) break;
		DrawFormatString(sml_texts[i]->getPosition().x, sml_texts[i]->getPosition().y - 20, -1, "[ %s ]", names[i].c_str());
		sml_texts[i]->draw();
	}
	for (int i = (page * 4); i < (page * 4) + 4; ++i) {
		if (i >= sml_texts.size()) break;
		reg_texts[i]->draw();
	}
	for (int i = (page * 4); i < (page * 4) + 4; ++i) {
		if (i >= sml_texts.size()) break;
		big_texts[i]->draw();
	}


	DrawFormatString(0, 0, -1, "page ( <-  -> ) %d / %d", page, int32_t((sml_texts.size() / 4)));


	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });
}


//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {
	sml_texts.clear();
	reg_texts.clear();
	big_texts.clear();
}


#endif