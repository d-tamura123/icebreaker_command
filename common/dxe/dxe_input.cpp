#include "Dxlib.h"
#include "dxe_input.h"
#include "dxe_prototype.h"

namespace dxe {

	DXE_PROTOTYPE_GET_DELTA_TIME;

	int32_t Input::running_stage_ = 0;
	tnl::Vector2f Input::before_mouse_point_;
	tnl::Vector2f Input::forword_mouse_point_;
	std::stack<int32_t> Input::before_stages_;
	std::list<Weak<Input>> Input::maneged_inputs_ ;

	char Input::btn_keep[static_cast<int32_t>(eButton::UNIQUE_DEVICE_BUTTON_MAX)];
	char Input::btn_xor_down[static_cast<int32_t>(eButton::UNIQUE_DEVICE_BUTTON_MAX)];
	char Input::btn_trg_down[static_cast<int32_t>(eButton::UNIQUE_DEVICE_BUTTON_MAX)];
	char Input::btn_xor_release[static_cast<int32_t>(eButton::UNIQUE_DEVICE_BUTTON_MAX)];
	char Input::btn_trg_release[static_cast<int32_t>(eButton::UNIQUE_DEVICE_BUTTON_MAX)];
	char Input::trg_mask_table_[static_cast<int32_t>(eButton::UNIQUE_DEVICE_BUTTON_MAX)] ;

