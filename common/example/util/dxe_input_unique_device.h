#if 0

//---------------------------------------------------------------------------------------------------------------
//
//
// 入力オブジェクト　サンプル ( キーボード & マウス )
// 
//
//---------------------------------------------------------------------------------------------------------------


#include <time.h>
#include <string>
#include <dxe.h>
#include "gm_main.h"


Shared<dxe::Input> inputA;
Shared<dxe::Input> inputB;

//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {
	srand(time(0));

	inputA = dxe::Input::Create(0);
	inputB = dxe::Input::Create(1);

}


//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {

	float y = 300;

	//-----------------------------------------------------------------------------------------------------------------------
	// 入力ステージ 0

	if (inputA->keep(dxe::Input::eButton::KB_RETURN)) {
		DrawFormatString(0, y, -1, "inputA : keep");
		y += 20;
	}

	if (inputA->pressed(dxe::Input::eButton::MOUSE_LEFT)) {
		DrawFormatString(0, y, -1, "inputA : pressed");
		y += 20;
	}

	if (inputA->released(dxe::Input::eButton::KB_X)) {
		DrawFormatString(0, y, -1, "inputA : released");
		y += 20;
	}

	if (inputA->barrage(dxe::Input::eButton::KB_C, 0.5f, 0.1f)) {
		DrawFormatString(0, y, -1, "inputA : barrage");
		y += 20;
	}


	dxe::Input::fDualReaction reac = inputA->dual(
		dxe::Input::eButton::KB_V
		, 0.15f
		, dxe::Input::eDualFirstReaction::PRESSED
		, dxe::Input::eDualSecondReaction::PRESSED);
	if (static_cast<bool>((reac & dxe::Input::fDualReaction::FIRST))) {
		DrawFormatString(0, y, -1, "inputA : dual : first");
		y += 20;
	}
	if (static_cast<bool>((reac & dxe::Input::fDualReaction::SECOND))) {
		DrawFormatString(0, y, -1, "inputA : dual : second");
		y += 20;
	}


	//-----------------------------------------------------------------------------------------------------------------------
	// 入力ステージ 1


	if (inputB->keep(dxe::Input::eButton::KB_RETURN)) {
		DrawFormatString(0, y, -1, "inputB : keep");
		y += 20;
	}

	if (inputB->pressed(dxe::Input::eButton::MOUSE_LEFT)) {
		DrawFormatString(0, y, -1, "inputB : pressed");
		y += 20;
	}

	if (inputB->released(dxe::Input::eButton::KB_X)) {
		DrawFormatString(0, y, -1, "inputB : released");
		y += 20;
	}

	if (inputB->barrage(dxe::Input::eButton::KB_C, 0.5f, 0.1f)) {
		DrawFormatString(0, y, -1, "inputB : barrage");
		y += 20;
	}


	reac = inputB->dual(
		dxe::Input::eButton::KB_V
		, 0.15f
		, dxe::Input::eDualFirstReaction::PRESSED
		, dxe::Input::eDualSecondReaction::PRESSED);
	if (static_cast<bool>((reac & dxe::Input::fDualReaction::FIRST))) {
		DrawFormatString(0, y, -1, "inputB : dual : first");
		y += 20;
	}
	if (static_cast<bool>((reac & dxe::Input::fDualReaction::SECOND))) {
		DrawFormatString(0, y, -1, "inputB : dual : second");
		y += 20;
	}


	if (tnl::Input::IsKeyDownTrigger(eKeys::KB_LEFT)) dxe::Input::ChangeEnableStage(0);
	if (tnl::Input::IsKeyDownTrigger(eKeys::KB_RIGHT)) dxe::Input::ChangeEnableStage(1);

	DrawFormatString(0, 0, -1, "input stage [ <- %d -> ]", dxe::Input::GetEnableStage());
	DrawFormatString(0, 20, -1, "keep : Enter");
	DrawFormatString(0, 40, -1, "pressed : Mouse Left");
	DrawFormatString(0, 60, -1, "released : X");
	DrawFormatString(0, 80, -1, "barrage : C");
	DrawFormatString(0, 100, -1, "dual : V");

	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });
}


//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {
}


#endif