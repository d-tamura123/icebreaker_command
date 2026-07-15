#pragma once
#pragma warning(disable:4819)

//---------------------------------------------------------------------------------------------------------------------------------
// 
// dxlib extension 
// 
//---------------------------------------------------------------------------------------------------------------------------------

#include "../tnl/tnl_util.h"
#include "../tnl/tnl_csv.h"
#include "../tnl/tnl_input.h"
#include "../tnl/tnl_intersect.h"
#include "../tnl/tnl_math.h"
#include "../tnl/tnl_matrix.h"
#include "../tnl/tnl_quaternion.h"
#include "../tnl/tnl_state_machine.h"
#include "../tnl/tnl_coroutine.h"
#include "../tnl/tnl_task_scheduler.h"
#include "../tnl/tnl_shared_factory.h"
#include "../tnl/tnl_vector.h"
#include "../tnl/tnl_vector2i.h"
#include "../tnl/tnl_vector2f.h"
#include "../tnl/tnl_vector3.h"
#include "../tnl/tnl_seek_unit.h"
#include "../tnl/tnl_rect.h"
#include "../tnl/tnl_interactor.h"
#include "../tnl/stb_image.h"
#include "../tnl/json11.hpp"
#include "DxLib.h"
#include "dxe_build_setting.h"
#include "dxe_data.h"
#include "dxe_camera.h"
#include "dxe_prototype.h"
#include "dxe_texture.h"
#include "dxe_graphics_layer.h"
#include "dxe_font_text.h"
#include "dxe_input.h"
#include "dxe_sprite.h"
#include "mesh/dxe_mesh.h"
#include "mesh/dxe_inst_mesh.h"
#include "gui/dxe_gui_value_slider.h"
#include "gui/dxe_gui_menu_selector.h"
#include "gui/dxe_gui_drag_box.h"
#include "gui/dxe_gui_input_box.h"
#include "gui/dxe_gui_check_box.h"
#include "effects/dxe_shadow_map.h"
#include "effects/dxe_particle.h"
#include "effects/dxe_water_plane.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

namespace dxe {

	// dxe の初期化
	DXE_PROTOTYPE_INITIALIZE;

	// dxe の終了処理
	DXE_PROTOTYPE_RELEASE;

	// dxe の更新
	DXE_PROTOTYPE_UPDATE;

	// アプリケーションの終了
	DXE_PROTOTYPE_EXIT_APPLICATION;

	// ウインドウモードの設定( false でフルスクリーン )
	DXE_PROTOTYPE_SET_WINDOW_MODE;

	// 1フレームの経過秒数を取得
	DXE_PROTOTYPE_GET_DELTA_TIME;

	// マウスポインタの表示設定
	DXE_PROTOTYPE_SET_VISIBLE_MOUSE_POINTER;

	// 文字描画
	// arg1... 描画座標
	// arg2... 16進数カラー
	// arg3... 文字列全体のどこを基点として扱うか
	// arg4... 描画文字列 ( printfと同様 書式指定子可 )
	// arg5... 可変引数
	DXE_PROTOTYPE_DRAW_STRING_F32;

	// フォントハンドル指定文字描画
	// arg1... 描画座標
	// arg2... 16進数カラー
	// arg3... フォントハンドル
	// arg4... 文字列全体のどこを基点として扱うか
	// arg5... 描画文字列 ( printfと同様 書式指定子可 )
	// arg6... 可変引数
	DXE_PROTOTYPE_DRAW_STRING_TO_HANDLE;

	// 座標指定なしの文字描画
	// arg1... 16進数カラー
	// arg2... 描画文字列 ( printfと同様 書式指定子可 )
	DXE_PROTOTYPE_DRAW_STACK_STRING;

	// ライン描画
	// arg1... 始点
	// arg2... 方向ベクトル
	// arg3... 長さ
	// arg4... 16進数カラー
	DXE_PROTOTYPE_DRAW_LINE;

	// 点線の色を設定
	// arg1... 16進数カラー( dxe::Color )
	DXE_PROTOTYPE_SET_DOTTED_COLOR;

	// 点線ライン描画
	// arg1... 始点
	// arg2... 終点
	// arg3... 強調演出の有効化
	DXE_PROTOTYPE_DRAW_DOTTED_LINE1;

