#include "tnl_seek_unit.h"

namespace tnl {

	//-------------------------------------------------------------------------------------------------------
	Shared<SeekUnit> SeekUnit::Create
		( float fix_fps
		, uint32_t total_frame_num
		, float total_time
		, ePlayMode play_mode
		, ePlayDir play_dir) {

		Shared<SeekUnit> seek = std::shared_ptr<SeekUnit>(new SeekUnit());
		seek->fix_fps_ = fix_fps;
		seek->setTotalTime(total_time);
		seek->setTotalFrameNum(total_frame_num);
		seek->setPlayState(play_mode, play_dir);
		return seek;
	}

	//-------------------------------------------------------------------------------------------------------
	void SeekUnit::play() {
		if (eCondition::ENDED == condition_) return;
		TNL_SEEK_UNIT_LOCK_GUARD_MUTEX();
		condition_ = eCondition::PLAYING;
	}

	//-------------------------------------------------------------------------------------------------------
	void SeekUnit::pause() {
		if (eCondition::ENDED == condition_) return;
		TNL_SEEK_UNIT_LOCK_GUARD_MUTEX();
		condition_ = eCondition::STANDBY;
	}

	//-------------------------------------------------------------------------------------------------------
	void SeekUnit::stop() {
		if (eCondition::STANDBY == condition_) return;
		TNL_SEEK_UNIT_LOCK_GUARD_MUTEX();
		condition_ = eCondition::ENDED;
	}

	//-------------------------------------------------------------------------------------------------------
	void SeekUnit::restart() {
		TNL_SEEK_UNIT_LOCK_GUARD_MUTEX();
		if (ePlayDir::REVERSE == play_dir_) {
			jumpSeekRate(1.0);
			direction_ = -1;
		}
		else {
			jumpSeekRate(0.0);
			direction_ = 1;
		}
		condition_ = eCondition::PLAYING;
		frame_progress_time_ = 0;

		if (frame_callback_storage_.empty()) return;
		uint32_t start_frame = (ePlayDir::REVERSE == play_dir_) ? total_frame_num_ - 1 : 0;
		if (!frame_callback_storage_[start_frame]) return;
		if (call_of_trigger_) return;
		call_of_trigger_ = frame_callback_storage_[start_frame];
	}

	//-------------------------------------------------------------------------------------------------------
	void SeekUnit::jumpSeekRate(float seek_rate) {
		TNL_SEEK_UNIT_LOCK_GUARD_MUTEX();
		seek_position_ = total_time_ * seek_rate;
		seek_position_ = std::clamp<float>(seek_position_, 0, total_time_);
	}

	//-------------------------------------------------------------------------------------------------------
	void SeekUnit::jumpSeekFrame(uint32_t frame) {
		TNL_SEEK_UNIT_LOCK_GUARD_MUTEX();
		frame = std::clamp<uint32_t>(frame, 0, (total_frame_num_ - 1));
		seek_position_ = (static_cast<float>(frame) / total_frame_num_) * total_time_;
		seek_position_ = std::clamp<float>(seek_position_, 0, total_time_);
		if (frame_callback_storage_.empty()) return;
		if (!frame_callback_storage_[frame]) return;
		call_of_trigger_ = frame_callback_storage_[frame];
	}

	//-------------------------------------------------------------------------------------------------------
	void SeekUnit::update(float delta_time) {
		if ( condition_ != eCondition::PLAYING ) return;
		frame_progress_time_ += delta_time * time_scale_ ;
		uint32_t frame = getSeekFrame(eFrameType::CURRENT);
		seekProcess(delta_time);
		if (frame_callback_storage_.empty()) return;
		uint32_t update_frame = getSeekFrame(eFrameType::CURRENT);

		TNL_SEEK_UNIT_LOCK_GUARD_MUTEX();
		update_frame %= total_frame_num_;
		if (update_frame != frame) {
			frame_progress_time_ = 0.0f;
			if (!call_of_trigger_ && frame_callback_storage_[update_frame]) {
				call_of_trigger_ = frame_callback_storage_[update_frame];
			}
		}
		if (!call_of_trigger_) return;
		call_of_trigger_(frame_callback_reference_);
		call_of_trigger_ = nullptr;
	}


	//-------------------------------------------------------------------------------------------------------
	void SeekUnit::setFrameTriggerCallback(int32_t frame, const std::function<void(const std::any& reference)>& callback) {
		if (frame_callback_storage_.empty()) return;
		if (frame_callback_storage_.size() <= frame) return;
		TNL_SEEK_UNIT_LOCK_GUARD_MUTEX();
		if (0 > frame) {
			for (int i = 0; i < frame_callback_storage_.size(); ++i) {
				frame_callback_storage_[i] = callback;
			}
		}
		else {
			frame_callback_storage_[frame] = callback;
		}
		uint32_t start_frame = (ePlayDir::REVERSE == play_dir_) ? total_frame_num_ - 1 : 0;
		if (start_frame == frame) call_of_trigger_ = callback;
	}

