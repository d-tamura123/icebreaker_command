#pragma once
#include <stack>
#include "../tnl/tnl_vector2f.h"
#include "../tnl/tnl_using.h"
#include "../tnl/tnl_util.h"

namespace dxe {

	class Input {
	public :

		//---------------------------------------------------------------------------------------------------------------------------------
		enum class eJoypad {
			UNUSED, PAD1, PAD2, PAD3, PAD4, MAX
		};
		enum class fDualReaction { NONE = 1<<0, FIRST = 1<<1, SECOND = 1<<2 };
		enum class eDualFirstReaction { NONE, KEEP, PRESSED };
		enum class eDualSecondReaction { KEEP, PRESSED };

		//---------------------------------------------------------------------------------------------------------------------------------
		enum class eButton {

			//---------------------------------------------------------------------------------------------------------------
			//
			// keybord
			//
			KB_BACK				// BackSpaceキー	D_DIK_BACK
			, KB_TAB			// Tabキー			D_DIK_TAB
			, KB_RETURN			// Enterキー		D_DIK_RETURN
			, KB_LSHIFT			// 左Shiftキー		D_DIK_LSHIFT
			, KB_RSHIFT			// 右Shiftキー		D_DIK_RSHIFT
			, KB_LCONTROL		// 左Ctrlキー		D_DIK_LCONTROL
			, KB_RCONTROL		// 右Ctrlキー		D_DIK_RCONTROL
			, KB_ESCAPE			// Escキー			D_DIK_ESCAPE
			, KB_SPACE			// スペースキー		D_DIK_SPACE
			, KB_PGUP			// PageUpキー		D_DIK_PGUP
			, KB_PGDN			// PageDownキー		D_DIK_PGDN
			, KB_END			// Endキー			D_DIK_END
			, KB_HOME			// Homeキー			D_DIK_HOME
			, KB_LEFT			// 左キー			D_DIK_LEFT
			, KB_UP				// 上キー			D_DIK_UP
			, KB_RIGHT			// 右キー			D_DIK_RIGHT
			, KB_DOWN			// 下キー			D_DIK_DOWN
			, KB_INSERT			// Insertキー		D_DIK_INSERT
			, KB_DELETE			// Deleteキー		D_DIK_DELETE
			, KB_MINUS			// －キー			D_DIK_MINUS
			, KB_YEN			// ￥キー			D_DIK_YEN
			, KB_PREVTRACK		// ＾キー			D_DIK_PREVTRACK
			, KB_PERIOD			// ．キー			D_DIK_PERIOD
			, KB_SLASH			// ／キー			D_DIK_SLASH
			, KB_LALT			// 左Altキー		D_DIK_LALT
			, KB_RALT			// 右Altキー		D_DIK_RALT
			, KB_SCROLL			// ScrollLockキー	D_DIK_SCROLL
			, KB_SEMICOLON		// ；キー			D_DIK_SEMICOLON
			, KB_COLON			// ：キー			D_DIK_COLON
			, KB_LBRACKET		// ［キー			D_DIK_LBRACKET
			, KB_RBRACKET		// ］キー			D_DIK_RBRACKET
			, KB_AT				// ＠キー			D_DIK_AT
			, KB_BACKSLASH		// ＼キー			D_DIK_BACKSLASH
			, KB_COMMA			// ，キー			D_DIK_COMMA
			, KB_KANJI			// 漢字キー			D_DIK_KANJI
			, KB_CONVERT		// 変換キー			D_DIK_CONVERT
			, KB_NOCONVERT		// 無変換キー		D_DIK_NOCONVERT
			, KB_KANA			// カナキー			D_DIK_KANA
			, KB_APPS			// アプリケーションメニューキー		D_DIK_APPS
			, KB_CAPSLOCK		// CaspLockキー		D_DIK_CAPSLOCK
			, KB_SYSRQ			// PrintScreenキー	D_DIK_SYSRQ
			, KB_PAUSE			// PauseBreakキー	D_DIK_PAUSE
			, KB_LWIN			// 左Winキー		D_DIK_LWIN
			, KB_RWIN			// 右Winキー		D_DIK_RWIN
			, KB_NUMLOCK		// テンキーNumLockキー		D_DIK_NUMLOCK
			, KB_NUMPAD0		// テンキー０				D_DIK_NUMPAD0
			, KB_NUMPAD1		// テンキー１				D_DIK_NUMPAD1
			, KB_NUMPAD2		// テンキー２				D_DIK_NUMPAD2
			, KB_NUMPAD3		// テンキー３				D_DIK_NUMPAD3
			, KB_NUMPAD4		// テンキー４				D_DIK_NUMPAD4
			, KB_NUMPAD5		// テンキー５				D_DIK_NUMPAD5
			, KB_NUMPAD6		// テンキー６				D_DIK_NUMPAD6
			, KB_NUMPAD7		// テンキー７				D_DIK_NUMPAD7
			, KB_NUMPAD8		// テンキー８				D_DIK_NUMPAD8
			, KB_NUMPAD9		// テンキー９				D_DIK_NUMPAD9
			, KB_MULTIPLY		// テンキー＊キー			D_DIK_MULTIPLY
			, KB_ADD			// テンキー＋キー			D_DIK_ADD
			, KB_SUBTRACT		// テンキー－キー			D_DIK_SUBTRACT
			, KB_DECIMAL		// テンキー．キー			D_DIK_DECIMAL
			, KB_DIVIDE			// テンキー／キー			D_DIK_DIVIDE
			, KB_NUMPADENTER	// テンキーのエンターキー	D_DIK_NUMPADENTER
			, KB_F1				// Ｆ１キー			D_DIK_F1
			, KB_F2				// Ｆ２キー			D_DIK_F2
			, KB_F3				// Ｆ３キー			D_DIK_F3
			, KB_F4				// Ｆ４キー			D_DIK_F4
			, KB_F5				// Ｆ５キー			D_DIK_F5
			, KB_F6				// Ｆ６キー			D_DIK_F6
			, KB_F7				// Ｆ７キー			D_DIK_F7
			, KB_F8				// Ｆ８キー			D_DIK_F8
			, KB_F9				// Ｆ９キー			D_DIK_F9
			, KB_F10			// Ｆ１０キー		D_DIK_F10
			, KB_F11			// Ｆ１１キー		D_DIK_F11
			, KB_F12			// Ｆ１２キー		D_DIK_F12
			, KB_A				// Ａキー			D_DIK_A
			, KB_B				// Ｂキー			D_DIK_B
			, KB_C				// Ｃキー			D_DIK_C
			, KB_D				// Ｄキー			D_DIK_D
			, KB_E				// Ｅキー			D_DIK_E
			, KB_F				// Ｆキー			D_DIK_F
			, KB_G				// Ｇキー			D_DIK_G
			, KB_H				// Ｈキー			D_DIK_H
			, KB_I				// Ｉキー			D_DIK_I
			, KB_J				// Ｊキー			D_DIK_J
			, KB_K				// Ｋキー			D_DIK_K
			, KB_L				// Ｌキー			D_DIK_L
			, KB_M				// Ｍキー			D_DIK_M
			, KB_N				// Ｎキー			D_DIK_N
			, KB_O				// Ｏキー			D_DIK_O
			, KB_P				// Ｐキー			D_DIK_P
			, KB_Q				// Ｑキー			D_DIK_Q
			, KB_R				// Ｒキー			D_DIK_R
			, KB_S				// Ｓキー			D_DIK_S
			, KB_T				// Ｔキー			D_DIK_T
			, KB_U				// Ｕキー			D_DIK_U
			, KB_V				// Ｖキー			D_DIK_V
			, KB_W				// Ｗキー			D_DIK_W
			, KB_X				// Ｘキー			D_DIK_X
			, KB_Y				// Ｙキー			D_DIK_Y
			, KB_Z				// Ｚキー			D_DIK_Z
			, KB_0 				// ０キー			D_DIK_0
			, KB_1				// １キー			D_DIK_1
			, KB_2				// ２キー			D_DIK_2
			, KB_3				// ３キー			D_DIK_3
			, KB_4				// ４キー			D_DIK_4
			, KB_5				// ５キー			D_DIK_5
			, KB_6				// ６キー			D_DIK_6
			, KB_7				// ７キー			D_DIK_7
			, KB_8				// ８キー			D_DIK_8
			, KB_9				// ９キー			D_DIK_9
			, KB_MAX

