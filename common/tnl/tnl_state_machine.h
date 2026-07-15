#pragma once
#include <deque>
#include <functional>
#include "tnl_using.h"

namespace tnl{

	//--------------------------------------------------------------------------------------------------------------------------
	// StateMachine
	//
	// 任意の処理を「状態」として登録し、update() 呼び出しごとに実行するステートマシン。
	// 状態は std::function<void()> で定義し、状態の切り替えや undo（巻き戻し）機能を持つ。
	// 
	// 主な特徴：
	//   - 毎フレームの update で現在の状態処理を実行
	//   - change() で次フレームから実行する状態へ切り替え
	//   - immediatelyChange() で即座に状態を切り替え
	//   - undo() で前の状態に戻る（undo回数制限あり）
	//   - 経過時間・経過フレーム数を取得可能
	//
	// 想定用途：
	//   - ゲームのシーン管理
	//   - キャラクターの状態遷移（移動・攻撃・待機など）
	//   - 非同期的な処理フロー制御
	//
	class StateMachine final {
	public:
		~StateMachine() = default;

		using State = std::function<void()>;
		#define TNL_STATE_BIND( func, instance ) std::function<void()>(std::bind(func, instance))
		#define TNL_STATE_MACHINE_CREATE( func, instance ) tnl::StateMachine::Create( std::function<void()>(std::bind(func, instance)) )

		//---------------------------------------------------------------------------------------------------------
		// 生成
		// arg1... update で実行されるステートの初期メソッド
		static Shared<StateMachine> Create(const State& func);

		//---------------------------------------------------------------------------------------------------------
		// アップデート ( 毎フレーム呼び出せばOK )
		// arg1... フレーム間の経過時間( 秒のデルタタイム )
		void update(float delta_time);

		//---------------------------------------------------------------------------------------------------------
		// 初期化用　最初の１フレームだけ true が帰る
		inline bool isStart() const { return is_start_; }

		//---------------------------------------------------------------------------------------------------------
		// 経過時間を取得 ( 秒 )
		inline float getProgressTime() const { return sum_time_; }

		//---------------------------------------------------------------------------------------------------------
		// 経過フレーム数を取得
		inline uint32_t getProgressFrame() const { return sum_frame_; }

		//---------------------------------------------------------------------------------------------------------
		// 実行される処理の変更
		// arg1... 次のフレームから実行させるメソッドを指定
		void change(const State& func);

		//---------------------------------------------------------------------------------------------------------
		// 処理内容を即座に変更
		// arg1... 実行させるメソッドを指定
		// tisp... 次フレームを待たず即座にシーケンスを変更する
		void immediatelyChange(const State& func);

		//---------------------------------------------------------------------------------------------------------
		// 指定秒後に処理内容を変更
		// arg1... 実行させるメソッドを指定
		// arg2... 切り替わるまでの時間
		void delayChangeByTime(const State& func, float delay_time);

		//---------------------------------------------------------------------------------------------------------
		// 指定フレーム経過で処理内容を変更
		// arg1... 実行させるメソッドを指定
		// arg2... 切り替わるまでの時間
		void delayChangeByFrame(const State& func, int32_t delay_frame);

		//---------------------------------------------------------------------------------------------------------
		// 1つ前の処理に戻る
		// tips... 前の処理が存在しなければ何もしない
		void undo();

		//---------------------------------------------------------------------------------------------------------
		// undo 処理の上限を設定する
		// tips... デフォルトは 0 回
		inline void setUndoLimit(uint32_t undo_limit) { undo_limit_ = undo_limit; }


	private:
		enum class eDelayType {
			None,          // 遅延なし
			ByTime,        // 時間による遅延
			ByFrame,       // フレームによる遅延
		};

		StateMachine() = default;
		void pushToHistory();

		State now_;
		State next_;
		std::deque<State> prevs_;

		bool is_start_ = true;
		bool is_change_ = false;
		float sum_time_ = 0;
		float delay_time_ = 0;
		float delay_time_count_ = 0;
		int32_t delay_frame_ = 0;
		int32_t delay_frame_count_ = 0;
		uint32_t sum_frame_ = 0;
		uint32_t undo_limit_ = 0;
		eDelayType delay_type_ = eDelayType::None; // 現在の遅延モード
	};

}


