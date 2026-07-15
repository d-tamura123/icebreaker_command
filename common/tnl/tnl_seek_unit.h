#pragma once
#include <vector>
#include <algorithm>
#include <functional>
#include <mutex>
#include "tnl_using.h"
#include "tnl_shared_factory.h"

namespace tnl {

	/* マルチスレッドを利用するプロジェクトでは tnl_build_setting.h の TNL_BUILD_SWITCH_USE_MULTI_THREDING を true に設定してください */
#if TNL_BUILD_SWITCH_USE_MULTI_THREDING
#define TNL_SEEK_UNIT_LOCK_GUARD_MUTEX() std::lock_guard<std::mutex> lock(mutex_);
#else
#define TNL_SEEK_UNIT_LOCK_GUARD_MUTEX()
#endif


	class SeekUnit final {
	private :
		SeekUnit() = default;
	public:
		~SeekUnit() {}

		enum class ePlayMode {
			SINGLE,		// 単発再生
			REPEAT,		// リピート再生
			REFLECTION	// 反転再生( リピート )
		};
		enum class ePlayDir {
			FORWARD,	// 通常
			REVERSE		// 逆再生
		};
		enum class eCondition {
			STANDBY,	// 待機中
			PLAYING,	// 再生中
			ENDED		// 再生完了
		};
		enum class eFrameType {
			PREV,		// 前のフレーム
			CURRENT,	// 現在のフレーム
			NEXT		// 次のフレーム
		};

		//-----------------------------------------------------------------------------------------------------------------
		// arg1... 固定フレームレート
		// arg2... 全体の再生時間
		// arg3... 総フレーム数
		// arg4... ePlayMode
		// arg5... ePlayDir
		// tips... 総フレーム数に 0 を設定するとフレーム管理ではなく時間の割合で管理されるものと想定します
		//         そのため総フレーム数に 0 を設定した場合 メソッド名に Frame と付いているものは非推奨です
		static Shared<SeekUnit> Create( float fix_fps, uint32_t total_frame_num, float total_time, ePlayMode play_mode, ePlayDir play_dir ) ;


		//-----------------------------------------------------------------------------------------------------------------
		// 再生
		// tips... 一時停止に実行した場合は停止した場所からの再生になります
		// tips... 単発再生で最後まで再生された後に実行すると始めからの再生になります
		void play();

		//-----------------------------------------------------------------------------------------------------------------
		// 一時停止
		void pause();

		//-----------------------------------------------------------------------------------------------------------------
		// リスタート( 始めから再生 )
		void restart();

		//-----------------------------------------------------------------------------------------------------------------
		// シーク場所を 0 ～ 1.0 の割合で指定した場所へ飛ばす
		void jumpSeekRate(float seek_rate);

		//-----------------------------------------------------------------------------------------------------------------
		// シーク場所を フレーム番号 で設定した場所へ飛ばす
		void jumpSeekFrame(uint32_t frame);

		//-----------------------------------------------------------------------------------------------------------------
		// アップデート処理( 毎フレーム実行してください )
		void update(float delta_time);


		//-------------------------------------------------------------------------------------------------------
		//
		// setter
		//
		// 

		// 再生ステート設定
		void setPlayState(ePlayMode play_mode, ePlayDir play_dir);

		// 全体のスピードスケールを設定
		void setTimeScale(float time_scale);

		// フレームの再生開始時に１度だけ実行されるコールバックを設定
		// arg1... フレーム番号
		// arg2... コールバック
		void setFrameTriggerCallback(int32_t frame, const std::function<void(const std::any&)>& callback);

		// setFrameTriggerCallback のコールバック関数で参照するオブジェクトを設定する
		void setFrameTriggerCallbacReference(const std::any& reference);


		//-------------------------------------------------------------------------------------------------------
		//
		// getter
		//

		// 現在の状態を取得
		eCondition getCondition() { return condition_; }

		// 総フレーム数を取得
		inline uint32_t getTotalFrameNum() { return total_frame_num_; }

		// 現在再生中のフレーム番号を取得
		uint32_t getSeekFrame(eFrameType e_frame = eFrameType::CURRENT);

		// 現在のシークポジションを 0 ～ 1.0 の割合で取得
		float getSeekRate() { 
			if (total_time_ <= FLT_EPSILON) return 0.0f; 
			return seek_position_ / total_time_; 
		}

		// 全体の再生時間を設定
		void setTotalTime(float total_time);
		float getTotalTime() { return total_time_; }

		// 現在のフレームのフレームあたりの再生時間の割合
		float getSeekRateInFrame();

		// フレーム更新からの経過時間
		float getFrameProgressTime() { return frame_progress_time_; }

	private:
		mutable std::mutex mutex_;
		float fix_fps_ = 60.0f;
		float time_scale_ = 1.0f;
		float total_time_ = 0;
		float seek_position_ = 0;
		float frame_progress_time_ = 0;
		int32_t direction_ = 1;
		uint32_t total_frame_num_ = 0;
		std::function<void(const std::any&)> call_of_trigger_ = nullptr;
		std::vector<std::function<void(const std::any&)>> frame_callback_storage_;
		ePlayMode play_mode_ = ePlayMode::SINGLE;
		ePlayDir play_dir_ = ePlayDir::FORWARD;
		eCondition condition_ = eCondition::STANDBY;
		std::any frame_callback_reference_;

		// シーク処理
		void seekProcess(float delta_time);

		// 総フレーム数の設定
		void setTotalFrameNum(uint32_t frame_num = 0);

		// 停止
		void stop();

	};


}