	std::vector<int32_t> Input::dxlib_button_table_ = {

			//---------------------------------------------------------------------------------------------------------------
			//
			// keybord
			//
			  KEY_INPUT_BACK			// BackSpaceキー	D_DIK_BACK
			 ,KEY_INPUT_TAB				// Tabキー			D_DIK_TAB
			 ,KEY_INPUT_RETURN			// Enterキー		D_DIK_RETURN
			 ,KEY_INPUT_LSHIFT			// 左Shiftキー		D_DIK_LSHIFT
			 ,KEY_INPUT_RSHIFT			// 右Shiftキー		D_DIK_RSHIFT
			 ,KEY_INPUT_LCONTROL		// 左Ctrlキー		D_DIK_LCONTROL
			 ,KEY_INPUT_RCONTROL		// 右Ctrlキー		D_DIK_RCONTROL
			 ,KEY_INPUT_ESCAPE			// Escキー			D_DIK_ESCAPE
			 ,KEY_INPUT_SPACE			// スペースキー		D_DIK_SPACE
			 ,KEY_INPUT_PGUP			// PageUpキー		D_DIK_PGUP
			 ,KEY_INPUT_PGDN			// PageDownキー		D_DIK_PGDN
			 ,KEY_INPUT_END				// Endキー			D_DIK_END
			 ,KEY_INPUT_HOME			// Homeキー			D_DIK_HOME
			 ,KEY_INPUT_LEFT			// 左キー			D_DIK_LEFT
			 ,KEY_INPUT_UP				// 上キー			D_DIK_UP
			 ,KEY_INPUT_RIGHT			// 右キー			D_DIK_RIGHT
			 ,KEY_INPUT_DOWN			// 下キー			D_DIK_DOWN
			 ,KEY_INPUT_INSERT			// Insertキー		D_DIK_INSERT
			 ,KEY_INPUT_DELETE			// Deleteキー		D_DIK_DELETE
			 ,KEY_INPUT_MINUS			// －キー			D_DIK_MINUS
			 ,KEY_INPUT_YEN				// ￥キー			D_DIK_YEN
			 ,KEY_INPUT_PREVTRACK		// ＾キー			D_DIK_PREVTRACK
			 ,KEY_INPUT_PERIOD			// ．キー			D_DIK_PERIOD
			 ,KEY_INPUT_SLASH			// ／キー			D_DIK_SLASH
			 ,KEY_INPUT_LALT			// 左Altキー		D_DIK_LALT
			 ,KEY_INPUT_RALT			// 右Altキー		D_DIK_RALT
			 ,KEY_INPUT_SCROLL			// ScrollLockキー	D_DIK_SCROLL
			 ,KEY_INPUT_SEMICOLON		// ；キー			D_DIK_SEMICOLON
			 ,KEY_INPUT_COLON			// ：キー			D_DIK_COLON
			 ,KEY_INPUT_LBRACKET		// ［キー			D_DIK_LBRACKET
			 ,KEY_INPUT_RBRACKET		// ］キー			D_DIK_RBRACKET
			 ,KEY_INPUT_AT				// ＠キー			D_DIK_AT
			 ,KEY_INPUT_BACKSLASH		// ＼キー			D_DIK_BACKSLASH
			 ,KEY_INPUT_COMMA			// ，キー			D_DIK_COMMA
			 ,KEY_INPUT_KANJI			// 漢字キー			D_DIK_KANJI
			 ,KEY_INPUT_CONVERT			// 変換キー			D_DIK_CONVERT
			 ,KEY_INPUT_NOCONVERT		// 無変換キー		D_DIK_NOCONVERT
			 ,KEY_INPUT_KANA			// カナキー			D_DIK_KANA
			 ,KEY_INPUT_APPS			// アプリケーションメニューキー		D_DIK_APPS
			 ,KEY_INPUT_CAPSLOCK		// CaspLockキー		D_DIK_CAPSLOCK
			 ,KEY_INPUT_SYSRQ			// PrintScreenキー	D_DIK_SYSRQ
			 ,KEY_INPUT_PAUSE			// PauseBreakキー	D_DIK_PAUSE
			 ,KEY_INPUT_LWIN			// 左Winキー		D_DIK_LWIN
			 ,KEY_INPUT_RWIN			// 右Winキー		D_DIK_RWIN
			 ,KEY_INPUT_NUMLOCK			// テンキーNumLockキー		D_DIK_NUMLOCK
			 ,KEY_INPUT_NUMPAD0			// テンキー０				D_DIK_NUMPAD0
			 ,KEY_INPUT_NUMPAD1			// テンキー１				D_DIK_NUMPAD1
			 ,KEY_INPUT_NUMPAD2			// テンキー２				D_DIK_NUMPAD2
			 ,KEY_INPUT_NUMPAD3			// テンキー３				D_DIK_NUMPAD3
			 ,KEY_INPUT_NUMPAD4			// テンキー４				D_DIK_NUMPAD4
			 ,KEY_INPUT_NUMPAD5			// テンキー５				D_DIK_NUMPAD5
			 ,KEY_INPUT_NUMPAD6			// テンキー６				D_DIK_NUMPAD6
			 ,KEY_INPUT_NUMPAD7			// テンキー７				D_DIK_NUMPAD7
			 ,KEY_INPUT_NUMPAD8			// テンキー８				D_DIK_NUMPAD8
			 ,KEY_INPUT_NUMPAD9			// テンキー９				D_DIK_NUMPAD9
			 ,KEY_INPUT_MULTIPLY		// テンキー＊キー			D_DIK_MULTIPLY
			 ,KEY_INPUT_ADD				// テンキー＋キー			D_DIK_ADD
			 ,KEY_INPUT_SUBTRACT		// テンキー－キー			D_DIK_SUBTRACT
			 ,KEY_INPUT_DECIMAL			// テンキー．キー			D_DIK_DECIMAL
			 ,KEY_INPUT_DIVIDE			// テンキー／キー			D_DIK_DIVIDE
			 ,KEY_INPUT_NUMPADENTER		// テンキーのエンターキー	D_DIK_NUMPADENTER
			 ,KEY_INPUT_F1				// Ｆ１キー			D_DIK_F1
			 ,KEY_INPUT_F2				// Ｆ２キー			D_DIK_F2
			 ,KEY_INPUT_F3				// Ｆ３キー			D_DIK_F3
			 ,KEY_INPUT_F4				// Ｆ４キー			D_DIK_F4
			 ,KEY_INPUT_F5				// Ｆ５キー			D_DIK_F5
			 ,KEY_INPUT_F6				// Ｆ６キー			D_DIK_F6
			 ,KEY_INPUT_F7				// Ｆ７キー			D_DIK_F7
			 ,KEY_INPUT_F8				// Ｆ８キー			D_DIK_F8
			 ,KEY_INPUT_F9				// Ｆ９キー			D_DIK_F9
			 ,KEY_INPUT_F10				// Ｆ１０キー		D_DIK_F10
			 ,KEY_INPUT_F11				// Ｆ１１キー		D_DIK_F11
			 ,KEY_INPUT_F12				// Ｆ１２キー		D_DIK_F12
			 ,KEY_INPUT_A				// Ａキー			D_DIK_A
			 ,KEY_INPUT_B				// Ｂキー			D_DIK_B
			 ,KEY_INPUT_C				// Ｃキー			D_DIK_C
			 ,KEY_INPUT_D				// Ｄキー			D_DIK_D
			 ,KEY_INPUT_E				// Ｅキー			D_DIK_E
			 ,KEY_INPUT_F				// Ｆキー			D_DIK_F
			 ,KEY_INPUT_G				// Ｇキー			D_DIK_G
			 ,KEY_INPUT_H				// Ｈキー			D_DIK_H
			 ,KEY_INPUT_I				// Ｉキー			D_DIK_I
			 ,KEY_INPUT_J				// Ｊキー			D_DIK_J
			 ,KEY_INPUT_K				// Ｋキー			D_DIK_K
			 ,KEY_INPUT_L				// Ｌキー			D_DIK_L
			 ,KEY_INPUT_M				// Ｍキー			D_DIK_M
			 ,KEY_INPUT_N				// Ｎキー			D_DIK_N
			 ,KEY_INPUT_O				// Ｏキー			D_DIK_O
			 ,KEY_INPUT_P				// Ｐキー			D_DIK_P
			 ,KEY_INPUT_Q				// Ｑキー			D_DIK_Q
			 ,KEY_INPUT_R				// Ｒキー			D_DIK_R
			 ,KEY_INPUT_S				// Ｓキー			D_DIK_S
			 ,KEY_INPUT_T				// Ｔキー			D_DIK_T
			 ,KEY_INPUT_U				// Ｕキー			D_DIK_U
			 ,KEY_INPUT_V				// Ｖキー			D_DIK_V
			 ,KEY_INPUT_W				// Ｗキー			D_DIK_W
			 ,KEY_INPUT_X				// Ｘキー			D_DIK_X
			 ,KEY_INPUT_Y				// Ｙキー			D_DIK_Y
			 ,KEY_INPUT_Z				// Ｚキー			D_DIK_Z
			 ,KEY_INPUT_0 				// ０キー			D_DIK_0
			 ,KEY_INPUT_1				// １キー			D_DIK_1
			 ,KEY_INPUT_2				// ２キー			D_DIK_2
			 ,KEY_INPUT_3				// ３キー			D_DIK_3
			 ,KEY_INPUT_4				// ４キー			D_DIK_4
			 ,KEY_INPUT_5				// ５キー			D_DIK_5
			 ,KEY_INPUT_6				// ６キー			D_DIK_6
			 ,KEY_INPUT_7				// ７キー			D_DIK_7
			 ,KEY_INPUT_8				// ８キー			D_DIK_8
			 ,KEY_INPUT_9				// ９キー			D_DIK_9


			 //---------------------------------------------------------------------------------------------------------------
			 //
			 // mouse
			 //
			,MOUSE_INPUT_LEFT			// マウス左ボタン
			,MOUSE_INPUT_RIGHT			// マウス右ボタン
			,MOUSE_INPUT_MIDDLE			// マウス中央ボタン

	} ;

