#pragma warning(disable:4819)
#include "DxLib.h"
#include "dxe_graphics_layer.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

namespace dxe {

	//-------------------------------------------------------------------------------------------------------------------
	GraphicsLayer::GraphicsLayer(const tnl::Vector2i& size)
		: size_(size)
	{
		reset();
		rebuild();
	}

	//-------------------------------------------------------------------------------------------------------------------
	GraphicsLayer::GraphicsLayer(const std::string& file_path)
	{
		loadStatus(file_path);
	}


	//-------------------------------------------------------------------------------------------------------------------
	GraphicsLayer::~GraphicsLayer() {
		DeleteGraph(scr_col_);
		DeleteGraph(scr_heght_bright_);
		DeleteGraph(scr_down_scale_);
		DeleteGraph(scr_gauss_);
	}

	//-------------------------------------------------------------------------------------------------------------------
	void GraphicsLayer::rebuild() {
		if (scr_col_) DeleteGraph(scr_col_);
		if (scr_heght_bright_) DeleteGraph(scr_heght_bright_);
		if (scr_down_scale_) DeleteGraph(scr_down_scale_);
		if (scr_gauss_) DeleteGraph(scr_gauss_);

		// 通常の描画結果を書き込むスクリーンと、フィルターの処理結果を書き込むスクリーンの作成
		int down_scl_w = size_.x / 8;
		int down_scl_h = size_.y / 8;
		scr_col_ = MakeScreen(size_.x, size_.y, is_enable_alpha_channel_);
		scr_heght_bright_ = MakeScreen(size_.x, size_.y, is_enable_alpha_channel_);
		scr_down_scale_ = MakeScreen(down_scl_w, down_scl_h, is_enable_alpha_channel_);
		scr_gauss_ = MakeScreen(down_scl_w, down_scl_h, is_enable_alpha_channel_);

	}

	//-------------------------------------------------------------------------------------------------------------------
	void GraphicsLayer::reset() {

		//dx_blend_mode_ = DX_BLENDMODE_NOBLEND;

		bloom_threshold_ = 230;		// ブルーム用輝度の閾値
		bloom_ratio_ = 1200;		// ブルーム用のぼかし度合い
		bloom_alpha_ = 1.0f;		// ブルームフィルタ透明度

		blur_ratio_ = 1200;			// ブラー用のぼかし度合い
		blur_alpha_ = 0.0f;		// ブラーフィルタ透明度

		mono_cb_ = 0;			// モノトーン YCb
		mono_cr_ = 0;			// モノトーン YCr

		hsb_hue_ = 0;			// HSB 色相
		hsb_saturation_ = 0;	// HSB 彩度
		hsb_bright_ = 0;		// HSB 輝度

		level_min_ = 0;			// level 補正 変換元の下限値( 0 ～ 255 )
		level_max_ = 255;		// level 補正 変換元の上限値( 0 ～ 255 )
		level_gamma_ = 100;		// level 補正 ガンマ値( 100 で 1.0 を表し、ガンマ補正無し、1 より小さい値は不可 )
		level_after_min_ = 0;	// level 補正 変換後の最低値( 0 ～ 255 )
		level_after_max_ = 255;	// level 補正 変換後の最大値( 0 ～ 255 )

		f_adoption_ = static_cast<fAdoption>(0);
		is_enable_alpha_channel_ = false;

		memset(save_file_name, 0, sizeof(char)*64);
		sprintf_s(save_file_name, "graphics_layer_setting.bin\0");
	}


	//-------------------------------------------------------------------------------------------------------------------
	void GraphicsLayer::setUseAlphaChannel(const bool& value) {
		is_enable_alpha_channel_ = value;
		rebuild();
	}

