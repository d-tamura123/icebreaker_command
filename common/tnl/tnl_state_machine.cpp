#include "tnl_state_machine.h"


namespace tnl {

	//---------------------------------------------------------------------------------------------------------
	Shared<StateMachine> StateMachine::Create(const State& func) {
		Shared<StateMachine> instance = std::shared_ptr<StateMachine>(new StateMachine());
		instance->next_ = func;
		instance->now_ = func;
		return instance;
	}

	//---------------------------------------------------------------------------------------------------------
	void StateMachine::update(float deltatime) {
		sum_time_ += deltatime;
		if (now_) now_();
		sum_frame_++;
		is_start_ = false;

		if (!is_change_) return; // 変更要求がなければ何もしない

		// 遅延条件のチェック
		bool can_change = false; // 状態遷移してよいかどうかのフラグ
		if (delay_type_ == eDelayType::ByTime) {
			delay_time_count_ += deltatime;
			if (delay_time_count_ >= delay_time_) {
				can_change = true;
			}
		}
		else if (delay_type_ == eDelayType::ByFrame) {
			delay_frame_count_++;
			if (delay_frame_count_ >= delay_frame_) {
				can_change = true;
			}
		}
		else { // DelayType::None の場合
			can_change = true;
		}

		if (!can_change) return; // 遷移条件を満たしていなければここで抜ける

		// --- 状態遷移の実行 ---
		pushToHistory();
		now_ = next_;
		is_start_ = true;
		is_change_ = false;
		sum_time_ = 0;
		sum_frame_ = 0;

		// 遅延タイマーをリセット
		delay_type_ = eDelayType::None;
		delay_time_count_ = 0;
		delay_frame_count_ = 0;
	}

	//---------------------------------------------------------------------------------------------------------
	void StateMachine::change(const State& func) {
		next_ = func;
		is_change_ = true;
		delay_type_ = eDelayType::None; // 遅延なしに設定
	}

	//---------------------------------------------------------------------------------------------------------
	void StateMachine::delayChangeByTime(const State& func, float delay_time) {
		next_ = func;
		is_change_ = true;
		delay_type_ = eDelayType::ByTime; // 時間遅延に設定
		delay_time_ = delay_time;
		delay_time_count_ = 0;
	}

	//---------------------------------------------------------------------------------------------------------
	void StateMachine::delayChangeByFrame(const State& func, int32_t delay_frame) {
		next_ = func;
		is_change_ = true;
		delay_type_ = eDelayType::ByFrame; // フレーム遅延に設定
		delay_frame_ = delay_frame;
		delay_frame_count_ = 0;
	}

	//---------------------------------------------------------------------------------------------------------
	void StateMachine::immediatelyChange(const State& func) {
		pushToHistory();	// now_ が上書きされる直前に履歴へ保存
		now_ = func;
		next_ = func;		// next_ も同期させておくと、意図しない挙動を防げる

		// 状態情報をリセット
		is_start_ = true;
		sum_time_ = 0;
		sum_frame_ = 0;

		// 保留中の変更要求と遅延をキャンセル
		is_change_ = false;
		delay_type_ = eDelayType::None;
		delay_time_count_ = 0;
		delay_frame_count_ = 0;
	}



	//---------------------------------------------------------------------------------------------------------
	void StateMachine::undo() {
		if (prevs_.empty()) return;
		next_ = prevs_.front();
		prevs_.pop_front();
		is_change_ = true;
	}

	//---------------------------------------------------------------------------------------------------------
	void StateMachine::pushToHistory() {
		prevs_.push_front(now_);
		if (prevs_.size() > undo_limit_) {
			prevs_.pop_back();
		}
	}


}