	//---------------------------------------------------------------------------------------------------------------------------------------
	Shared<Input> Input::Create(int32_t enable_stage, eJoypad use_joypad_id) noexcept {
		Shared<Input> inst = std::shared_ptr<Input>(new Input());
		inst->enable_stage_ = static_cast<int32_t>(enable_stage);
		inst->use_joypad_id_ = use_joypad_id;
		std::fill(std::begin(inst->btn_dual_time_limit), std::end(inst->btn_dual_time_limit), FLT_MAX);

		memcpy(inst->pad_trg_release, trg_mask_table_, sizeof(static_cast<int32_t>(eButton::PAD_BTN_NUM)));
		memcpy(inst->pad_xor_release, trg_mask_table_, sizeof(static_cast<int32_t>(eButton::PAD_BTN_NUM)));

		maneged_inputs_.emplace_back(inst);

		return inst;
	}

	//---------------------------------------------------------------------------------------------------------------------------------------
	void Input::ChangeEnableStage(int32_t target_satge) noexcept {
		before_stages_.push(running_stage_);
		running_stage_ = target_satge;
	}

	//---------------------------------------------------------------------------------------------------------------------------------------
	void Input::UndoEnableStage() noexcept {
		if (before_stages_.empty()) return;
		running_stage_ = before_stages_.top();
		before_stages_.pop();
	}

