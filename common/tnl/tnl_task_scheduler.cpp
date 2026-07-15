#include "tnl_task_scheduler.h"

namespace tnl {

	//----------------------------------------------------------------------------------------------------------------
	/// @brief シングルスレッドユースケース向けのTaskSchedulerインスタンスを生成します。
	Shared<TaskScheduler> TaskScheduler::CreateUseCaseSingleThread() {
		// 内部でprivateコンストラクタを呼び出し、シングルスレッドモードで初期化
		Shared<TaskScheduler> instance = std::shared_ptr<TaskScheduler>(new TaskScheduler(true));
		return instance;
	}

	//----------------------------------------------------------------------------------------------------------------
	/// @brief マルチスレッドユースケース向けのTaskSchedulerインスタンスを生成します。
	Shared<TaskScheduler> TaskScheduler::CreateUseCaseMultiThread() {
		// 内部でprivateコンストラクタを呼び出し、マルチスレッドモードで初期化
		Shared<TaskScheduler> instance = std::shared_ptr<TaskScheduler>(new TaskScheduler(false));
		return instance;
	}


	//----------------------------------------------------------------------------------------------------------------
	void TaskScheduler::insertTask(const Shared<Task>& task) {
		// mutexでタスクリストへのアクセスを保護
		std::lock_guard<std::mutex> lock(mutex_);

		// 優先度に基づいてタスクを挿入する位置を検索
		int32_t priority = task->priority_;
		auto insert_pos = std::find_if(tasks_.begin(), tasks_.end(),
			[priority](const Shared<Task>& t) { return t->getPriority() < priority; });
		// 検索した位置にタスクを挿入
		tasks_.insert(insert_pos, task);

		// 高速検索用のマップにもタスクを追加
		task_map_[task->getID()] = task;
	}


	//----------------------------------------------------------------------------------------------------------------
	/// @brief 新しいタスクをスケジューラに追加します。
	int64_t TaskScheduler::addTask(const std::function<void()>& task, float start_delay, float repeat_interval, int32_t repeat_frequency, int32_t priority) {

		// 引数として渡されたタスク関数が有効かチェック
		if (!task) {
			throw std::invalid_argument("Task function cannot be null");
		}
		// 新しいタスクオブジェクトを生成
		Shared<Task> new_task = std::shared_ptr<Task>(new Task(task, start_delay, repeat_interval, repeat_frequency, priority));

		// シングルスレッドモードなら update 中にタスク追加が行われた場合
		// 即座に追加するのではなく遅延追加リストへ ( update 完了を待って tasks_ に追加される )
		if (is_use_case_single_thread_ && isUpDating()) {
			delay_add_tasks_.emplace_back( new_task );
			return new_task->getID() ;
		}

		insertTask( new_task );

		// 生成したタスクのIDを返す
		return new_task->getID();
	}

	//----------------------------------------------------------------------------------------------------------------
	/// @brief 一度だけ実行されるタスクを簡単に追加するためのヘルパーメソッド。
	int64_t TaskScheduler::addOneShotTask(const std::function<void()>& task, float start_delay, int32_t priority) {
		// addTaskを呼び出し、繰り返し回数を1回に設定
		return addTask(task, start_delay, 0.0f, 1, priority);
	}

	//----------------------------------------------------------------------------------------------------------------
	/// @brief 指定したタスクに完了時コールバックを設定します。
	void TaskScheduler::finishedTaskCallback(int64_t id, const std::function<void()>& callback) {
		// スレッドセーフにマップを操作
		std::lock_guard<std::mutex> lock(mutex_);
		auto it = task_map_.find(id);
		if (it != task_map_.end()) {
			it->second->setFinishedCallback(callback);
		}
	}

	//----------------------------------------------------------------------------------------------------------------
	/// @brief 指定したタスクを終了させます。
	void TaskScheduler::killTask(int64_t id) {
		// スレッドセーフにマップを操作
		std::lock_guard<std::mutex> lock(mutex_);
		auto it = task_map_.find(id);
		if (it != task_map_.end()) {
			// タスクの終了フラグを立てる
			it->second->setKillSwitch(true);
		}
		// 次のupdateで削除処理が走るようにトリガーをセット
		onRemoveTrigger();
	}

	//----------------------------------------------------------------------------------------------------------------
	/// @brief 全てタスクを終了させます。
	void TaskScheduler::killTaskAll() {
		// スレッドセーフにマップを操作
		std::lock_guard<std::mutex> lock(mutex_);
		for (auto task : tasks_) {
			task->setKillSwitch(true);
		}
		// 次のupdateで削除処理が走るようにトリガーをセット
		onRemoveTrigger();
	}