			//---------------------------------------------------------------------------------------------------------------
			//
			// mouse
			//
			, MOUSE_MIN = KB_MAX
			, MOUSE_LEFT = MOUSE_MIN	// マウス左ボタン
			, MOUSE_RIGHT				// マウス右ボタン
			, MOUSE_MIDDLE				// マウス中央ボタン
			, MOUSE_MAX
			, MOUSE_NUM = MOUSE_MAX - MOUSE_MIN


			, UNIQUE_DEVICE_BUTTON_MAX = MOUSE_MAX

			//---------------------------------------------------------------------------------------------------------------
			//
			// joypad
			//
			, PAD_MIN = MOUSE_MAX
			, PAD_A	= PAD_MIN		// Ａボタン
			, PAD_B					// Ｂボタン
			, PAD_X					// Ｘボタン
			, PAD_Y					// Ｙボタン
			, PAD_L					// Ｌボタン
			, PAD_R					// Ｒボタン
			, PAD_SELECT			// SELECT ボタン
			, PAD_START				// STARTボタン
			, PAD_L_STICK			// Lスティックボタン
			, PAD_R_STICK			// Rスティックボタン
			, PAD_UP				// 十字キー　↑
			, PAD_RIGHT				// 十字キー　→
			, PAD_DOWN				// 十字キー　↓
			, PAD_LEFT				// 十字キー　←
			, PAD_L_STICK_UP		// Lスティック　↑　
			, PAD_L_STICK_RIGHT		// Lスティック　→　
			, PAD_L_STICK_DOWN		// Lスティック　↓　
			, PAD_L_STICK_LEFT		// Lスティック　←　
			, PAD_R_STICK_UP		// Rスティック　↑　
			, PAD_R_STICK_RIGHT		// Rスティック　→　
			, PAD_R_STICK_DOWN		// Rスティック　↓　
			, PAD_R_STICK_LEFT		// Rスティック　←　
			, PAD_L_TRIGGER			// Lトリガーキー
			, PAD_R_TRIGGER			// Rトリガーキー
			, PAD_MAX
			, PAD_BTN_NUM = PAD_MAX - PAD_MIN
			//---------------------------------------------------------------------------------------------------------------------------------