	//---------------------------------------------------------------------------------------------------------------------------------------
	void Input::update() noexcept {

		if (!checkEnable()) return;
		if (eJoypad::UNUSED == use_joypad_id_) return;

		memset(pad_keep, 0, sizeof(pad_keep));
		memset(pad_trg_down, 0, sizeof(pad_trg_down));
		memset(pad_trg_release, 0, sizeof(pad_trg_release));
		

		XINPUT_STATE joystate;
		GetJoypadXInputState(static_cast<int>(use_joypad_id_), &joystate);

		int32_t stick_threshold = static_cast<int32_t>(STICK_THRESHOLD_MAX * stick_threshold_);
		int32_t trigger_threshold = static_cast<int32_t>(TRIGGER_THRESHOLD_MAX * trigger_threshold_);

		for (int32_t i = 0; i < static_cast<int32_t>(eButton::PAD_BTN_NUM); ++i ) {

			bool f_on_bit = false;
			int32_t elem = static_cast<int32_t>(eButton::PAD_MIN) + i;
			switch (static_cast<eButton>(elem)) {
				case eButton::PAD_A:		f_on_bit = (joystate.Buttons[XINPUT_BUTTON_A]); break;
				case eButton::PAD_B:		f_on_bit = (joystate.Buttons[XINPUT_BUTTON_B]); break;
				case eButton::PAD_X:		f_on_bit = (joystate.Buttons[XINPUT_BUTTON_X]); break;
				case eButton::PAD_Y:		f_on_bit = (joystate.Buttons[XINPUT_BUTTON_Y]); break;
				case eButton::PAD_L:		f_on_bit = (joystate.Buttons[XINPUT_BUTTON_LEFT_SHOULDER]); break;
				case eButton::PAD_R:		f_on_bit = (joystate.Buttons[XINPUT_BUTTON_RIGHT_SHOULDER]); break;
				case eButton::PAD_SELECT:	f_on_bit = (joystate.Buttons[XINPUT_BUTTON_BACK]); break;
				case eButton::PAD_START:	f_on_bit = (joystate.Buttons[XINPUT_BUTTON_START]); break;
				case eButton::PAD_L_STICK:	f_on_bit = (joystate.Buttons[XINPUT_BUTTON_LEFT_THUMB]); break;
				case eButton::PAD_R_STICK:	f_on_bit = (joystate.Buttons[XINPUT_BUTTON_RIGHT_THUMB]); break;
				case eButton::PAD_UP:		f_on_bit = (joystate.Buttons[XINPUT_BUTTON_DPAD_UP]); break;
				case eButton::PAD_RIGHT:	f_on_bit = (joystate.Buttons[XINPUT_BUTTON_DPAD_RIGHT]); break;
				case eButton::PAD_DOWN:		f_on_bit = (joystate.Buttons[XINPUT_BUTTON_DPAD_DOWN]); break;
				case eButton::PAD_LEFT:		f_on_bit = (joystate.Buttons[XINPUT_BUTTON_DPAD_LEFT]); break;
				case eButton::PAD_L_STICK_UP:		f_on_bit = (-stick_threshold > joystate.ThumbLY); break;
				case eButton::PAD_L_STICK_RIGHT:	f_on_bit = (stick_threshold < joystate.ThumbLX); break;
				case eButton::PAD_L_STICK_DOWN:		f_on_bit = (stick_threshold < joystate.ThumbLY); break;
				case eButton::PAD_L_STICK_LEFT:		f_on_bit = (-stick_threshold > joystate.ThumbLX); break;
				case eButton::PAD_R_STICK_UP:		f_on_bit = (-stick_threshold > joystate.ThumbRY); break;
				case eButton::PAD_R_STICK_RIGHT:	f_on_bit = (stick_threshold < joystate.ThumbRX); break;
				case eButton::PAD_R_STICK_DOWN:		f_on_bit = (stick_threshold < joystate.ThumbRY); break;
				case eButton::PAD_R_STICK_LEFT:		f_on_bit = (-stick_threshold > joystate.ThumbRX); break;
				case eButton::PAD_L_TRIGGER:		f_on_bit = (trigger_threshold < joystate.LeftTrigger); break;
				case eButton::PAD_R_TRIGGER:		f_on_bit = (-trigger_threshold > joystate.RightTrigger); break;
			}

			if(f_on_bit){
				pad_keep[i] = 0x01;
				pad_trg_down[i] = pad_xor_down[i] ^ 0x01;
				pad_xor_down[i] = 0x01;
				pad_xor_release[i] = 0;
			}
			else {
				pad_xor_down[i] = 0;
				pad_trg_release[i] = pad_xor_release[i] ^ 0x01;
				pad_xor_release[i] = 0x01;
			}
		}

	}


