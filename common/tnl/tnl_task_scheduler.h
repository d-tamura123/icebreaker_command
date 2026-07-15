#pragma once
#include <mutex>
#include "tnl_using.h"
#include "tnl_util.h"

namespace tnl {

	class Task; // Taskクラスの前方宣言

	/// @brief 複数のタスクのスケジューリングを管理するクラス。
	///
	/// タスクの追加、削除、優先度変更などを行い、updateメソッドで各タスクの状態を更新します。
	/// シングルスレッド環境とマルチスレッド環境の両方に対応したユースケースを提供します。
	class TaskScheduler final {
	public:
		/// @brief デストラクタ
		~TaskScheduler() = default;

		/// @brief シングルスレッド環境向けのTaskSchedulerインスタンスを生成します。
		/// @details このモードでは、update中のタスクリストのコピーが省略され、パフォーマンスが向上します。
		/// @return TaskSchedulerの共有ポインタ
		static Shared<TaskScheduler> CreateUseCaseSingleThread();

		/// @brief マルチスレッド環境向けのTaskSchedulerインスタンスを生成します。
		/// @details このモードでは、update実行時にタスクリストの安全なコピーを作成するため、
		///          別スレッドからいつでもタスクの追加や変更が可能です。
		/// @return TaskSchedulerの共有ポインタ
		static Shared<TaskScheduler> CreateUseCaseMultiThread();

		/// @brief 新しいタスクをスケジューラに追加します。
		/// @param[in] task 実行する処理本体（std::functionオブジェクト）。
		/// @param[in] start_delay 最初の実行まで待機する時間（秒）。
		/// @param[in] repeat_interval 2回目以降の実行間隔（秒）。
		/// @param[in] repeat_frequency 繰り返し回数。Task::REPEAT_INFINITEで無限回。
		/// @param[in] priority タスクの優先度。数値が大きいほど優先度が高い。
		/// @return 生成されたタスクのユニークID。
		int64_t addTask(const std::function<void()>& task, float start_delay, float repeat_interval, int32_t repeat_frequency = 1, int32_t priority = 0);

		/// @brief 一度だけ実行されるタスク（ワンショットタスク）を追加します。
		/// @param[in] task 実行する処理本体（std::functionオブジェクト）。
		/// @param[in] start_delay 実行まで待機する時間（秒）。
		/// @param[in] priority タスクの優先度。
		/// @return 生成されたタスクのユニークID。
		int64_t addOneShotTask(const std::function<void()>& task, float start_delay, int32_t priority = 0);

		/// @brief タスク完了時に呼び出されるコールバック関数を設定します。
		/// @param[in] id 対象タスクのユニークID。
		/// @param[in] callback タスク完了時に実行するコールバック関数。
		void finishedTaskCallback(int64_t id, const std::function<void()>& callback);

		/// @brief 登録されているタスクの数を取得します
		/// @return タスク数
		int32_t getTaskNum() { return static_cast<int32_t>( tasks_.size() ); }

		/// @brief 指定したIDのタスクを強制終了させます。
		/// @details タスクは次のupdateタイミングでリストから削除されます。
		/// @param[in] id 対象タスクのユニークID。
		void killTask(int64_t id);

		/// @brief 全てのタスクを強制終了させます。
		/// @details タスクは次のupdateタイミングでリストから削除されます。
		void killTaskAll();

		/// @brief 指定したIDのタスクの優先度を変更します。
		/// @details 優先度リストは次のupdateタイミングで再ソートされます。
		/// @param[in] id 対象タスクのユニークID。
		/// @param[in] priority 新しい優先度。
		void changeTaskPriority(int64_t id, int32_t priority);

		/// @brief 全ての管理下タスクの状態を更新します。このメソッドを毎フレーム呼び出してください。
		/// @param[in] delta_time 前回updateを呼び出してからの経過時間（秒）。
		void update(float delta_time);

		/// @brief update処理が完了しているかどうかを返します。
		/// @details update中かどうかを外部から安全に確認するために使用します。
		/// @return update処理が実行中ならtrue そうでなければ false
		bool isUpDating() { return is_updating_.load(std::memory_order_acquire); }

	private:
		/// @brief デフォルトコンストラクタ
		TaskScheduler() = default;