	// 点線ライン描画
	// arg1... 始点
	// arg2... 方向ベクトル
	// arg3... 長さ
	// arg4... 強調演出の有効化
	DXE_PROTOTYPE_DRAW_DOTTED_LINE2;

	// 矢印描画
	// arg1... 始点
	// arg2... 長さ
	// arg3... 方向ベクトル
	// arg4... 16進数カラー
	// arg5... 傘部分の長さ ( 省略可 )
	DXE_PROTOTYPE_DRAW_ARROW;

	// 矢印描画
	// arg1... 始点
	// arg2... 終点
	// arg4... 16進数カラー
	// arg5... 傘部分の長さ ( 省略可 )
	DXE_PROTOTYPE_DRAW_ARROW2;

	// 矩形描画
	// arg1... 矩形中心座標
	// arg2... 幅
	// arg3... 高さ
	// arg4... 塗りつぶすか
	// arg5... 16進数カラー
	DXE_PROTOTYPE_DRAW_BOX;

	// 回転矩形描画
	// arg1... 矩形中心座標
	// arg2... 幅
	// arg3... 高さ
	// arg4... ラジアン
	// arg5... 16進数カラー
	DXE_PROTOTYPE_DRAW_ROTA_BOX;

	// 点線矩形描画
	// arg1... 矩形の左上座標
	// arg2... 矩形の右下座標
	// arg3... 強調演出の有効化
	DXE_PROTOTYPE_DRAW_DOTTED_BOX1;

	// 点線矩形描画
	// arg1... 矩形の中心座標
	// arg2... 矩形の横幅
	// arg3... 矩形の縦幅
	// arg4... 強調演出の有効化
	DXE_PROTOTYPE_DRAW_DOTTED_BOX2;

	// FPS インジケータ描画
	// arg1... 描画座標
	DXE_PROTOTYPE_DRAW_FPS_INDICATOR;

	// 正六角形を描画
	// arg1... 正六角の中心座標
	// arg2... 正六角の幅
	// arg3... 16進数カラー
	DXE_PROTOTYPE_DRAW_HEXAGON;

	// デバッグ用のグリッド描画
	// arg1... グリッドマスのサイズ
	// arg2... マスを並べる個数 ( 横方向 )
	// arg3... マスを並べる個数 ( 縦方向 )
	// arg3... 2D カメラ( 省略可 )
	DXE_PROTOTYPE_DRAW_DOTTED_GRID;

	// デバッグ用のグリッド地面
	// arg1... カメラ
	// arg2... グリッドマスのサイズ
	// arg3... グリッドを並べる個数 ( 偶数個で指定 )
	// arg4... 16進数カラー
	// tips... この関数を使用する前にカメラを生成して update 関数を実行してください
	DXE_PROTOTYPE_DRAW_GRID_GROUND;


	// 正六角形を敷き詰めた地面を描画
	// arg1... カメラ
	// arg2... 正六角形の幅
	// arg3... 縦横に何個並べるか
	// arg4... XYZ 軸を描画するか
	// arg5... 色
	// tips... この関数を使用する前にカメラを生成して update 関数を実行してください
	// tips... 軽い処理ではないのでデバッグ用途にしてください
	DXE_PROTOTYPE_DRAW_HEXAGON_GROUND;

	// デフォルトライトパラメータをリソースから設定
	// arg1... DrawDefaultLightGuiController で出力したバイナリファイルパス
	DXE_PROTOTYPE_SET_DEFAULT_LIGHT_PARAM;

	// デフォルトライトパラメータをコントロールするGUIを描画
	DXE_PROTOTYPE_DRAW_DEFAULT_LIGHT_GUI_CONTROLLER;

	// クライアント領域にドラッグされたファイルパスを取得する
	// ret... ファイルパス
	DXE_PROTOTYPE_GET_DRAG_FILE_PATH_TRIGGER;