	//---------------------------------------------------------------------------------------------------------------------------------------
	bool Input::barrage(eButton button, float first_time_space, float since_time_space) noexcept {
		if (!checkEnable()) return false;
		int index = static_cast<int>(button);
		if (pressed(button)) {
			btn_barrage_keep_time_limit[index] = first_time_space;
			return true;
		}
		if (keep(button)) {
			btn_barrage_keep_time_count[index] += dxe::GetDeltaTime();
			if (btn_barrage_keep_time_count[index] >= btn_barrage_keep_time_limit[index]) {
				btn_barrage_keep_time_count[index] = 0;
				btn_barrage_keep_time_limit[index] = since_time_space;
				return true;
			}
			return false;
		}
		btn_barrage_keep_time_count[index] = 0;
		return false;
	}


	//---------------------------------------------------------------------------------------------------------------------------------------
	Input::fDualReaction Input::dual(eButton button, float time_limit, eDualFirstReaction first_reaction, eDualSecondReaction second_reaction) noexcept {
		if (!checkEnable()) return fDualReaction::NONE ;
		int index = static_cast<int>(button);

		if (released( button )) {
			btn_dual_time_limit[index] = 0;
		}
		if (!keep(button)) {
			if (!tnl::IsAlmostEqualRelative(btn_dual_time_limit[index], FLT_MAX)) {
				btn_dual_time_limit[index] += dxe::GetDeltaTime();
			}
			if (btn_dual_time_limit[index] > time_limit) {
				btn_dual_time_limit[index] = FLT_MAX;
			}
		}

		if (tnl::IsAlmostEqualRelative(btn_dual_time_limit[index], FLT_MAX)) {
			if (eDualFirstReaction::KEEP == first_reaction) {
				if (keep(button)) return fDualReaction::FIRST;
			}
			else if (eDualFirstReaction::PRESSED == first_reaction) {
				if(pressed(button)) return fDualReaction::FIRST;
				else return fDualReaction::NONE;
			}
			else {
				return fDualReaction::NONE;
			}
		}
		else if (btn_dual_time_limit[index] < time_limit) {
			if (eDualSecondReaction::KEEP == second_reaction) {
				if (keep(button)) return fDualReaction::SECOND;
			}
			else if (eDualSecondReaction::PRESSED == second_reaction) {
				if (pressed(button)) return fDualReaction::SECOND;
			}
		}

		return fDualReaction::NONE;

	}


	//---------------------------------------------------------------------------------------------------------------------------------------
	float Input::getValue(eVariable type)  const noexcept {

		if (!checkEnable()) return 0 ;

		switch (type) {
		case eVariable::MOUSE_VEL_X :
			return forword_mouse_point_.x - before_mouse_point_.x;
			break;
		case eVariable::MOUSE_VEL_Y:
			return forword_mouse_point_.y - before_mouse_point_.y;
			break;
		case eVariable::MOUSE_VEL_W:
			return GetMouseWheelRotVolF();
			break;
		}
		if (eJoypad::UNUSED == use_joypad_id_) return 0;


		XINPUT_STATE joystate;
		GetJoypadXInputState(static_cast<int>(use_joypad_id_), &joystate);


		if (eVariable::PAD_L_STICK_X == type) {
			return float(joystate.ThumbLX) / STICK_THRESHOLD_MAX;
		}
		if (eVariable::PAD_L_STICK_Y == type) {
			return float(joystate.ThumbLY) / STICK_THRESHOLD_MAX;
		}
		if (eVariable::PAD_R_STICK_X == type) {
			return float(joystate.ThumbRX) / STICK_THRESHOLD_MAX;
		}
		if (eVariable::PAD_R_STICK_Y == type) {
			return float(joystate.ThumbRY) / STICK_THRESHOLD_MAX;
		}
		if (eVariable::PAD_L_TRIGGER == type) {
			return float(joystate.LeftTrigger) / TRIGGER_THRESHOLD_MAX;
		}
		if (eVariable::PAD_R_TRIGGER == type) {
			return float(joystate.RightTrigger) / TRIGGER_THRESHOLD_MAX;
		}

		return 0;
	}