	//-------------------------------------------------------------------------------------------------------------------
	void GraphicsLayer::write(const std::function<void()>& user_write) {
		SetDrawScreen(scr_col_);
		SetBackgroundColor(0, 0, 0, 0);
		ClearDrawScreen();

		user_write();

		if (static_cast<bool>(f_adoption_ & fAdoption::BLOOM)) {
			// 描画結果から高輝度部分のみを抜き出した画像を得る
			GraphFilterBlt(scr_col_, scr_heght_bright_, DX_GRAPH_FILTER_BRIGHT_CLIP, DX_CMP_LESS, bloom_threshold_, TRUE, GetColor(0, 0, 0), 255);

			// 高輝度部分を８分の１に縮小した画像を得る
			GraphFilterBlt(scr_heght_bright_, scr_down_scale_, DX_GRAPH_FILTER_DOWN_SCALE, 8);

			// ８分の１に縮小した画像をガウスフィルタでぼかす
			GraphFilterBlt(scr_down_scale_, scr_gauss_, DX_GRAPH_FILTER_GAUSS, 16, bloom_ratio_);

			// 描画モードをバイリニアフィルタリングにする( 拡大したときにドットがぼやけるようにする )
			SetDrawMode(DX_DRAWMODE_BILINEAR);

			// 描画ブレンドモードを加算にする
			SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(bloom_alpha_ * 255.0f));

			// 高輝度部分を縮小してぼかした画像を画面いっぱいに２回描画する( ２回描画するのはより明るくみえるようにするため )
			DrawExtendGraph(0, 0, size_.x, size_.y, scr_gauss_, is_enable_alpha_channel_);
			DrawExtendGraph(0, 0, size_.x, size_.y, scr_gauss_, is_enable_alpha_channel_);

			// 描画ブレンドモードをブレンド無しに戻す
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

			// 描画モードを二アレストに戻す
			SetDrawMode(DX_DRAWMODE_NEAREST);
		}


		if (static_cast<bool>(f_adoption_ & fAdoption::BLUR)) {
			// 描画結果を８分の１に縮小した画像を得る
			GraphFilterBlt(scr_col_, scr_down_scale_, DX_GRAPH_FILTER_DOWN_SCALE, 8);

			// ８分の１に縮小した画像をガウスフィルタでぼかす
			GraphFilterBlt(scr_down_scale_, scr_gauss_, DX_GRAPH_FILTER_GAUSS, 16, blur_ratio_);

			// 描画モードをバイリニアフィルタリングにする( 拡大したときにドットがぼやけるようにする )
			SetDrawMode(DX_DRAWMODE_BILINEAR);

			SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(blur_alpha_ * 255.0f));
			DrawExtendGraph(0, 0, size_.x, size_.y, scr_gauss_, is_enable_alpha_channel_);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