			, E_BUTTON_MAX = PAD_MAX
		};

		//---------------------------------------------------------------------------------------------------------------------------------
		enum class eVariable {
			MOUSE_VEL_X			// マウス X 座標変化量
			, MOUSE_VEL_Y		// マウス Y 座標変化量
			, MOUSE_VEL_W		// マウスホイール変化量
			, PAD_L_STICK_X		// 左スティック X 軸 傾倒量
			, PAD_L_STICK_Y		// 左スティック Y 軸 傾倒量
			, PAD_R_STICK_X		// 右スティック X 軸 傾倒量
			, PAD_R_STICK_Y		// 右スティック Y 軸 傾倒量
			, PAD_L_TRIGGER		// 左トリガー押下量
			, PAD_R_TRIGGER		// 右トリガー押下量
		};


		// 生成
		// arg1... 有効ステージ
		// arg2... 使用するジョイパッド
		static Shared<Input> Create(int32_t enable_stage = 0, eJoypad use_joypad_id = eJoypad::UNUSED) noexcept ;

		// 入力検知の有効ステージ設定
		// 設定するステージ
		static void ChangeEnableStage(int32_t target_satge) noexcept ;

		// 有効ステージを前回設定へ戻す
		static void UndoEnableStage() noexcept ;

		// 現在の有効ステージを取得
		static int32_t GetEnableStage() noexcept { return running_stage_; }