	//---------------------------------------------------------------------------------------------------------------------------------------
	void Input::drawDebugJoypadInfo(const tnl::Vector2f& position) {
		if (eJoypad::UNUSED == use_joypad_id_) {
			DrawStringF(position.x, position.y, "unused joypad", -1);
			return;
		}
		if (!checkEnable()) {
			DrawStringF(position.x, position.y, "not enable stage", -1);
			return;
		}

		XINPUT_STATE joystate;
		GetJoypadXInputState(static_cast<int>(use_joypad_id_), &joystate);

		float y = position.y - 20 ;
		y += 20; DrawFormatStringF(position.x, y, -1, "%d%d%d%d%d", joystate.Buttons[0] >> 7, joystate.Buttons[1] >> 7, joystate.Buttons[2] >> 7, joystate.Buttons[3] >> 7, joystate.Buttons[4] >> 7);
		y += 20; DrawFormatStringF(position.x, y, -1, "%d%d%d%d%d", joystate.Buttons[5] >> 7, joystate.Buttons[6] >> 7, joystate.Buttons[7] >> 7, joystate.Buttons[8] >> 7, joystate.Buttons[9] >> 7);
		y += 20; DrawFormatStringF(position.x, y, -1, "%d%d%d%d%d", joystate.Buttons[10] >> 7, joystate.Buttons[11] >> 7, joystate.Buttons[12] >> 7, joystate.Buttons[13] >> 7, joystate.Buttons[14] >> 7);
		y += 20; DrawFormatStringF(position.x, y, -1, "%d%d%d%d%d", joystate.Buttons[15] >> 7);
		y += 20; DrawFormatStringF(position.x, y, -1, "%d", joystate.ThumbLX);
		y += 20; DrawFormatStringF(position.x, y, -1, "%d", joystate.ThumbLY);
		y += 20; DrawFormatStringF(position.x, y, -1, "%d", joystate.ThumbRX);
		y += 20; DrawFormatStringF(position.x, y, -1, "%d", joystate.ThumbRY);
		y += 20; DrawFormatStringF(position.x, y, -1, "%d", joystate.LeftTrigger);
		y += 20; DrawFormatStringF(position.x, y, -1, "%d", joystate.RightTrigger);
	}

	//==============================================================================================================================================




	//---------------------------------------------------------------------------------------------------------------------------------------
	void Input::Initialize() noexcept {
		for (int i = 0; i < static_cast<int>(eButton::UNIQUE_DEVICE_BUTTON_MAX); ++i) {
			trg_mask_table_[i] = 1 ;
		}
		memcpy(btn_trg_release, trg_mask_table_, sizeof(trg_mask_table_));
		memcpy(btn_xor_release, trg_mask_table_, sizeof(trg_mask_table_));

		int mx, my;
		GetMousePoint(&mx, &my);
		forword_mouse_point_ = { (float)mx, (float)(my) };
	}

	//---------------------------------------------------------------------------------------------------------------------------------------
	void Input::Update() noexcept {

		before_mouse_point_ = forword_mouse_point_;
		int mx, my;
		GetMousePoint(&mx, &my);
		forword_mouse_point_ = { (float)mx, (float)(my) };

		memset(btn_keep, 0, sizeof(btn_keep));
		memset(btn_trg_down, 0, sizeof(btn_trg_down));
		memset(btn_trg_release, 0, sizeof(btn_trg_release));

		for (int32_t i = 0; i < static_cast<int32_t>( eButton::UNIQUE_DEVICE_BUTTON_MAX ); ++i) {
			bool f_on_bit = false;
			if (i < static_cast<int32_t>(eButton::KB_MAX))			f_on_bit = CheckHitKey( dxlib_button_table_[i] );
			else if (i < static_cast<int32_t>(eButton::MOUSE_MAX))	f_on_bit = GetMouseInput() & dxlib_button_table_[i] ;

			if (f_on_bit) {
				btn_keep[i] = 0x01;
				btn_trg_down[i] = btn_xor_down[i] ^ 0x01;
				btn_xor_down[i] = 0x01;
				btn_xor_release[i] = 0;
			}
			else {
				btn_xor_down[i] = 0;
				btn_trg_release[i] = btn_xor_release[i] ^ 0x01;
				btn_xor_release[i] = 0x01;
			}
		}


		auto it = maneged_inputs_.begin();
		Shared<Input> ipt = nullptr;
		while (it != maneged_inputs_.end()) {
			ipt = (*it).lock();
			if (!ipt) {
				it = maneged_inputs_.erase(it);
				continue;
			}
			ipt->update();
			it++;
		}
	}

	//---------------------------------------------------------------------------------------------------------------------------------------
	void Input::Destroy() noexcept {

		maneged_inputs_.clear();
	}

}