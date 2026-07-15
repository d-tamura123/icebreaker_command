#if 0


//---------------------------------------------------------------------------------------------------------------
//
//
// クラス間での疎結合を保ちながら通信を行う インタラクションコーディネーター サンプル
//
// ２つのインスタンス間でメディエータとしてインタラクターを利用した通信の場合
//
//---------------------------------------------------------------------------------------------------------------
#include <time.h>
#include <string>
#include <dxe.h>
#include "gm_main.h"


//---------------------------------------------------------------------------------------
//
// UIButton.h
//
//---------------------------------------------------------------------------------------

class UIButton : public std::enable_shared_from_this<UIButton> {
public:

	// クラス間の疎結合通信をサポートする インタラクター 記述マクロ
	// このマクロを記述することで tnlInteractor で始まる通信関数が自動記述されます
	TNL_INTERACTOR_DECLARE(
		CALL_BALL_MOVE,					// 円への「動け」を通知する ID
		CALL_BALL_STOP,					// 円への「止まれ」を通知する ID
		CALL_BALL_RANDOM_COLOR_CHENGE	// 円へ色替えボタンが押された事を通知する ID
	);

	UIButton
	(const tnl::Vector3& position
		, const std::string& on_text
		, const std::string& off_text
		, const std::function<void(const Shared<UIButton>&)>& on_click_proc)

		: position_(position)
		, on_text_(on_text)
		, off_text_(off_text)
		, on_click_proc_(on_click_proc)
	{}

	bool getSelected() { return is_selected_; }
	void setSelected(bool flag) { is_selected_ = flag; }

	const tnl::Vector3& getPosition() { return position_; }
	void setPosition(const tnl::Vector3& position) { position_ = position; }

	void update();
	void draw();
private:
	bool is_forcus_ = false;
	bool is_click_ = false;
	bool is_selected_ = false;
	tnl::Vector3 size_ = { 100, 30, 0 };
	tnl::Vector3 position_;
	std::function<void(const Shared<UIButton>&)> on_click_proc_;
	std::string on_text_;
	std::string off_text_;
};

//---------------------------------------------------------------------------------------
//
// UIButton.cpp
//
//---------------------------------------------------------------------------------------

void UIButton::update() {
	is_forcus_ = false;
	is_click_ = false;
	tnl::Vector3 ms = tnl::Input::GetMousePosition();
	if (tnl::IsIntersectPointRect((int)ms.x, (int)ms.y, (int)position_.x, (int)position_.y, (int)size_.x, (int)size_.y)) {
		is_forcus_ = true;
		if (tnl::Input::IsMouseDown(tnl::Input::eMouse::LEFT)) is_click_ = true;

		if (tnl::Input::IsMouseTrigger(tnl::Input::eMouseTrigger::OUT_LEFT)) {
			on_click_proc_(shared_from_this());
		}
	}

}

void UIButton::draw() {
	int color = -1;
	if (is_forcus_) color = GetColor(128, 128, 255);
	if (is_click_) color = GetColor(255, 128, 128);

	std::string btn_str = (!is_selected_) ? on_text_ : off_text_;

	dxe::DrawBox({ TNL_DEP_V2(position_) }, size_.x, size_.y, false, color);
	DrawFormatString((int)(position_.x - (size_.x / 4)), (int)position_.y - 10, -1, btn_str.c_str());
}



//---------------------------------------------------------------------------------------
//
// Circle.h
//
//---------------------------------------------------------------------------------------

class Circle : public std::enable_shared_from_this<Circle> {
public:
	Circle(const tnl::Vector3& position) : position_(position) {}

	void update();
	void draw();

	void setMoveFlag(bool flag) { is_move_ = flag; }
	const tnl::Vector3& getPosition() { return position_; }
	void randomColorChange() { color_ = GetColor(rand() % 255, rand() % 255, rand() % 255); }

private:
	bool is_move_ = false;
	float speed_ = 10;
	float radius_ = 100;
	int color_ = -1;
	tnl::Vector3 position_;
};

//---------------------------------------------------------------------------------------
//
// Circle.cpp
//
//---------------------------------------------------------------------------------------

