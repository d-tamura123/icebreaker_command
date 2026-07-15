#if 0

//---------------------------------------------------------------------------------------------------------------
//
//
// DxeMapEditorの出力したデータの使用サンプル
// 
// ※ このサンプルはエディタが出力したデータの１部を使用する簡略版です
//
//---------------------------------------------------------------------------------------------------------------


#include <time.h>
#include <string>
#include <dxe.h>
#include "../ResourceConstantHedder.h"
#include "gm_main.h"


// レイヤー情報
int layer_num = 0;
std::vector<std::vector<tnl::CsvCell>> layer_setting;

// チップ情報
// このサンプルでは 32, 96, 128 サイズだけ扱っていますが
// 本来はマップエディタで出力されたサイズカテゴリー数分が必要です
std::vector<std::vector<tnl::CsvCell>> stil_chip_info_32;
std::vector<std::vector<tnl::CsvCell>> stil_chip_info_96;
std::vector<std::vector<tnl::CsvCell>> stil_chip_info_128;
std::vector<std::vector<tnl::CsvCell>> anim_chip_info_32;

// チップの配置データ
// [ レイヤー順 ][ Y方向 ][ X方向 ]
// landscape_layer_***.csv の配列
std::vector<std::vector<std::vector<tnl::CsvCell>>> put_info;


int stil_chip_32_hdls_num = 0;
int stil_chip_96_hdls_num = 0;
int stil_chip_128_hdls_num = 0;
int anim_chip_32_hdls_num = 0;

int* stil_chip_32_hdls = nullptr;
int* stil_chip_96_hdls = nullptr;
int* stil_chip_128_hdls = nullptr;
int* anim_chip_32_hdls = nullptr;


// チップ個別に再生時間や再生モードが存在しますが
// このサンプルでは簡略化のためアニメーションの再生は
// 全て同一の変数で行います
float anim_time_count = 0;
int anim_frame_count = 0;


std::string base_path = "resource/dxe_map_editor_data/sample/map1/";

//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {
	/* DxLib_Init() はシステム側で行われているので必要ありません */
	srand(time(0));

	//--------------------------------------------------------------------------------------------------------------------------
	//
	// レイヤー設定の読み込み
	//

	layer_setting = tnl::LoadCsv(base_path + "map1_layer_settings.csv");
	layer_num = (int)layer_setting.size() - 1;
	for (int i = 0; i < layer_num; ++i) {
		put_info.emplace_back(tnl::LoadCsv(base_path + "map1_landscape_layer_" + std::to_string(i) + ".csv"));
	}



	//--------------------------------------------------------------------------------------------------------------------------
	//
	// 固定チップ情報の読み込み
	//
	stil_chip_info_32 = tnl::LoadCsv(base_path + "map1_stil_chip_info_32.csv");
	stil_chip_info_96 = tnl::LoadCsv(base_path + "map1_stil_chip_info_96.csv");
	stil_chip_info_128 = tnl::LoadCsv(base_path + "map1_stil_chip_info_128.csv");

	stil_chip_32_hdls_num = (int)stil_chip_info_32.size() - 1;
	stil_chip_96_hdls_num = (int)stil_chip_info_96.size() - 1;
	stil_chip_128_hdls_num = (int)stil_chip_info_128.size() - 1;

	stil_chip_32_hdls = new int[stil_chip_32_hdls_num];
	stil_chip_96_hdls = new int[stil_chip_96_hdls_num];
	stil_chip_128_hdls = new int[stil_chip_128_hdls_num];

	LoadDivGraph((base_path + "map1_stil_chips_32.png").c_str(), stil_chip_32_hdls_num, stil_chip_32_hdls_num, 1, 32, 32, stil_chip_32_hdls);
	LoadDivGraph((base_path + "map1_stil_chips_96.png").c_str(), stil_chip_96_hdls_num, stil_chip_96_hdls_num, 1, 96, 96, stil_chip_96_hdls);
	LoadDivGraph((base_path + "map1_stil_chips_128.png").c_str(), stil_chip_128_hdls_num, stil_chip_128_hdls_num, 1, 128, 128, stil_chip_128_hdls);



	//--------------------------------------------------------------------------------------------------------------------------
	//
	// アニメーションチップ情報の読み込み
	//
	anim_chip_info_32 = tnl::LoadCsv(base_path + "map1_anim_chip_info_32.csv");

	int anim_chip_num = (int)anim_chip_info_32.size() - 1;

	for (int i = 0; i < anim_chip_num; ++i) {
		anim_chip_32_hdls_num += anim_chip_info_32[i + 1][4].getInt();
	}

	anim_chip_32_hdls = new int[anim_chip_32_hdls_num];
	LoadDivGraph((base_path + "map1_anim_chips_32.png").c_str(), anim_chip_32_hdls_num, anim_chip_32_hdls_num, 1, 32, 32, anim_chip_32_hdls);


}


//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {

	anim_time_count += delta_time;
	if (anim_time_count > 0.1f) {
		anim_frame_count++;
		anim_time_count = 0;
	}

	for (int i = 0; i < layer_num; ++i) {

		// layer_setting[0] はヘッダ情報のためアクセスする際は layer_setting[ i + 1 ] としています
		int stil_or_anim = layer_setting[i + 1][0].getInt();
		int tile_size = layer_setting[i + 1][2].getInt();
		int tile_num_x = layer_setting[i + 1][3].getInt();
		int tile_num_y = layer_setting[i + 1][4].getInt();

		int chip_handle_num = 0;
		int* chip_handles = nullptr;
		std::vector<std::vector<tnl::CsvCell>>* chip_info = nullptr;

		if (0 == stil_or_anim) {

			switch (tile_size) {
			case 32:
				chip_handle_num = stil_chip_32_hdls_num;
				chip_handles = stil_chip_32_hdls;
				chip_info = &stil_chip_info_32;
				break;
			case 96:
				chip_handle_num = stil_chip_96_hdls_num;
				chip_handles = stil_chip_96_hdls;
				chip_info = &stil_chip_info_96;
				break;
			case 128:
				chip_handle_num = stil_chip_128_hdls_num;
				chip_handles = stil_chip_128_hdls;
				chip_info = &stil_chip_info_128;
				break;
			}

		}
		else {
			switch (tile_size) {
			case 32:
				chip_handle_num = anim_chip_32_hdls_num;
				chip_handles = anim_chip_32_hdls;
				chip_info = &anim_chip_info_32;
				break;
			}
		}

		if (!chip_handles) continue;

		for (int y = 0; y < tile_num_y; ++y) {
			for (int x = 0; x < tile_num_x; ++x) {

				int index = put_info[i][y][x].getInt();
				if (-1 == index) continue;
				if (0 == stil_or_anim) {
					DrawGraph(x * tile_size, y * tile_size, chip_handles[index], true);
				}
				else {

					int anim_start_index = (*chip_info)[index + 1][3].getInt();
					int anim_frame_num = (*chip_info)[index + 1][4].getInt();
					int anim_playing_index = anim_start_index + (anim_frame_count % anim_frame_num);

					DrawGraph(x * tile_size, y * tile_size, chip_handles[anim_playing_index], true);

				}

			}
		}

	}

	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });
}


//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {
	/* DxLib_End() はシステム側で終了処理が行われるので必要ありません */

	TNL_SAFE_DELETE_ARY(stil_chip_32_hdls);
	TNL_SAFE_DELETE_ARY(stil_chip_96_hdls);
	TNL_SAFE_DELETE_ARY(stil_chip_128_hdls);
	TNL_SAFE_DELETE_ARY(anim_chip_32_hdls);

}


#endif