		/// @brief ユースケースを指定して初期化するコンストラクタ。
		/// @param is_use_case_single_thread シングルスレッドモードならtrue。
		TaskScheduler(bool is_use_case_single_thread) : is_use_case_single_thread_(is_use_case_single_thread) {}

		/// @brief タスク削除が必要になったことを示すフラグを立てます。
		void onRemoveTrigger() { on_remove_trigger_ = true; }

		/// @brief タスクリストの再ソートが必要になったことを示すフラグを立てます。
		void onReSortTrigger() { on_resort_trigger_ = true; }

		void insertTask( const Shared<Task>& task );

		mutable std::mutex mutex_; ///< タスクリストやマップへのアクセスを保護するミューテックス。
		bool on_resort_trigger_ = false; ///< 再ソートが必要かどうかのフラグ。
		bool on_remove_trigger_ = false; ///< 削除処理が必要かどうかのフラグ。
		bool is_use_case_single_thread_ = false; ///< シングルスレッドモードで動作しているかのフラグ。
		std::atomic<bool> is_updating_ = false; ///< update処理実行中かどうかアトミックなフラグ。
		std::list<Shared<Task>> tasks_; ///< 優先度順にソートされたタスクのリスト。
		std::list<Shared<Task>> delay_add_tasks_;
		std::unordered_map<int64_t, Shared<Task>> task_map_; ///< タスクIDからタスクを高速に検索するためのマップ。
};


	/// @brief スケジューラによって管理される個々のタスクを表すクラス。
	class Task final {
	public:
		/// @brief タスクを無限に繰り返すことを示す定数。
		static constexpr int32_t REPEAT_INFINITE = -1;

		/// @brief デフォルトコンストラクタ
		Task() = default;
		/// @brief デストラクタ
		~Task() = default;

		/// @brief タスクを初期化するコンストラクタ。
		/// @param[in] task 実行する処理本体。
		/// @param[in] start_delay 最初の実行までの待機時間（秒）。
		/// @param[in] repeat_interval 2回目以降の実行間隔（秒）。
		/// @param[in] repeat_frequency 繰り返し回数。
		/// @param[in] priority タスクの優先度。
		Task(const std::function<void()>& task, float start_delay, float repeat_interval = 0.0f,
			int32_t repeat_frequency = 1, int32_t priority = 0)
			: task_(task)
			, start_delay_(start_delay)
			, repeat_interval_(repeat_interval)
			, repeat_frequency_(repeat_frequency)
			, priority_(priority)
			, uuid_(generateUUID()) {}

	private:
		// TaskSchedulerのみがタスクの内部状態を操作できるようにfriend宣言
		friend class TaskScheduler;

		bool execute(float delta_time);
		int64_t getID() { return uuid_; };
		TNL_PROPERTY_ATOMIC(std::atomic<bool>, KillSwitch, kill_switch_);
		TNL_PROPERTY_ATOMIC(std::atomic<int32_t>, Priority, priority_);

		std::function<void()> getFinishedCallback() const;
		void setFinishedCallback(const std::function<void()>& callback);

		mutable std::mutex mutex_; ///< 完了時コールバック関数へのアクセスを保護するミューテックス。
		int64_t uuid_ = 0; ///< タスクを一位に識別するID。
		std::atomic<int32_t> repeat_frequency_ = 1; ///< 残りの繰り返し回数。
		std::atomic<int32_t> priority_ = 0; ///< タスクの優先度。
		float progress_time_ = 0; ///< 前回の実行からの経過時間。
		float start_delay_ = 0; ///< 開始遅延時間。
		float repeat_interval_ = 0; ///< 繰り返し間隔。
		bool is_first_call_ = true; ///< 初回実行かどうかを判定するフラグ。
		std::atomic<bool> kill_switch_ = false; ///< タスクを終了させるためのフラグ。
		std::function<void()> task_; ///< 実行されるタスクの本体。
		std::function<void()> finished_callback_; ///< タスク完了時に呼び出される関数。

		/// @brief UUIDを生成するための静的カウンタ。
		inline static std::atomic<int64_t> next_uuid_;
		/// @brief スレッドセーフにユニークなIDを生成します。
		/// @return 新しいユニークID。
		inline static int64_t generateUUID() {
			return next_uuid_++; // アトミックにインクリメント
		}
	};

}