	//----------------------------------------------------------------------------------------------------------------
	/// @brief 指定したタスクの優先度を変更します。
	void TaskScheduler::changeTaskPriority(int64_t id, int32_t priority) {
		// スレッドセーフにマップを操作
		std::lock_guard<std::mutex> lock(mutex_);
		auto it = task_map_.find(id);
		if (it != task_map_.end()) {
			// タスクの優先度を変更
			it->second->setPriority(priority);
		}
		// 次のupdateで再ソートが走るようにトリガーをセット
		onReSortTrigger();
	}

	//----------------------------------------------------------------------------------------------------------------
	/// @brief 全てのタスクの状態を更新します。
	void TaskScheduler::update(float delta_time) {

		// update処理が開始したことを示す
		is_updating_.store(true, std::memory_order_release);

		// 再ソートの必要がある場合
		if (on_resort_trigger_) {
			std::lock_guard<std::mutex> lock(mutex_);
			// 優先度の降順でリストをソート
			tasks_.sort([](const Shared<Task>& l, const Shared<Task>& r) {
				return l->getPriority() > r->getPriority();
				});
			on_resort_trigger_ = false; // トリガーをリセット
		}

		std::list<Shared<Task>> local_tasks;
		std::list<Shared<Task>>* ref_tasks = nullptr;

		// マルチスレッドモードの場合は、安全のためにタスクリストをコピーして使用
		if (!is_use_case_single_thread_) {
			std::lock_guard<std::mutex> lock(mutex_);
			local_tasks = tasks_;
			ref_tasks = &local_tasks;
		}
		// シングルスレッドモードの場合は、コピーのオーバーヘッドを避けるため元のリストを直接参照
		else {
			ref_tasks = &tasks_;
		}

		// 処理対象のタスクリストをループ
		for (auto& task : *ref_tasks) {
			// タスクを実行し、終了したかどうかを判定
			if (task->execute(delta_time)) {
				// 終了していたら、完了コールバックを取得して実行
				std::function<void()> callback = task->getFinishedCallback();
				if (callback) {
					callback();
				}
				// 削除トリガーをセット
				onRemoveTrigger();
			}
		}

		// 削除の必要がある場合
		if (on_remove_trigger_) {
			std::lock_guard<std::mutex> lock(mutex_);
			// kill_switchがtrueのタスクをリストから削除
			tasks_.remove_if([this](const Shared<Task>& task) {
				if (task->getKillSwitch()) {
					// 検索用マップからも対応するタスクを削除
					task_map_.erase(task->getID());
					return true;
				}
				return false;
				});
			on_remove_trigger_ = false; // トリガーをリセット
		}

		// update処理が完了したことを示す
		is_updating_.store(false, std::memory_order_release);

		// 遅延追加リストにタスクがあれば処理リストへ追加
		if (delay_add_tasks_.empty()) return;
		for (auto task : delay_add_tasks_) {
			insertTask(task);
		}
		delay_add_tasks_.clear();
	}

	//----------------------------------------------------------------------------------------------------------------
	/// @brief タスクの状態を更新し、実行タイミングであればタスクを実行します。
	/// @return タスクがこのフレームで終了した場合はtrueを返します。
	bool Task::execute(float delta_time) {
		// すでに終了しているか、タスク本体がなければ即時終了
		if (kill_switch_ || !task_) return true;

		// 経過時間を加算
		progress_time_ += delta_time;

		// 実行タイミングのチェック時間を決定（初回はstart_delay、以降はrepeat_interval）
		float check_time = (is_first_call_) ? start_delay_ : repeat_interval_;

		// まだ実行タイミングに達していなければ何もしない
		if (check_time > progress_time_) return false;

		// タスク本体を実行
		task_();
		is_first_call_ = false; // 初回実行フラグを倒す

		// 無限繰り返しでない場合、残り回数をデクリメント
		if (REPEAT_INFINITE != repeat_frequency_) {
			repeat_frequency_--;
		}
		// 残り回数が0になったら終了フラグを立てる
		if (0 == repeat_frequency_) kill_switch_ = true;

		// 次の実行タイミングのために経過時間を調整
		if (check_time > 0) {
			// 経過時間をチェック時間で割った余りを新しい経過時間とする
			progress_time_ = std::fmod(progress_time_, check_time);
		}
		else {
			// 毎フレーム実行のようなケースでは経過時間をリセット
			progress_time_ = 0;
		}

		// 終了フラグの状態を返す
		return kill_switch_;
	}


	//----------------------------------------------------------------------------------------------------------------
	/// @brief 設定されている完了時コールバックをスレッドセーフに取得します。
	std::function<void()> Task::getFinishedCallback() const {
		std::lock_guard<std::mutex> lock(mutex_);
		return finished_callback_;
	}

	//----------------------------------------------------------------------------------------------------------------
	/// @brief 完了時コールバックをスレッドセーフに設定します。
	void Task::setFinishedCallback(const std::function<void()>& callback) {
		std::lock_guard<std::mutex> lock(mutex_);
		finished_callback_ = callback;
	}

}