void Circle::update() {
	if (!is_move_) return;
	position_.x += speed_;

	float l_lim_x = 200;
	float r_lim_x = 1000;
	if (position_.x < l_lim_x) {
		position_.x = l_lim_x;
		speed_ = -speed_;
	}
	if (position_.x > r_lim_x) {
		position_.x = r_lim_x;
		speed_ = -speed_;
	}
}

void Circle::draw() {
	DrawCircle((int)position_.x, (int)position_.y, (int)radius_, color_, false);
}




//---------------------------------------------------------------------------------------
//
// game main
//
//---------------------------------------------------------------------------------------


// それぞれのインスタンスは独立したグローバルな変数であり
// 特定のクラスの包含関係にもない状態
Shared<UIButton> button_move;
Shared<UIButton> button_color_change;
Shared<Circle> circle;

//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {

	// move stop ボタンの作成
	button_move = std::make_shared<UIButton>(
		tnl::Vector3(DXE_WINDOW_WIDTH_HALF, DXE_WINDOW_HEIGHT_HALF, 0),
		"move",
		"stop",

		// クリックされた時に行われる処理
		[](const Shared<UIButton>& btn) {
			btn->setSelected(!btn->getSelected());
			if (btn->getSelected()) {

				// 円への「動け」通知
				// この関数による戻り値は無いのでテンプレート型指定は void
				// arg1... TNL_INTERACTOR_DECLARE に設定した 「動け」通知 ID
				// arg2... CALL_BALL_MOVE に対応する通知に付加されるパラメータ ( 今回は bool 型 ※何の型でも対応 )
				// tips... arg2 で渡す型については この後の TNL_INTERACTOR_SEND_TRANSMITTER_REGISTER で設定
				btn->tnlInteractorArgsTransmitter<void>(UIButton::eTnlICId::CALL_BALL_MOVE, true);
			}
			else {

				// 円への「止まれ」通知
				// この関数による戻り値は const tnl::Vector3& なのでテンプレート型指定にそのように記述
				// arg1... TNL_INTERACTOR_DECLARE に設定した 「止まれ」通知 ID
				// arg2... CALL_BALL_MOVE に対応する通知に付加されるパラメータ ( 今回は bool 型 ※何の型でも対応 )
				// ret.... CALL_BALL_STOP を実行した結果の 返信 ( 今回は円の座標を返信情報としている )
				// tips... arg2 で渡す型については この後の TNL_INTERACTOR_SEND_TRANSMITTER_REGISTER で設定
				tnl::Vector3 recipt = btn->tnlInteractorArgsTransmitter<const tnl::Vector3&>(UIButton::eTnlICId::CALL_BALL_STOP, false);

				// 受け取った返信情報( 円の座標 )でボタンの座標を更新
				btn->setPosition(recipt);
			}
		});


	// color ボタンの作成
	button_color_change = std::make_shared<UIButton>(
		tnl::Vector3(DXE_WINDOW_WIDTH - 100, 100, 0),
		"color",
		"color",

		// クリックされた時に行われる処理
		[](const Shared<UIButton>& btn) {

			// 円への「色を変えろ」通知
			// この関数による戻り値は無いのでテンプレート型指定は void
			// arg1... TNL_INTERACTOR_DECLARE に設定した 「色を変えろ」通知 ID
			// tips... この関数での円への通知で付加情報は無いので arg2 は存在しない ( 上記とは関数名が違う事に留意 )
			btn->tnlInteractorTransmitter<>(UIButton::eTnlICId::CALL_BALL_RANDOM_COLOR_CHENGE);
		});



	// 円の作成
	circle = std::make_shared<Circle>(tnl::Vector3(DXE_WINDOW_WIDTH_HALF, DXE_WINDOW_HEIGHT_HALF, 0));


	// move ボタンから 円に対して「動け」通知される内容を登録
	// arg1... move ボタンが通知を出すので ボタンのインスタンス
	// arg2... 通知からの返信は設定しないので void
	// arg3... 通知を識別する ID を設定
	// arg4... 通知先のインスタンスを std::weak_ptr で指定
	// arg5... 円への通知に付加情報を設定する ( ここでは bool 型 )
	// arg6... ラムダ式の {} のみ記述 ( 通知内容の処理 )
	TNL_INTERACTOR_ARGS_TRANSMITTER_REGISTER(button_move, void, UIButton::eTnlICId::CALL_BALL_MOVE, Weak<Circle>(circle), bool, {
		// ボタンから通知されたら 通知先として設定されているインスタンスを取得
		Shared<Circle> cir = TNL_INTERACTOR_GET_WEAK_REFERENCE(Weak<Circle>).lock();

	// ボタンから渡された付加情報を使用して円の情報を更新
	// ここでは TNL_INTERACTOR_GET_RECEIPT_INFORMATION() で得られるのは arg5で指定した型 ( bool )
	cir->setMoveFlag(TNL_INTERACTOR_GET_RECEIPT_INFORMATION());
		});

	// stop ボタンから 円に対して「止まれ」通知される内容を登録
	// arg1... stop ボタンが通知を出すので ボタンのインスタンス
	// arg2... 通知からの返信を受けるための型を設定 ここでは const tnl::Vector3& 
	// arg3... 通知を識別する ID を設定
	// arg4... 通知先のインスタンスを std::weak_ptr で指定
	// arg5... 円への通知に付加情報を設定する ( ここでは bool 型 )
	// arg6... ラムダ式の {} のみ記述 ( 通知内容の処理 )
	TNL_INTERACTOR_UNSAFE_ARGS_TRANSMITTER_REGISTER(button_move, const tnl::Vector3&, UIButton::eTnlICId::CALL_BALL_STOP, Weak<Circle>(circle), bool, {
		Shared<Circle> cir = TNL_INTERACTOR_GET_WEAK_REFERENCE(Weak<Circle>).lock();
		cir->setMoveFlag(TNL_INTERACTOR_GET_RECEIPT_INFORMATION());

		// 返信として const tnl::Vector3& を返す ( 円の座標 )
		return cir->getPosition();
		});


	// color ボタンから 円に対して「色を変えろ」通知される内容を登録
	// arg1... color ボタンが通知を出すので ボタンのインスタンス
	// arg2... 通知からの返信は設定しないので void
	// arg3... 通知を識別する ID を設定
	// arg4... 通知先のインスタンスを std::weak_ptr で指定
	// arg5... ラムダ式の {} のみ記述 ( 通知内容の処理 )
	// tips... 上記２つと違って円への付加情報を設定しないマクロ ( マクロ名の SEND と VOID が違う事に留意 )
	TNL_INTERACTOR_UNSAFE_TRANSMITTER_REGISTER(button_color_change, void, UIButton::eTnlICId::CALL_BALL_RANDOM_COLOR_CHENGE, Weak<Circle>(circle), {
		Shared<Circle> cir = TNL_INTERACTOR_GET_WEAK_REFERENCE(Weak<Circle>).lock();
		cir->randomColorChange();
		});

}


//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {


	button_move->update();
	button_color_change->update();
	circle->update();


	button_move->draw();
	button_color_change->draw();
	circle->draw();

	DrawFormatString(0, 0, -1, "move と color はマウスクリックで反応するボタンです");
	DrawFormatString(0, 20, -1, "stop をクリックすると動いている円が止まり、move ボタンが円の中心に配置されます");

	DrawFormatString(0, 50, -1, "このサンプルはボタンと円の間でメディエータパターンをサポートした TNL_INTERACTOR のサンプルです");
	DrawFormatString(0, 70, -1, "move がクリックされるとボタンの状態を元に 円に対して 「動け( true )」もしくは 「止まれ( false )」の通知を行います");
	DrawFormatString(0, 90, -1, "「止まれ」の通知をしたとき、円に対する通知の返信として円の座標を受け取り、ボタンの座標を更新しています");

	DrawFormatString(0, 120, -1, "color ボタンは円への通知において move stop のように通知情報を付加しません また円からの返信もありません");


	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });

}




//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {

}

#endif
