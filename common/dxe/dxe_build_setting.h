#pragma once

/* マルチスレッドを利用するプロジェクトでは tnl_build_setting.h の TNL_BUILD_SWITCH_USE_MULTI_THREDING を true に設定してください */
#define DXE_BUILD_SWITCH_USE_MULTI_THREDING		TNL_BUILD_SWITCH_USE_MULTI_THREDING


// ウィンドウの幅
const int DXE_WINDOW_WIDTH = 1280;
const int DXE_WINDOW_WIDTH_HALF = DXE_WINDOW_WIDTH >> 1;
const int DXE_WINDOW_WIDTH_QUARTER = DXE_WINDOW_WIDTH_HALF >> 1;

const float DXE_WINDOW_WIDTH_F = (float)DXE_WINDOW_WIDTH ;
const float DXE_WINDOW_WIDTH_HALF_F = DXE_WINDOW_WIDTH_F * 0.5f ;
const float DXE_WINDOW_WIDTH_QUARTER_F = DXE_WINDOW_WIDTH_HALF_F * 0.5f;


// ウィンドウの高さ
const int DXE_WINDOW_HEIGHT = 720;
const int DXE_WINDOW_HEIGHT_HALF = DXE_WINDOW_HEIGHT >> 1;
const int DXE_WINDOW_HEIGHT_QUARTER = DXE_WINDOW_HEIGHT_HALF >> 1;

const float DXE_WINDOW_HEIGHT_F = (float)DXE_WINDOW_HEIGHT;
const float DXE_WINDOW_HEIGHT_HALF_F = DXE_WINDOW_HEIGHT_F * 0.5f;
const float DXE_WINDOW_HEIGHT_QUARTER_F = DXE_WINDOW_HEIGHT_HALF_F * 0.5f;

// 固定フレームレート
const float DXE_FIX_FPS = 60.0f;

// ウインドウが非アクティブでもアプリケーションを動かすか
const bool DXE_ALLWAYS_RUN_FLAG = true ;

// アプリケーションを開始するフレーム数
const int DXE_APPLICATION_START_FRAME = 1;