		// 使用するキーボードのキーを追加
		// arg1... eButton ( 可変個 )
		template<typename... tKeys>
		void addDetectKeybord(tKeys... iParams) noexcept {
			const eButton array[] = { static_cast<eButton>(iParams)... };
			for (eButton elem : array) {
				if (elem >= eButton::KB_MAX) continue;
				check_buttons_.emplace_back( elem );
			}
		}

		// 押しっぱなし入力検知
		// arg1... eButton ( 可変個 )
		// ret.... [ 入力有り : true ] [ 入力無し : false ]
		template<typename... tBtns>
		bool keep(tBtns... iParams) const noexcept {
			if (!checkEnable()) return false ;
			const eButton array[] = { static_cast<eButton>(iParams)... };
			for (eButton elem : array) {
				if (static_cast<int32_t>(elem) < static_cast<int32_t>(eButton::UNIQUE_DEVICE_BUTTON_MAX)) {
					if (btn_keep[static_cast<int32_t>(elem)]) return true;
				}
				else {
					if (pad_keep[static_cast<int32_t>(elem) - static_cast<int32_t>(eButton::PAD_MIN)]) return true;
				}
			}
			return false;
		}

		// 押された瞬間
		// arg1... eButton ( 可変個 )
		// ret.... [ 入力有り : true ] [ 入力無し : false ]
		template<typename... tBtns>
		bool pressed(tBtns... iParams) const noexcept {
			if (!checkEnable()) return false;
			const eButton array[] = { static_cast<eButton>(iParams)... };
			for (eButton elem : array) {
				if (static_cast<int32_t>(elem) < static_cast<int32_t>(eButton::UNIQUE_DEVICE_BUTTON_MAX)) {
					if (0 != btn_trg_down[static_cast<int>(elem)]) return true;
				}
				else {
					if (0 != pad_trg_down[static_cast<int32_t>(elem) - static_cast<int32_t>(eButton::PAD_MIN)]) return true;
				}
			}
			return false;
		}

		// 離された瞬間
		// arg1... eButton ( 可変個 )
		// ret.... [ 入力有り : true ] [ 入力無し : false ]
		template<typename... tBtns>
		bool released(tBtns... iParams) const noexcept {
			if (!checkEnable()) return false;
			const eButton array[] = { static_cast<eButton>(iParams)... };
			for (eButton elem : array) {
				if (static_cast<int32_t>(elem) < static_cast<int32_t>(eButton::UNIQUE_DEVICE_BUTTON_MAX)) {
					if (0 != btn_trg_release[static_cast<int>(elem)]) return true;
				}
				else {
					if (0 != pad_trg_release[static_cast<int32_t>(elem) - static_cast<int32_t>(eButton::PAD_MIN)]) return true;
				}
			}
			return false;
		}


		// 押しっぱなしでの連打判定
		// arg1... eButton
		// arg2... 初めに押されてから二度目の判定を何秒後に行うか
		// arg3... 三度目以降を何秒間隔で行うか
		// ret.... [ 入力有り : true ] [ 入力無し : false ]
		bool barrage(eButton button, float first_time_space, float since_time_space) noexcept ;


		// 二度押し判定
		// arg1... eButton
		// arg2... 判定時間間隔 ( 秒 )
		// arg3... 一度目の入力に対する反応 eDualFirstReaction [ 反応なし : NONE ] [ キープ扱い : KEEP ] [ 瞬間のみ : PRESSED ]
		// arg4... 二度目の入力に対する反応 eDualSecondReaction [ キープ扱い : KEEP ] [ 瞬間のみ : PRESSED ]
		// ret.... arg3,4 で指定したパラメータによって 入力判定が fDualReaction のビットフラグが帰り 初回入力か二度目の入力かを判定できます
		fDualReaction dual(eButton button, float time_limit, eDualFirstReaction first_reaction = eDualFirstReaction::PRESSED, eDualSecondReaction second_reaction = eDualSecondReaction::PRESSED) noexcept;