	// 読み込みダイアログからファイルパスを読み取る
	// arg1... 読み込みファイルの対象拡張子の指定  使用例 : std::stirng path = GetOpenReadFilePath("*.bmp;*.jpg;*.png") ;
	// ret.... 読み込んだファイルのフルパス
	// tips... ファイルがダイアログから読み込まれるかキャンセルされるまで、この関数をコールした時点でプログラムは一時停止します
	// tips... キャンセルした場合、戻り値は空文字です
	DXE_PROTOTYPE_GET_OPEN_READ_FILE_PATH;

	// ダイアログからファイル保存パスを読み取る
	// arg1... 保存ファイルの対象拡張子の指定  使用例 : std::stirng path = GetOpenSaveFilePath("png") ;
	// arg2... デフォルトのファイル名 (省略可)
	// ret.... 保存ファイルのフルパス
	// tips... ファイルがダイアログから読み込まれるかキャンセルされるまで、この関数をコールした時点でプログラムは一時停止します
	// tips... キャンセルした場合、戻り値は空文字です
	DXE_PROTOTYPE_GET_OPEN_SAVE_FILE_PATH;

	// マウスカーソルをウィンドウに固定させる関数
	DXE_PROTOTYPE_LOCK_CURSOR_TO_WINDOW;

	// 指定したファイル名のパスがリソース定数テーブルに登録されているか
	// arg1... 検索するファイル名
	DXE_PROTOTYPE_IS_ENABLE_FILE_PATH ;

	// 指定したフォント名のパスがリソース定数テーブルに登録されているか
	// arg1... 検索するフォント名
	DXE_PROTOTYPE_IS_ENABLE_FONT_PATH ;

	// ファイル名からパスを取得する
	DXE_PROTOTYPE_GET_PATH_FROM_FILE_NAME;

	// フォント名からパスを取得する
	DXE_PROTOTYPE_GET_FONT_PATH_FROM_NAME;

	// 中心座標とサイズから指定した基準位置の座標を得る
	DXE_PROTOTYPE_COMPUTE_RECT_POSITION_BY_ORIGIN;

	// ImGui の描画( Begin End )を行う前にコールします
	DXE_PROTOTYPE_IMGUI_NEW_FRAME;

	// ImGui の描画( Begin End )を行った後にコールします
	DXE_PROTOTYPE_IMGUI_RENDER;

	// ImGui のスタイルプリセット
	DXE_PROTOTYPE_IMGUI_SET_STYLE_THEMA_MOCHA;
	// ImGui のスタイルプリセット
	DXE_PROTOTYPE_IMGUI_SET_STYLE_THEMA_LAIVY;
	// ImGui のスタイルプリセット
	DXE_PROTOTYPE_IMGUI_SET_STYLE_THEMA_V3;
	// ImGui のスタイルプリセット
	DXE_PROTOTYPE_IMGUI_SET_STYLE_THEMA_SIMV0L;

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------
	DXE_PROTOTYPE_IS_RUN_APPLICATION;
	DXE_PROTOTYPE_GET_RECT_ORIGIN_LOCATION;
	DXE_PROTOTYPE_DRAW_AXIS;
	DXE_PROTOTYPE_DRAW_AABB;
	DXE_PROTOTYPE_DRAW_OBB;
	DXE_PROTOTYPE_DRAW_BD_SPHERE;
	DXE_PROTOTYPE_DRAW_CAPSULE;
	DXE_PROTOTYPE_DRAW_CYLINDER;
	DXE_PROTOTYPE_DRAW_PLANE;
	DXE_PROTOTYPE_DRAW_3D_ARROW;
	DXE_PROTOTYPE_DRAW_3D_LINE;

	DXE_PROTOTYPE_GET_LOOK_AT_CAMERA_FACTOR;
	DXE_PROTOTYPE_GET_FREE_LOOK_CAMERA_FACTOR;
	DXE_PROTOTYPE_GET_TRANSFORM_CAMERA_FACTOR;

	DXE_PROTOTYPE_SET_FILE_PATH_DICTIONARY;
	DXE_PROTOTYPE_SET_FONT_NAME_TO_PATH_DICTIONARY;
	DXE_PROTOTYPE_SET_DRAG_FILE_PATH;
	DXE_PROTOTYPE_SET_DELTA_TIME;
	DXE_PROTOTYPE_SET_UNLIMIT_DELTA_TIME;

