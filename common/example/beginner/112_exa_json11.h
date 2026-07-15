#if 0

//---------------------------------------------------------------------------------------------------------------
//
//
// Json11 使用法サンプル
// 
//
//---------------------------------------------------------------------------------------------------------------
#include <time.h>
#include <string>
#include <dxe.h>
#include "gm_main.h"

json11::Json json;

//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {
	srand(time(0));

	std::string json_str;

	// リソースから読み込み
	FILE* fp = nullptr;
	fopen_s(&fp, "resource/json/example.json", "r");
	if (fp) {
		char* buff = nullptr;
		size_t size = tnl::GetFileSize("resource/json/example.json");
		buff = new char[size + 1];
		memset(buff, 0, sizeof(char) * (size + 1));
		fread(buff, size, 1, fp);

		json_str = std::string(buff);

		delete[] buff;
		fclose(fp);
	}

	std::string err;
	json = json11::Json::parse(json_str, err);
}


//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {

	DrawFormatString(10, 10, -1, "%s", json["k1"].string_value().c_str());
	DrawFormatString(10, 30, -1, "%d", json["k2"].int_value());
	DrawFormatString(10, 50, -1, "%s", json["k3"].array_items()[0].string_value().c_str());
	DrawFormatString(10, 70, -1, "%d", json["k3"].array_items()[1].int_value());
	if (json["k3"].array_items()[2].bool_value()) {
		DrawFormatString(10, 90, -1, "true");
	}
	if (!json["k3"].array_items()[3].bool_value()) {
		DrawFormatString(10, 110, -1, "false");
	}
	if (json["k3"].array_items()[4].is_null()) {
		DrawFormatString(10, 130, -1, "null");
	}

	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });
}


//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {
}


#endif