		// パラメータで得られる入力値
		// arg1... eVariable
		// ret.... 入力があった値 ( 0 は基本的に入力なし )
		// tips1.. 左右のトリガーに関して DirectInput は左右トリガーを一つの変数で
		//         管理している関係で [ + 領域が左トリガー ] [ - 領域が右トリガー ]
		//         となっています、そのため両方を同時押しすると +- が相殺されてしまうので注意
		//         要するに左右トリガーは両方を同時使用しない仕様でゲームを作った方が良い
		// tips2.. 右スティックに関して パッドの種類によっては反応しない可能性があります
		// tips3.. 各値は -1.0f ～ +1.0f までの範囲で取得できます
		float getValue(eVariable type) const noexcept ;


		// ジョイパッドの入力情報出力
		void drawDebugJoypadInfo(const tnl::Vector2f& position);


		// スティック傾倒による単発処理の閾値
		TNL_PROPERTY(float, StickThreshold, stick_threshold_);

		// トリガー押下量による単発処理の閾値
		TNL_PROPERTY(float, TriggerThreshold, trigger_threshold_);



		//---------------------------------------------------------------------------------------------------------------------------
		//
		// 以下ユーザー使用禁止
		//
		static void Initialize() noexcept ;
		static void Update() noexcept ;
		static void Destroy() noexcept ;
	private :
		static constexpr float STICK_THRESHOLD_MAX = 32767.0f;
		static constexpr float TRIGGER_THRESHOLD_MAX = 255.0f;

		float btn_barrage_keep_time_count[static_cast<int32_t>(eButton::E_BUTTON_MAX)] = { 0 };
		float btn_barrage_keep_time_limit[static_cast<int32_t>(eButton::E_BUTTON_MAX)] = { 0 };
		float btn_dual_time_limit[static_cast<int32_t>(eButton::E_BUTTON_MAX)] = { 0 };

		char pad_keep[static_cast<int32_t>(eButton::PAD_BTN_NUM)] = { 0 };
		char pad_xor_down[static_cast<int32_t>(eButton::PAD_BTN_NUM)] = { 0 };
		char pad_trg_down[static_cast<int32_t>(eButton::PAD_BTN_NUM)] = { 0 };
		char pad_xor_release[static_cast<int32_t>(eButton::PAD_BTN_NUM)] = { 0 };
		char pad_trg_release[static_cast<int32_t>(eButton::PAD_BTN_NUM)] = { 0 };

		std::vector<eButton> check_buttons_;
		std::vector<eVariable> check_values_;
		int32_t enable_stage_ = 0;
		float stick_threshold_ = 0.3f;
		float trigger_threshold_ = 0.3f;
		eJoypad use_joypad_id_ = eJoypad::UNUSED;
		bool is_active_ = true;
		void update() noexcept ;
		bool checkEnable() const noexcept { return (is_active_ && (running_stage_ == enable_stage_)); }

		static char btn_keep[static_cast<int32_t>(eButton::UNIQUE_DEVICE_BUTTON_MAX)] ;
		static char btn_xor_down[static_cast<int32_t>(eButton::UNIQUE_DEVICE_BUTTON_MAX)] ;
		static char btn_trg_down[static_cast<int32_t>(eButton::UNIQUE_DEVICE_BUTTON_MAX)] ;
		static char btn_xor_release[static_cast<int32_t>(eButton::UNIQUE_DEVICE_BUTTON_MAX)] ;
		static char btn_trg_release[static_cast<int32_t>(eButton::UNIQUE_DEVICE_BUTTON_MAX)] ;
		static char trg_mask_table_[static_cast<int32_t>(eButton::UNIQUE_DEVICE_BUTTON_MAX)];

		static std::vector<int> dxlib_button_table_;
		static std::stack<int32_t> before_stages_;
		static std::list<Weak<Input>> maneged_inputs_;
		static tnl::Vector2f before_mouse_point_;
		static tnl::Vector2f forword_mouse_point_;
		static int32_t running_stage_;
	};
	TNL_ENUM_CLASS_BIT_MASK_OPERATOR(Input::fDualReaction);
}