	DXE_PROTOTYPE_GET_VISIBLE_MOUSE_POINTER;
	DXE_PROTOTYPE_GET_WINDOW_MODE;
	DXE_PROTOTYPE_IS_EXIT_APPLICATION;
	DXE_PROTOTYPE_DIRECTX_INITIALIZE;
	DXE_PROTOTYPE_DIRECTX_RENDER_BEGIN;
	DXE_PROTOTYPE_DIRECTX_RENDER_END;
	DXE_PROTOTYPE_DIRECTX_RELEASE;

	DXE_PROTOTYPE_GET_DEPTH_STENCEL_STATE;
	DXE_PROTOTYPE_GET_BLEND_STATE;
	DXE_PROTOTYPE_GET_SAMPLER_STATE;
	DXE_PROTOTYPE_GET_RASTERRIZE_STATE;

	DXE_PROTOTYPE_IMGUI_INITIALIZE;
	DXE_PROTOTYPE_IMGUI_RELEASE;

}

/*

class GameCamera : public dxe::Camera {
public :

	GameCamera(){};
	GameCamera(int screen_w, int screen_h) : dxe::Camera(screen_w, screen_h){}

	enum class eCtrlMode {
		LOOK_AT, FREE_LOOK, TRANSFORM
	};
	inline void setControllMode( eCtrlMode mode ) noexcept { ctrl_mode_ = mode; }
	inline eCtrlMode getControllMode()const noexcept { return ctrl_mode_; }

	void update();


	void lookAtToOther();
	void freeLookToOther();
	void transformToOhter();


	eCtrlMode ctrl_mode_ = eCtrlMode::FREE_LOOK;

	// mode freelook param
	float to_target_distance_ = 300 ;
	float axis_x_angle_ = tnl::ToRadian(45);
	float axis_y_angle_ = 0;

	// mode transform param
	tnl::Quaternion rot_;

};


void GameCamera::lookAtToOther() {
	rot_ = tnl::Quaternion::LookAt(pos_, target_, up_);

	tnl::Vector3 invlook = pos_ - target_;
	to_target_distance_ = invlook.length();
	tnl::Vector3 vx = tnl::Vector3::Normalize(invlook.xz() );
	tnl::Vector3 vy = tnl::Vector3::Normalize(invlook);
	axis_y_angle_ = (invlook.x < 0)? vx.angle({ 0, 0, -1 }) : -vx.angle({ 0, 0, -1 });
	axis_x_angle_ = (invlook.y < 0)? -vy.angle(vx) : vy.angle(vx) ;
}

void GameCamera::freeLookToOther() {
	tnl::Quaternion  q = tnl::Quaternion::RotationAxis( {0, 1, 0}, axis_y_angle_);
	tnl::Vector3 vz = tnl::Vector3::TransformCoord({ 0, 0, 1 }, q);
	q *= tnl::Quaternion::RotationAxis(tnl::Vector3::Cross({0, 1, 0}, vz), axis_x_angle_);
	pos_ = target_ + tnl::Vector3::TransformCoord({ 0, 0, -to_target_distance_ }, q);

	rot_ = tnl::Quaternion::LookAt(pos_, target_, up_);
}

void GameCamera::transformToOhter() {

	target_ = pos_ + tnl::Vector3::TransformCoord({ 0, 0, to_target_distance_ }, rot_);

	tnl::Vector3 invlook = pos_ - target_;
	to_target_distance_ = invlook.length();
	tnl::Vector3 vx = tnl::Vector3::Normalize(invlook.xz());
	tnl::Vector3 vy = tnl::Vector3::Normalize(invlook);
	axis_y_angle_ = (invlook.x < 0) ? vx.angle({ 0, 0, -1 }) : -vx.angle({ 0, 0, -1 });
	axis_x_angle_ = (invlook.y < 0) ? -vy.angle(vx) : vy.angle(vx);

}


void GameCamera::update() {
	switch ( ctrl_mode_ ) {
	case eCtrlMode::LOOK_AT :
		lookAtToOther();
		break;
	case eCtrlMode::FREE_LOOK :
		freeLookToOther();
		break;
	case eCtrlMode::TRANSFORM:
		transformToOhter();
		break;
	}
	dxe::Camera::update();
}
*/


