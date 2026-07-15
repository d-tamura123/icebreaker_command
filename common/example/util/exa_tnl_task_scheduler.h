#if 0

//---------------------------------------------------------------------------------------------------------------
//
//
// 
//					[ タスクスケジューラ使用サンプル ]
//  
// 
// ※ タスクシステムとは？
//    仮想関数によるポリモーフィズムは、共通のベースクラスを継承していることが前提ですが、
//    タスクシステムはクラスに縛られず、関数単位で処理とデータをセットで登録・実行できる仕組みです。
//    これにより、動的に柔軟な処理の登録や並列実行が可能になります。
//
//　　例えば、仮想関数での多態性は "何をするか" をオブジェクト指向で表現しますが、
//　　タスクシステムは "何をするか" を関数単位で管理し、柔軟にスケジュール・実行できます。
//　　特にゲームエンジンでは、並列処理や依存関係管理のためにタスクシステムが使われます。
//
/*
	  特性            | 仮想関数ポリモーフィズム     | タスクシステム
	------------------+------------------------------+-------------------------------
	  基本構造        | クラス継承 + 仮想関数        | 関数オブジェクト（ラムダ等）
	  実行単位        | オブジェクトのメソッド       | 関数単位の処理ブロック
	  呼び出し方法    | virtual関数で動的ディスパッチ| 登録した関数を直接呼び出し
	  データの保持    | メンバ変数                   | クロージャのキャプチャ
	  拡張性          | クラス階層に依存             | 処理単位で自由に追加可能
	  並列処理適性    | △（明示的な対応が必要）     | ◎（スケジューラで制御可能）
	  型への依存      | ◎（共通の基底型が必要）     | ○（型を超えて登録可能）
	  処理の抽象化    | オブジェクト指向             | データ指向・関数型に近い

	※ 備考：
		仮想関数は「型による多態性」を提供、
		タスクシステムは「処理単位の多様性・柔軟性」を提供します。
		両者は目的が異なるため、用途に応じて使い分けるのが理想です。
*/
// 
//  
//---------------------------------------------------------------------------------------------------------------

#include <time.h>
#include <string>
#include <dxe.h>
#include "../ResourceConstantHedder.h"
#include "gm_main.h"

class Foo {
public:
	Foo();

	int sound_handle_ = 0;
	int value_ = 0;

	// タスクスケジューラ登録時に得られるタスクID 記憶用
	// スケジューラから任意のタスクを削除する時に使用します
	// 個別のタスクを削除する予定がなければこれらの記憶用変数は必要ありません
	int64_t task_id_update_ = 0;
	int64_t task_id_test_ = 0;

	// 毎フレームタスクによって実行される関数
	void update();

	// スケジューラに登録されてから２秒後に１秒間隔で３回実行される関数
	void test();
};

Foo::Foo() {
	sound_handle_ = LoadSoundMem(FILE_PATH_MP3_SE_SAMPLE_1);
}

void Foo::update() {
	value_++;
	DrawFormatString(100, 100, -1, "[ foo update ] value = %d", value_);
}

void Foo::test() {
	PlaySoundMem(sound_handle_, DX_PLAYTYPE_BACK);
}


Shared<Foo> f;
Shared<tnl::TaskScheduler> sche;

//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {
	/* DxLib_Init() はシステム側で行われているので必要ありません */
	srand(time(0));

	/*
	【使用フロー】
		[1] タスクスケジューラを作成
		[2] 任意の関数やメソッドを登録（addTask / addOneShotTask）
		[3] 毎フレーム スケジューラの update() を呼ぶ
		[4] 条件に応じてタスクを削除（killTask / killTaskAll）
	*/
	sche = tnl::TaskScheduler::CreateUseCaseSingleThread();

	f = std::make_shared<Foo>();

	// タスクスケジューラへメソッドインスタンスを追加
	// 追加できるメソッドのシグネチャは void() のみになります
	f->task_id_update_ = sche->addTask(std::bind(&Foo::update, f), 0, 0, tnl::Task::REPEAT_INFINITE);
	f->task_id_test_ = sche->addTask(std::bind(&Foo::test, f), 2.0f, 1.0f, 3);

}


//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {

	if (tnl::Input::IsKeyDownTrigger(eKeys::KB_RETURN)) {

		// インスタンスメソッドではなく、単に「処理のみ」タスクに追加する方法
		// この追加関数の場合は１秒後に１度だけ実行されます
		sche->addOneShotTask([]() {

			DrawCircle(DXE_WINDOW_WIDTH_HALF, DXE_WINDOW_HEIGHT_HALF, 100, dxe::Colors::Yellow);

			}, 1.0f);

	}

	// 特定のタスクを削除
	if (tnl::Input::IsKeyDownTrigger(eKeys::KB_Z)) {
		sche->killTask(f->task_id_update_);
	}

	// 全てのタスクを削除
	if (tnl::Input::IsKeyDownTrigger(eKeys::KB_X)) {
		sche->killTaskAll();
	}

	// タスクスケジューラアップデート
	sche->update(dxe::GetDeltaTime());


	DrawFormatString(0, 0, -1, "登録されているタスク数 : %d", sche->getTaskNum());
	DrawFormatString(0, 20, -1, "Enter : １秒後に実行されるタスクを追加");
	DrawFormatString(0, 40, -1, "Z : 毎フレーム実行されているタスクを削除");
	DrawFormatString(0, 60, -1, "X : 全てのタスクを削除");

	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });
}


//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {
	/* DxLib_End() はシステム側で終了処理が行われるので必要ありません */
}


#endif