	//-------------------------------------------------------------------------------------------------------
	void SeekUnit::setFrameTriggerCallbacReference(const std::any& reference) {
		TNL_SEEK_UNIT_LOCK_GUARD_MUTEX();
		frame_callback_reference_ = reference;
	}

	//-------------------------------------------------------------------------------------------------------
	void SeekUnit::setPlayState(ePlayMode play_mode, ePlayDir play_dir) {
		if (condition_ == eCondition::PLAYING) return;
		TNL_SEEK_UNIT_LOCK_GUARD_MUTEX();
		play_dir_ = play_dir;
		play_mode_ = play_mode;
		if (ePlayDir::REVERSE == play_dir_) {
			direction_ = -1;
		}
		else {
			direction_ = 1;
		}
	}

	//-------------------------------------------------------------------------------------------------------
	void SeekUnit::setTotalTime(float total_time) { 
		if (condition_ == eCondition::PLAYING) return;
		TNL_SEEK_UNIT_LOCK_GUARD_MUTEX();
		total_time_ = total_time;
	}

	//-------------------------------------------------------------------------------------------------------
	void SeekUnit::seekProcess(float delta_time) {

		float d_delta_time = delta_time ;
		float time = seek_position_ + d_delta_time * time_scale_ * static_cast<float>(direction_);
		if (0 >= time || time >= total_time_) {
			float over_time = (0 < time) ? (time - total_time_) : time;
			if (ePlayMode::SINGLE == play_mode_) {
				{
					TNL_SEEK_UNIT_LOCK_GUARD_MUTEX();
					seek_position_ = time;
					seek_position_ = std::clamp(seek_position_, 0.0f, total_time_);
				}
				stop();
			}
			else if (ePlayMode::REPEAT == play_mode_) {
				restart();
				{
					TNL_SEEK_UNIT_LOCK_GUARD_MUTEX();
					seek_position_ += (d_delta_time * time_scale_ * static_cast<float>(direction_)) - over_time;
				}
			}
			else if (ePlayMode::REFLECTION == play_mode_) {
				TNL_SEEK_UNIT_LOCK_GUARD_MUTEX();
				direction_ *= -1;
				seek_position_ += ((total_time_ / total_frame_num_) * static_cast<float>(direction_)) - over_time;
			}
		}
		else {
			{
				TNL_SEEK_UNIT_LOCK_GUARD_MUTEX();
				seek_position_ = time;
			}
		}

	}

	//-------------------------------------------------------------------------------------------------------
	void SeekUnit::setTotalFrameNum(uint32_t frame_num) {
		TNL_SEEK_UNIT_LOCK_GUARD_MUTEX();
		if (0 == frame_num) {
			total_frame_num_ = static_cast<uint32_t>(total_time_ / (1.0 / fix_fps_));
			return;
		}
		total_frame_num_ = frame_num;
		frame_callback_storage_.resize(total_frame_num_);
	}

	//-------------------------------------------------------------------------------------------------------
	void SeekUnit::setTimeScale(float time_scale) {
		TNL_SEEK_UNIT_LOCK_GUARD_MUTEX();
		time_scale_ = time_scale;
	}

	//-------------------------------------------------------------------------------------------------------
	float SeekUnit::getSeekRateInFrame() {
		float frame_time = total_time_ / static_cast<float>(total_frame_num_);
		float frame_now = static_cast<float>(getSeekFrame(eFrameType::CURRENT));
		float rate = 0;
		if (direction_ > 0) {
			rate = (seek_position_ - (frame_now * frame_time)) / frame_time;
		}
		else {
			rate = 1.0f - ((seek_position_ - (frame_now * frame_time)) / frame_time);
		}
		rate = std::clamp<float>(rate, 0, 1.0);
		return rate;
	}

	//-------------------------------------------------------------------------------------------------------
	uint32_t SeekUnit::getSeekFrame(eFrameType e_frame) {

		uint32_t frame_now = static_cast<uint32_t>(std::floor(static_cast<float>(total_frame_num_) * getSeekRate()));
		if (eFrameType::CURRENT == e_frame) {
			if (ePlayMode::SINGLE == play_mode_) {
				if (fabs(getSeekRate() - 1.0f) <= FLT_EPSILON) return total_frame_num_ - 1;
			}
			return frame_now % total_frame_num_;
		}

		int32_t dir = (eFrameType::NEXT == e_frame) ? direction_ : -direction_;

		uint32_t frame = static_cast<int>(frame_now) + dir;
		if (total_frame_num_ <= frame || 0 > frame) {
			if (ePlayMode::SINGLE == play_mode_) {
				frame = std::clamp<int32_t>(frame, 0, total_frame_num_ - 1);
			}
			else if (ePlayMode::REPEAT == play_mode_) {
				frame %= static_cast<int>(total_frame_num_);
				frame = static_cast<uint32_t>(std::abs(static_cast<int>(frame))) * (total_frame_num_ - 1);
			}
			else if (ePlayMode::REFLECTION == play_mode_) {
				frame += -(dir * 2);
			}
		}
		return frame;
	}


}