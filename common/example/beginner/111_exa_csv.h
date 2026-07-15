#if 0

//---------------------------------------------------------------------------------------------------------------
//
//
// CSV 使用法サンプル
// 
//
//---------------------------------------------------------------------------------------------------------------
#include <time.h>
#include <string>
#include <dxe.h>
#include "gm_main.h"

std::vector<std::vector<tnl::CsvCell>> csv;

//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {
	srand(time(0));

	csv = tnl::LoadCsv("resource/csv/example.csv");

}


//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {

	DrawFormatString(10, 10, -1, "%s", csv[0][0].getString().c_str());
	if (csv[0][1].getBool()) {
		DrawFormatString(100, 10, -1, "true");
	}
	else {
		DrawFormatString(100, 10, -1, "false");
	}

	DrawFormatString(10, 30, -1, "%s", csv[1][0].getString().c_str());
	DrawFormatString(100, 30, -1, "%d", csv[1][1].getInt());

	DrawFormatString(10, 60, -1, "%s", csv[2][0].getString().c_str());
	DrawFormatString(100, 60, -1, "%d", csv[2][1].getInt());

	DrawFormatString(10, 90, -1, "%s", csv[3][0].getString().c_str());
	DrawFormatString(100, 90, -1, "%d", csv[3][1].getInt());

	DrawFormatString(10, 120, -1, "%s", csv[4][0].getString().c_str());
	DrawFormatString(100, 120, -1, "%f", csv[4][1].getFloat());

	DrawFormatString(10, 150, -1, "%s", csv[5][0].getString().c_str());
	DrawFormatString(100, 150, -1, "%f", csv[5][1].getFloat());

	DrawFormatString(10, 180, -1, "%s", csv[6][0].getString().c_str());
	DrawFormatString(100, 180, -1, "%f", csv[6][1].getFloat());


	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });
}


//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {
}


#endif