			// 描画モードを二アレストに戻す
			SetDrawMode(DX_DRAWMODE_NEAREST);
		}

		// モノトーンフィルタ
		if (static_cast<bool>(f_adoption_ & fAdoption::MONO)) {
			GraphFilter(scr_col_, DX_GRAPH_FILTER_MONO, mono_cb_, mono_cr_);
		}
		// HSBフィルタ
		if (static_cast<bool>(f_adoption_ & fAdoption::HSB)) {
			GraphFilter(scr_col_, DX_GRAPH_FILTER_HSB, 0, hsb_hue_, hsb_saturation_, hsb_bright_);
		}
		// Level補正フィルタ
		if (static_cast<bool>(f_adoption_ & fAdoption::LEVEL)) {
			GraphFilter(scr_col_, DX_GRAPH_FILTER_LEVEL, level_min_, level_max_, level_gamma_, level_after_min_, level_after_max_);
		}

		SetDrawScreen(DX_SCREEN_BACK);
	}

	//-------------------------------------------------------------------------------------------------------------------
	void GraphicsLayer::draw() {
		SetDrawBlendMode(dx_blend_mode_, 255);
		DrawGraph(position_.x, position_.y, scr_col_, TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
	}


	//-----------------------------------------------------------------------------------------------------------------------------------------------
	void GraphicsLayer::saveStatus(const std::string& file_path) {
		FILE* fp = nullptr;
		fopen_s(&fp, file_path.c_str(), "wb");
		if (fp) {
			fwrite(&size_.x, sizeof(int), 1, fp);
			fwrite(&size_.y, sizeof(int), 1, fp);
			fwrite(&dx_blend_mode_, sizeof(int), 1, fp);

			fwrite(&bloom_threshold_, sizeof(int), 1, fp);
			fwrite(&bloom_ratio_, sizeof(int), 1, fp);
			fwrite(&bloom_alpha_, sizeof(float), 1, fp);

			fwrite(&blur_ratio_, sizeof(int), 1, fp);
			fwrite(&blur_alpha_, sizeof(float), 1, fp);

			fwrite(&mono_cb_, sizeof(int), 1, fp);
			fwrite(&mono_cr_, sizeof(int), 1, fp);

			fwrite(&hsb_hue_, sizeof(int), 1, fp);
			fwrite(&hsb_saturation_, sizeof(int), 1, fp);
			fwrite(&hsb_bright_, sizeof(int), 1, fp);

			fwrite(&level_min_, sizeof(int), 1, fp);
			fwrite(&level_max_, sizeof(int), 1, fp);
			fwrite(&level_gamma_, sizeof(int), 1, fp);
			fwrite(&level_after_min_, sizeof(int), 1, fp);
			fwrite(&level_after_max_, sizeof(int), 1, fp);

			fwrite(&f_adoption_, sizeof(int), 1, fp);
			fwrite(&is_enable_alpha_channel_, sizeof(bool), 1, fp);

			fclose(fp);
		}

	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------
	void GraphicsLayer::loadStatus(const std::string& file_path) {
		FILE* fp = nullptr;
		fopen_s(&fp, file_path.c_str(), "rb");
		if (fp) {
			fread(&size_.x, sizeof(int), 1, fp);
			fread(&size_.y, sizeof(int), 1, fp);
			fread(&dx_blend_mode_, sizeof(int), 1, fp);

			fread(&bloom_threshold_, sizeof(int), 1, fp);
			fread(&bloom_ratio_, sizeof(int), 1, fp);
			fread(&bloom_alpha_, sizeof(float), 1, fp);

			fread(&blur_ratio_, sizeof(int), 1, fp);
			fread(&blur_alpha_, sizeof(float), 1, fp);

			fread(&mono_cb_, sizeof(int), 1, fp);
			fread(&mono_cr_, sizeof(int), 1, fp);

			fread(&hsb_hue_, sizeof(int), 1, fp);
			fread(&hsb_saturation_, sizeof(int), 1, fp);
			fread(&hsb_bright_, sizeof(int), 1, fp);

			fread(&level_min_, sizeof(int), 1, fp);
			fread(&level_max_, sizeof(int), 1, fp);
			fread(&level_gamma_, sizeof(int), 1, fp);
			fread(&level_after_min_, sizeof(int), 1, fp);
			fread(&level_after_max_, sizeof(int), 1, fp);

			fread(&f_adoption_, sizeof(int), 1, fp);
			fread(&is_enable_alpha_channel_, sizeof(bool), 1, fp);

			fclose(fp);

			rebuild();
		}
	}


	//-------------------------------------------------------------------------------------------------------------------
	void GraphicsLayer::drawImGuiController(const tnl::Vector2i& pos, bool is_window_opened, const char* window_label) {

		std::string label = "GraphicsLayer Setting "; 
		if(window_label) label += "( " + std::string(window_label) + " )";

		bool adoption = false ;

		//  新しいウィンドウの作成
		ImGui::StyleColorsClassic();
		ImGui::SetNextWindowCollapsed(is_window_opened, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImVec2((float)pos.x, (float)pos.y), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(400, 600));
		ImGui::Begin(label.c_str(), nullptr, ImGuiWindowFlags_NoResize);
		{
			if (ImGui::Button("save to file"))
			{
				saveStatus(std::string(save_file_name));
			}
			ImGui::InputText("save file name", save_file_name, 64);

			ImGui::Checkbox("use alpha channel", &is_enable_alpha_channel_);

			adoption = static_cast<bool>( getAdoption() & fAdoption::BLOOM ) ;
			ImGui::Checkbox("use bloom", &adoption);
			setAdoption(adoption ? ( getAdoption() | fAdoption::BLOOM ) : ( getAdoption() & ~fAdoption::BLOOM ));

			adoption = static_cast<bool>(getAdoption() & fAdoption::BLUR);
			ImGui::Checkbox("use blur", &adoption);
			setAdoption(adoption ? (getAdoption() | fAdoption::BLUR) : (getAdoption() & ~fAdoption::BLUR));

			adoption = static_cast<bool>(getAdoption() & fAdoption::MONO);
			ImGui::Checkbox("use mono", &adoption);
			setAdoption(adoption ? (getAdoption() | fAdoption::MONO) : (getAdoption() & ~fAdoption::MONO));

			adoption = static_cast<bool>(getAdoption() & fAdoption::HSB);
			ImGui::Checkbox("use hsb", &adoption);
			setAdoption(adoption ? (getAdoption() | fAdoption::HSB) : (getAdoption() & ~fAdoption::HSB));

			adoption = static_cast<bool>(getAdoption() & fAdoption::LEVEL);
			ImGui::Checkbox("use level", &adoption);
			setAdoption(adoption ? (getAdoption() | fAdoption::LEVEL) : (getAdoption() & ~fAdoption::LEVEL));

			ImGui::SliderInt("bloom threshold", &bloom_threshold_, 0, 255);
			ImGui::SliderFloat("bloom alpha", &bloom_alpha_, 0.0f, 1.0f);
			ImGui::SliderInt("bloom ratio", &bloom_ratio_, 0, 2000);
			ImGui::SliderFloat("blur alpha", &blur_alpha_, 0.0f, 1.0f);
			ImGui::SliderInt("blur ratio", &blur_ratio_, 0, 2000);
			ImGui::SliderInt("mono cb", &mono_cb_, -255, 255);
			ImGui::SliderInt("mono cr", &mono_cr_, -255, 255);
			ImGui::SliderInt("hsb hue", &hsb_hue_, -255, 255);
			ImGui::SliderInt("hsb saluration", &hsb_saturation_, -255, 255);
			ImGui::SliderInt("hsb bright", &hsb_bright_, -255, 255);
			ImGui::SliderInt("level min", &level_min_, 0, 255);
			ImGui::SliderInt("level max", &level_max_, 0, 255);
			ImGui::SliderInt("level gamma", &level_gamma_, 1, 255);
			ImGui::SliderInt("level after min", &level_after_min_, 0, 255);
			ImGui::SliderInt("level after max", &level_after_max_, 0, 255);

			if (ImGui::Button("reset"))
			{
				reset() ;
			}
		}
		ImGui::End();

	}

	//-------------------------------------------------------------------------------------------------------------------
	void GraphicsLayer::drawImGuiControllerParameters() {

		bool adoption = false;

		ImGui::Checkbox("use alpha channel", &is_enable_alpha_channel_);

		adoption = static_cast<bool>(getAdoption() & fAdoption::BLOOM);
		ImGui::Checkbox("use bloom", &adoption);
		setAdoption(adoption ? (getAdoption() | fAdoption::BLOOM) : (getAdoption() & ~fAdoption::BLOOM));

		adoption = static_cast<bool>(getAdoption() & fAdoption::BLUR);
		ImGui::Checkbox("use blur", &adoption);
		setAdoption(adoption ? (getAdoption() | fAdoption::BLUR) : (getAdoption() & ~fAdoption::BLUR));

		adoption = static_cast<bool>(getAdoption() & fAdoption::MONO);
		ImGui::Checkbox("use mono", &adoption);
		setAdoption(adoption ? (getAdoption() | fAdoption::MONO) : (getAdoption() & ~fAdoption::MONO));

		adoption = static_cast<bool>(getAdoption() & fAdoption::HSB);
		ImGui::Checkbox("use hsb", &adoption);
		setAdoption(adoption ? (getAdoption() | fAdoption::HSB) : (getAdoption() & ~fAdoption::HSB));

		adoption = static_cast<bool>(getAdoption() & fAdoption::LEVEL);
		ImGui::Checkbox("use level", &adoption);
		setAdoption(adoption ? (getAdoption() | fAdoption::LEVEL) : (getAdoption() & ~fAdoption::LEVEL));

		ImGui::SliderInt("bloom threshold", &bloom_threshold_, 0, 255);
		ImGui::SliderFloat("bloom alpha", &bloom_alpha_, 0.0f, 1.0f);
		ImGui::SliderInt("bloom ratio", &bloom_ratio_, 0, 2000);
		ImGui::SliderFloat("blur alpha", &blur_alpha_, 0.0f, 1.0f);
		ImGui::SliderInt("blur ratio", &blur_ratio_, 0, 2000);
		ImGui::SliderInt("mono cb", &mono_cb_, -255, 255);
		ImGui::SliderInt("mono cr", &mono_cr_, -255, 255);
		ImGui::SliderInt("hsb hue", &hsb_hue_, -255, 255);
		ImGui::SliderInt("hsb saluration", &hsb_saturation_, -255, 255);
		ImGui::SliderInt("hsb bright", &hsb_bright_, -255, 255);
		ImGui::SliderInt("level min", &level_min_, 0, 255);
		ImGui::SliderInt("level max", &level_max_, 0, 255);
		ImGui::SliderInt("level gamma", &level_gamma_, 1, 255);
		ImGui::SliderInt("level after min", &level_after_min_, 0, 255);
		ImGui::SliderInt("level after max", &level_after_max_, 0, 255);

		if (ImGui::Button("reset"))
		{
			reset();
		}

	}


}
