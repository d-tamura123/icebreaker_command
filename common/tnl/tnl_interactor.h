#pragma once
#include <vector>
#include <functional>
#include <type_traits>
#include <mutex>
#include "tnl_using.h"


/* マルチスレッドを利用するプロジェクトでは tnl_build_setting.h の TNL_BUILD_SWITCH_USE_MULTI_THREDING を true に設定してください */
#if TNL_BUILD_SWITCH_USE_MULTI_THREDING
#define TNL_INTERACTOR_NOT_USER_CALL_LOCK_GUARD_MUTEX() std::lock_guard<std::mutex> lock(interactor_mutex_);
#else
#define TNL_INTERACTOR_NOT_USER_CALL_LOCK_GUARD_MUTEX()
#endif


// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// インタラクションコーディネーター ( インタラクター )
// Interaction Coordinator  ( Interactor )
//
//   
// tips1.. クラス間の疎結合通信である
//		   メディエータパターンとオブザーバーパターンの構築をサポートする機能を提供します
//
// 
// tips2.. この機能は疎結合通信を実装したいクラスへの継承を必要としません
//         そのため、利用者は疎結合通信が必要になったタイミングでの機能付加が可能で、既存の継承構造に影響を与えません
//         また、この機能は包含によって実装できるので継承ツリーの途中へ段階をまたがっての機能付加も可能です
// 
// 
// tips3.. この機能を利用するインスタンスは std::shared_ptr である必要があります        
//
//  
// tips4.. この機能には ( SAFE モード ) ( UNSAFE モード ) が存在します
//         機能内部では dynamic_pointer_cast や std::any_cast といった高コストキャストが使用されており
//         それらを解決するため、型安全性を犠牲にして高速に動作する UNSAFE モードが実装されています
//         UNSAFE モードを利用する場合は、あらかじめ( SAFE モード )で動作することを確認してから使用してください
//         ※ 高頻度な通信を必要としない限り UNSAFE モード を利用するべきではありません
//
// 
// tips5.. この機能には ユーザーが直接コールしてはならない関数・マクロが存在し、特徴は以下の通りです
//         tnlInteractorNotUserCall_ で始まる関数
//         TNL_INTERACTOR_NOT_USER_CALL_ で始まるマクロ
//         
//  
//   
// -------------------------------------------------------------------------------------------------------------------------------------------
// TNL_INTERACTOR_DECLARE マクロ
//		・他クラスへの通知機能を付与したいクラスのメンバに宣言します
//		・このマクロへの引数は通知関数へのアクセス定数となり、任意の名前の定数を可変個で設定できます
//		・通知関数へのアクセス定数は内部で enum class eTnlICId 型のパラメータとなります
// 
// 例 )
//	class Foo {
//	public:
//		Foo(){}
//
//		// 通知機能を付与
//		// Foo が他クラス( Bar, Qux )への通知を行うための ID を設定
//		TNL_INTERACTOR_DECLARE(
//			CASE_BAR_NOTICE,
//			CASE_QUX_NOTICE_A,
//			CASE_QUX_NOTICE_B
//		); 
// 
//	} ;
// 
// -------------------------------------------------------------------------------------------------------------------------------------------
// TNL_INTERACTOR_TRANSMITTER_REGISTER マクロ	
//		・tips1.. 通知機能を付与したインスタンスへ通知関数を登録します ( 通知関数へ引数を付与しないタイプです )
//		・tips2.. このマクロによって設定された通知関数は tnlInteractorTransmitter を使用してアクセスします
//		・arg1... 通知機能を付与するインスタンス ( ポインタ )
//		・arg2... 通知関数の戻り値の型 ( void 指定も可 )
//		・arg3... TNL_INTERACTOR_DECLARE へ任意に設定した定数で、それを通知関数への ID とします
//		・arg4... 通知関数で参照する弱参照オブジェクト( std::weak_ptr 限定 )
//		・arg5... 通知内容 { } で囲った内部で処理を実装します( ラムダ式の内部処理部分のみ記述してください )
//				  { } 内部では TNL_INTERACTOR_GET_WEAK_REFERENCE マクロを利用して通知先オブジェクトへの std::weak_ptr が利用できます
// 
// 例 )
// 
// std::shared_ptr<Foo> foo ;
// std::shared_ptr<Bar> bar ;
// 
// void main(){
//		foo = std::make_shared<Foo>() ;
//		bar = std::make_shared<Bar>() ;
// 
//		TNL_INTERACTOR_TRANSMITTER_REGISTER(foo, int, Foo::eTnlICId::CASE_BAR_NOTICE, Weak<Bar>(bar), {
//			Shared<Bar> ref_bar = TNL_INTERACTOR_GET_WEAK_REFERENCE(Bar).lock();
//			if(!ref_bar) return 0 ;
// 
//			/* 任意の処理 */
// 
//			// arg2 で int 型を指定したので 通知関数は int型戻り値を返す
//			return 10 ;
//		});
// 
// }
// 
// ※ TNL_INTERACTOR_UNSAFE_TRANSMITTER_REGISTER が存在し
//    そちらを使用すると UNSAFE モード になります、使い方は同じです
// 
// 
// -------------------------------------------------------------------------------------------------------------------------------------------
// TNL_INTERACTOR_ARGS_TRANSMITTER_REGISTER マクロ
//		・tips1.. 通知機能を付与したインスタンスへ通知関数を登録します ( 通知関数へ引数を付与するタイプです )
//		・tips2.. このマクロによって設定された通知関数は tnlInteractorArgsTransmitter を使用してアクセスします
//		・arg1... 通知機能を付与するインスタンス ( ポインタ )
//		・arg2... 通知関数の戻り値の型 ( void 指定も可 )
//		・arg3... TNL_INTERACTOR_DECLARE へ任意に設定した定数で、それを通知関数への ID とします
//		・arg4... 通知関数で参照する弱参照オブジェクト( std::weak_ptr 限定 )
//		・arg5... 通知関数の引数に設定する任意の型
//		・arg6... 通知内容 { } で囲った内部で処理を実装します( ラムダ式の内部処理部分のみ記述してください )
//				  { } 内部では TNL_INTERACTOR_GET_WEAK_REFERENCE マクロを利用して通知先オブジェクトへの std::weak_ptr が利用できます
//				  { } 内部では TNL_INTERACTOR_GET_RECEIPT_INFORMATION マクロを利用して呼び出し元から渡された任意型のデータを得られます
//
// 例 )
// 
// std::shared_ptr<Foo> foo ;
// std::shared_ptr<Bar> bar ;
// 
// void main(){
//		foo = std::make_shared<Foo>() ;
//		bar = std::make_shared<Bar>() ;
// 
//		TNL_INTERACTOR_ARGS_TRANSMITTER_REGISTER(foo, void, Foo::eTnlICId::CASE_BAR_NOTICE, Weak<Bar>(bar), bool, {
//			Shared<Bar> ref_bar = TNL_INTERACTOR_GET_WEAK_REFERENCE(Bar).lock();
//			if(!ref_bar) return ;
// 
//			// ここでは arg5 が bool なので tnlInteractorArgsTransmitter に渡された bool 型パラメータを得られます
//			bool receipt = TNL_INTERACTOR_GET_RECEIPT_INFORMATION() ;
// 
// 
//			/* 任意の処理 */
// 
// 
//			/* arg2 は void なので戻り値無し */
//		});
// 
// }
// 
// ※ TNL_INTERACTOR_UNSAFE_ARGS_TRANSMITTER_REGISTER が存在し
//    そちらを使用すると UNSAFE モード になります、使い方は同じです
// 		
// 
// -------------------------------------------------------------------------------------------------------------------------------------------
// TNL_INTERACTOR_GET_WEAK_REFERENCE マクロ
//		・tips1.. 通知処理内で通知先オブジェクトの std::weak_ptr を得られます
//		・tips2.. TNL_INTERACTOR_****_TRANSMITTER_REGISTER の最後に記述した通知処理内で利用できるマクロです
//		・arg1... 通知先オブジェクトの型
//		・ret.... 通知先オブジェクトの std::weak_ptr
// 
// 使用例は TNL_INTERACTOR_TRANSMITTER_REGISTER を参照
// 
// 
// -------------------------------------------------------------------------------------------------------------------------------------------
// TNL_INTERACTOR_GET_RECEIPT_INFORMATION マクロ
//		・tips1.. 通知元から渡された任意型のデータを得られます
//		・tips2.. TNL_INTERACTOR_****_TRANSMITTER_REGISTER の最後に記述した通知処理内で利用できるマクロです
//
// 使用例は TNL_INTERACTOR_ARGS_TRANSMITTER_REGISTER を参照
//  
// 
// -------------------------------------------------------------------------------------------------------------------------------------------
// tnlInteractorTransmitter< ReturnType > 関数 
//		・tips1.. 通知機能を付与したクラスへ自動定義されるテンプレート関数です
//		・tips2.. ID 登録されている通知関数へアクセスし、他のインスタンスへ通知を行います
//		・tips3.. ReturnType には通知関数からの戻り値型を指定します
//      　　　　　戻り値型は TNL_INTERACTOR_****_TRANSMITTER_REGISTER の arg2 と同じものを指定する必要があります
// 
//		・arg1... 実行する通知関数へのアクセス定数 ( eTnlICId 型 )
//		・ret.... 通知処理からの返信値 ReturnType 型
// 
// 例 )
// 
// std::shared_ptr<Foo> foo ;
// std::shared_ptr<Bar> bar ;
// 
// void main(){
//		foo = std::make_shared<Foo>() ;
//		bar = std::make_shared<Bar>() ;
// 
//		// arg2 が int 型であることに注目
//		TNL_INTERACTOR_ARGS_TRANSMITTER_REGISTER(foo, int, Foo::eTnlICId::CASE_BAR_NOTICE, Weak<Bar>(bar) {
// 
//			/* 中略 */
// 
//			// 通知処理の結果として 100 を返す
//			return 100 ;
//		});
// 
//		// テンプレート引数が int 型であることに注目
//		// value には通知処理の結果として 100 が代入される
//		int value = foo->tnlInteractorTransmitter<int>(Foo::eTnlICId::CASE_BAR_NOTICE) ;
// 
// }
// 
// -------------------------------------------------------------------------------------------------------------------------------------------
// tnlInteractorArgsTransmitter< ReturnType, ArgType > 関数
//		・tips1.. 通知機能を付与したクラスへ自動定義されるテンプレート関数です
//		・tips2.. ID 登録されている通知関数へアクセスし、他のインスタンスへ通知を行います
//		・tips3.. ReturnType には通知関数からの戻り値型を指定します
//      　　　　　戻り値型は TNL_INTERACTOR_****_TRANSMITTER_REGISTER の arg2 と同じものを指定する必要があります
//		・tips4.. ArgType には通知先への付加情報として与えたい型を指定します
//  
//		・arg1... 実行する通知関数へのアクセス定数 ( eTnlICId 型 )
//		・arg2... 通知先へ送る任意型データ ( ArgType )
//		・ret.... 通知処理からの返信値 ReturnType 型
// 
// 例 )
// 
// std::shared_ptr<Foo> foo ;
// std::shared_ptr<Bar> bar ;
// 
// void main(){
//		foo = std::make_shared<Foo>() ;
//		bar = std::make_shared<Bar>() ;
// 
//		// arg5 が bool 型であることに注目
//		// tnlInteractorArgsTransmitter の arg2 に指定される型と同じにしてください
//		TNL_INTERACTOR_ARGS_TRANSMITTER_REGISTER(foo, void, Foo::eTnlICId::CASE_BAR_NOTICE, Weak<Bar>(bar), bool, {
// 
//			/* 中略 */
// 
//		});
// 
//		// １つめのテンプレート型指定に void を設定することで戻り値なしの通知も可能
//		// ２つめのテンプレート型指定に bool を設定したことで true or false を付加情報として送れます
//		// arg2 で true を付加情報として通知を実行
//		foo->tnlInteractorArgsTransmitter<void, bool>(Foo::eTnlICId::CASE_BAR_NOTICE, true) ;
// 
// }
// 
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// 
// [ オブザーバー的な使用例 ]
// 
// // Foo は Example に監視されるサブジェクト
//class Foo {
//public:
//	Foo() {}
//
//	// オブザーバー( Example ) への通知インタラクター
//	TNL_INTERACTOR_DECLARE(
//		CASE_EXAMPLE_NOTICE
//	);
//
//	void setValue(int v) { value_ = v; }
//
//	void update();
//
//private:
//	float value_ = 0;
//};
// 
// 
// // Bar は Example に監視されるサブジェクト
//class Bar {
//public:
//	Bar() {}
//
//	// オブザーバー( Example ) への通知インタラクター
//	TNL_INTERACTOR_DECLARE(
//		CASE_EXAMPLE_NOTICE
//	);
//
//	void setValue(int v) { value_ = v; }
//
//	void update();
//
//private:
//	int value_ = 0;
//};
//
// 
// 
// // Example をオブザーバーとする
// // Foo と Bar の監視者
// class Example : public std::enable_shared_from_this<Example> {
// public :
//		Example() = default ;
// 
//		// 遅延コンストラクターを想定
//		void lazyConstruct(){
//			foo = std::make_shared<Foo>() ;
//			bar = std::make_shared<Bar>() ;
//
//			// foo インスタンスから Example インスタンスへの通知を登録
//			// foo から受け取った情報で Example が bar を操作
//			TNL_INTERACTOR_ARGS_TRANSMITTER_REGISTER(foo, void, Foo::eTnlICId::CASE_EXAMPLE_NOTICE, Weak<Example>(shared_from_this()), float, {
//				Shared<Example> example = TNL_INTERACTOR_GET_WEAK_REFERENCE(Example).lock();
//				example->bar->setValue( TNL_INTERACTOR_GET_RECEIPT_INFORMATION() ) ;
//			});
//
//			// bar インスタンスから Example インスタンスへの通知を登録
//			// bar から受け取った情報で Example が foo を操作
//			TNL_INTERACTOR_ARGS_TRANSMITTER_REGISTER(bar, void, Bar::eTnlICId::CASE_EXAMPLE_NOTICE, Weak<Example>(shared_from_this()), float, {
//				Shared<Example> example = TNL_INTERACTOR_GET_WEAK_REFERENCE(Example).lock();
//				example->foo->setValue( TNL_INTERACTOR_GET_RECEIPT_INFORMATION() ) ;
//			});
//		}
// 
//		std::shared_ptr<Foo> foo = nullptr ;
//		std::shared_ptr<Bar> bar = nullptr ;
// } ;
// 
// 
// 
// void Foo::update(){
// 
//		// Example に対して 100.0f を通知
//		tnlInteractorArgsTransmitter<void, float>( eTnlICId::CASE_EXAMPLE_NOTICE, 100.0f );
// 
// }
// 
// 
// void Bar::update(){
// 
//		// Example に対して 200.0f を通知
//		tnlInteractorArgsTransmitter<void, float>( eTnlICId::CASE_EXAMPLE_NOTICE, 200.0f );
// 
// }
// 
// 
// std::shared_ptr<Example> example = nullptr ;
// 
// void main(){
// 
//		example = std::make_shared<Example>() ;
//		example->lazyConstruct() ; // foo bar ともに value は 0
// 
//		example->foo->update() ;	// foo により example に通知が送られ bar vlaue が 200.0f になる
//		example->bar->update() ;	// bar により example に通知が送られ foo vlaue が 100.0f になる
// 
// }
// 
// 
// 
//---------------------------------------------------------------------------------------------------------------
// 
// [ メディエータ的な使用例 ]
//		今回は Foo -> Bar の通信のみ実装 ( 双方向へしたい場合は TNL_INTERACTOR_DECLARE を Bar でも記述 )
// 
// 
//class Foo {
//public:
//	Foo() {}
//
//	// Bar との交信インタラクター
//	TNL_INTERACTOR_DECLARE(
//		CASE_BAR_NOTICE
//	);
//
//	void setValue(int v) { value_ = v; }
//
//private:
//	float value_ = 0;
//};
//
//
//class Bar {
//public:
//	Bar() {}
//
//	void setValue(int v) { value_ = v; }
//
//private:
//	int value_ = 0;
//};
//
//
//std::shared_ptr<Foo> foo = nullptr;
//std::shared_ptr<Bar> bar = nullptr;
//
//void gameStart() {
//
//	foo = std::make_shared<Foo>();
//	bar = std::make_shared<Bar>();
//
//	// foo から受け取った情報で bar が自信の情報を更新
//	TNL_INTERACTOR_ARGS_TRANSMITTER_REGISTER(foo, void, Foo::eTnlICId::CASE_BAR_NOTICE, Weak<Bar>(bar), float, {
//		Shared<Bar> ref_bar = TNL_INTERACTOR_GET_WEAK_REFERENCE(Bar).lock();
//		ref_bar->setValue( TNL_INTERACTOR_GET_RECEIPT_INFORMATION() );
//	});
//
//
//	// foo から bar へ通知 bar value が 100.0f に変化
//	foo->tnlInteractorArgsTransmitter<void, float>(Foo::eTnlICId::CASE_BAR_NOTICE, 100.0f);
//
//}
// 
// 
//



namespace tnl {

	class Interactor : public std::enable_shared_from_this<Interactor> {
	public:
		enum class eSafetyMode {
			SAFETY,
			UNSAFETY
		};
		Interactor() = default;
		virtual ~Interactor() {}

		template<class T>
		T getReference() {
			if (eSafetyMode::UNSAFETY == e_safety_) {
				return (*(T*)(unsafe_reference_.get()));
			}
			return std::any_cast<T>(reference_);
		}

		void setSafetyMode(eSafetyMode safety_cast) noexcept { e_safety_ = safety_cast ; }
		eSafetyMode getSafetyMode() const noexcept { return e_safety_; }

	protected :
		eSafetyMode e_safety_ = eSafetyMode::SAFETY;
		std::any reference_;
		std::unique_ptr<void, decltype(&free)> unsafe_reference_ = std::unique_ptr<void, decltype(&free)>(nullptr, free);
	};

	template<class ReturnType>
	class ConcreteInteractor final : public Interactor {
	public:
		ConcreteInteractor
		    ( const std::function<ReturnType(const Shared<Interactor>&)>& transmitter
			, const std::any& reference )
			: transmitter_(std::move(transmitter))
		{
			reference_ = reference;
		}
		ConcreteInteractor
		    ( const std::function<ReturnType(const Shared<Interactor>&)>& transmitter
			, void* unsafe_reference )
			: transmitter_(std::move(transmitter))
		{
			unsafe_reference_ = std::unique_ptr<void, decltype(&free)>(unsafe_reference, free);
		}

		~ConcreteInteractor() {}

		template<class T>
		T contact() { return transmitter_(shared_from_this()); }
		template<>
		void contact() { transmitter_(shared_from_this()); }

	private:
		std::function<ReturnType(const Shared<Interactor>&)> transmitter_;
	};

	template<class ReturnType, class ArgType>
	class ConcreteArgumentInteractor final : public Interactor {
	public:
		ConcreteArgumentInteractor
		    ( const std::function<ReturnType(const Shared<Interactor>&, ArgType)>& transmitter
			, const std::any& reference )
			: transmitter_(std::move(transmitter))
		{
			reference_ = reference;
		}
		ConcreteArgumentInteractor
		    ( const std::function<ReturnType(const Shared<Interactor>&, ArgType)>& transmitter
			, void* unsafe_reference )
			: transmitter_(std::move(transmitter))
			
		{
			unsafe_reference_ = std::unique_ptr<void, decltype(&free)>(unsafe_reference, free);
		}

		~ConcreteArgumentInteractor() {}

		template<class T>
		T contact(ArgType receipt_information) { return transmitter_(shared_from_this(), receipt_information); }
		template<>
		void contact(ArgType receipt_information) { transmitter_(shared_from_this(), receipt_information); }

	private:
		std::function<ReturnType(const Shared<Interactor>&, ArgType)> transmitter_;
	};


	#define TNL_INTERACTOR_TRANSMITTER_REGISTER( instance, return_type, e_tnl_icid, weak_ref_object, transmitter ){												\
		TNL_INTERACTOR_NOT_USER_CALL_TRANSMITTER_REGISTER_WEAK_CHECK(weak_ref_object) ;																								\
		instance->tnlInteractorNotUserCall_TransmitterRegister<return_type>( e_tnl_icid, weak_ref_object, [](const Shared<tnl::Interactor>& interactor)->return_type transmitter ) ;	\
	}

	#define TNL_INTERACTOR_ARGS_TRANSMITTER_REGISTER( instance, return_type, e_tnl_icid, weak_ref_object, send_object_type, transmitter ){								\
		TNL_INTERACTOR_NOT_USER_CALL_TRANSMITTER_REGISTER_WEAK_CHECK(weak_ref_object) ;																								\
		instance->tnlInteractorNotUserCall_ArgsTransmitterRegister<return_type, send_object_type>( e_tnl_icid, weak_ref_object, [](const Shared<tnl::Interactor>& interactor, send_object_type receipt_information)->return_type transmitter ) ;\
	}

	#define TNL_INTERACTOR_UNSAFE_TRANSMITTER_REGISTER( instance, return_type, e_tnl_icid, weak_ref_object, transmitter ){											\
		TNL_INTERACTOR_NOT_USER_CALL_TRANSMITTER_REGISTER_WEAK_CHECK(weak_ref_object) ;																								\
		auto object = weak_ref_object ;																																	\
		size_t size = sizeof(object);																																	\
		void* unsafe_reference = malloc(size);																															\
		memcpy(unsafe_reference, &object, size);																														\
		instance->tnlInteractorNotUserCall_UnsafeTransmitterRegister<return_type>( e_tnl_icid, unsafe_reference, [](const Shared<tnl::Interactor>& interactor)->return_type transmitter ) ;	\
		instance->tnlInteractorNotUserCall_SetSafetyMode(e_tnl_icid, tnl::Interactor::eSafetyMode::UNSAFETY) ;															\
	}

	#define TNL_INTERACTOR_UNSAFE_ARGS_TRANSMITTER_REGISTER( instance, return_type, e_tnl_icid, weak_ref_object, send_object_type, transmitter ){						\
		TNL_INTERACTOR_NOT_USER_CALL_TRANSMITTER_REGISTER_WEAK_CHECK(weak_ref_object) ;																								\
		auto object = weak_ref_object ;																																	\
		size_t size = sizeof(object);																																	\
		void* unsafe_reference = malloc(size);																															\
		memcpy(unsafe_reference, &object, size);																														\
		instance->tnlInteractorNotUserCall_UnsafeArgsTransmitterRegister<return_type, send_object_type>( e_tnl_icid, unsafe_reference, [](const Shared<tnl::Interactor>& interactor, send_object_type receipt_information)->return_type transmitter ) ;\
		instance->tnlInteractorNotUserCall_SetSafetyMode(e_tnl_icid, tnl::Interactor::eSafetyMode::UNSAFETY) ;															\
	}

	#define TNL_INTERACTOR_GET_WEAK_REFERENCE(ref_type) interactor->getReference<ref_type>()

	#define TNL_INTERACTOR_GET_RECEIPT_INFORMATION() receipt_information



#define TNL_INTERACTOR_DECLARE(...)					\
public :											\
	enum class eTnlICId {							\
		__VA_ARGS__									\
		, TNL_ICID_MAX								\
	};												\
private :																																								\
	std::vector<Shared<tnl::Interactor>> tnl_interactors_ = std::vector<Shared<tnl::Interactor>>( static_cast<uint32_t>(eTnlICId::TNL_ICID_MAX) );						\
	mutable std::mutex interactor_mutex_;																																\
	template<class TnlReturnType>																																		\
	void getConcreteInteractor(Shared<tnl::ConcreteInteractor<TnlReturnType>>& medi, eTnlICId e_tnl_icid) const {														\
		{																																								\
			TNL_INTERACTOR_NOT_USER_CALL_LOCK_GUARD_MUTEX();  																														\
			Shared<tnl::Interactor> interactor = tnl_interactors_[static_cast<uint32_t>(e_tnl_icid)] ;																	\
			if (tnl::Interactor::eSafetyMode::UNSAFETY == interactor->getSafetyMode() ){																				\
				medi = static_pointer_cast<tnl::ConcreteInteractor<TnlReturnType>>(interactor);																			\
			}																																							\
			else {																																						\
				medi = dynamic_pointer_cast<tnl::ConcreteInteractor<TnlReturnType>>(interactor);																		\
			}																																							\
		}																																								\
		if (medi) return ;																																				\
		tnl::DebugTrace("tnlInteractorTransmitter error : [ %s ] dynamic pointer cast mismatch \n", typeid(TnlReturnType).name());									\
		throw std::runtime_error("tnlInteractorTransmitter error : dynamic pointer cast mismatch");																	\
	}																																									\
	template<class TnlReturnType, class TnlArgType>																														\
	void getConcreteArgumentInteractor(Shared<tnl::ConcreteArgumentInteractor<TnlReturnType, TnlArgType>>& medi, eTnlICId e_tnl_icid ) const {				\
		{																																								\
			TNL_INTERACTOR_NOT_USER_CALL_LOCK_GUARD_MUTEX() ;  																										\
			Shared<tnl::Interactor> interactor = tnl_interactors_[static_cast<uint32_t>(e_tnl_icid)] ;																	\
			if (tnl::Interactor::eSafetyMode::UNSAFETY == interactor->getSafetyMode() ){																				\
				medi = static_pointer_cast<tnl::ConcreteArgumentInteractor<TnlReturnType, TnlArgType>>(interactor);														\
			}																																							\
			else {																																						\
				medi = dynamic_pointer_cast<tnl::ConcreteArgumentInteractor<TnlReturnType, TnlArgType>>(interactor);													\
			}																																							\
		}																																								\
		if (medi) return ;																																				\
		tnl::DebugTrace("tnlInteractorArgsTransmitter error : [ return type %s ] [ argument type %s ] dynamic pointer cast mismatch \n", typeid(TnlReturnType).name(), typeid(TnlArgType).name());	\
		throw std::runtime_error("tnlInteractorArgsTransmitter error : dynamic pointer cast mismatch");																	\
	}																																									\
public :																																								\
	template<class TnlReturnType = void>																																		\
	TnlReturnType tnlInteractorTransmitter( eTnlICId e_tnl_icid ) const {																							\
		Shared<tnl::ConcreteInteractor<TnlReturnType>> medi = nullptr ;																									\
		getConcreteInteractor<TnlReturnType>(medi, e_tnl_icid) ;																										\
		if constexpr (std::is_void_v<TnlReturnType>) {																													\
			medi->contact<void>();																																		\
		}																																								\
		else {																																							\
			return medi->contact<TnlReturnType>();																														\
		}																																								\
	}																																									\
	template<class TnlReturnType, class TnlArgType>																													\
	TnlReturnType tnlInteractorArgsTransmitter(eTnlICId e_tnl_icid, TnlArgType argument) const {																		\
		Shared<tnl::ConcreteArgumentInteractor<TnlReturnType, TnlArgType>> medi = nullptr ;																			\
 		getConcreteArgumentInteractor<TnlReturnType, TnlArgType>(medi, e_tnl_icid) ;																			\
		if constexpr (std::is_void_v<TnlReturnType>) {																													\
			medi->contact<void>(argument);																																\
		}																																								\
		else {																																							\
			return medi->contact<TnlReturnType>(argument);																												\
		}																																								\
	}																																									\
	/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/ \
	/*																																								 */	\
	/* 以下ユーザーによる直接コールは禁止 */																															\
	/*																																								 */	\
	/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/ \
	void tnlInteractorNotUserCall_SetSafetyMode( eTnlICId e_tnl_icid, tnl::Interactor::eSafetyMode cast_mode ){															\
        TNL_INTERACTOR_NOT_USER_CALL_LOCK_GUARD_MUTEX() ;   																											\
		tnl_interactors_[static_cast<uint32_t>(e_tnl_icid)]->setSafetyMode( cast_mode ) ;																				\
	}																																									\
	template< class TnlReturnType >																																	\
	void tnlInteractorNotUserCall_TransmitterRegister(eTnlICId e_tnl_icid, const std::any& reference, const std::function<TnlReturnType(const Shared<tnl::Interactor>&)>& transmitter) {		\
		Shared<tnl::ConcreteInteractor<TnlReturnType>> interactor = std::make_shared<tnl::ConcreteInteractor<TnlReturnType>>(std::move(transmitter), reference);		\
        TNL_INTERACTOR_NOT_USER_CALL_LOCK_GUARD_MUTEX() ;   																											\
		tnl_interactors_[static_cast<uint32_t>(e_tnl_icid)] = interactor ;																								\
	}																																									\
	template< class TnlReturnType, class TnlArgType >																													\
	void tnlInteractorNotUserCall_ArgsTransmitterRegister(eTnlICId e_tnl_icid, const std::any& reference, const std::function<TnlReturnType(const Shared<tnl::Interactor>&, TnlArgType)>& transmitter) {						\
		Shared<tnl::ConcreteArgumentInteractor<TnlReturnType, TnlArgType>> interactor = std::make_shared<tnl::ConcreteArgumentInteractor<TnlReturnType, TnlArgType>>(std::move(transmitter), reference);		\
        TNL_INTERACTOR_NOT_USER_CALL_LOCK_GUARD_MUTEX() ;  																											\
		tnl_interactors_[static_cast<uint32_t>(e_tnl_icid)] = interactor ;																								\
	}																																									\
	template< class TnlReturnType >																																	\
	void tnlInteractorNotUserCall_UnsafeTransmitterRegister(eTnlICId e_tnl_icid, void* reference, const std::function<TnlReturnType(const Shared<tnl::Interactor>&)>& transmitter) {		\
		Shared<tnl::ConcreteInteractor<TnlReturnType>> interactor = std::make_shared<tnl::ConcreteInteractor<TnlReturnType>>(std::move(transmitter), reference);		\
        TNL_INTERACTOR_NOT_USER_CALL_LOCK_GUARD_MUTEX() ;   																											\
		tnl_interactors_[static_cast<uint32_t>(e_tnl_icid)] = interactor ;																								\
	}																																									\
	template< class TnlReturnType, class TnlArgType >																													\
	void tnlInteractorNotUserCall_UnsafeArgsTransmitterRegister(eTnlICId e_tnl_icid, void* reference, const std::function<TnlReturnType(const Shared<tnl::Interactor>&, TnlArgType)>& transmitter) {							\
		Shared<tnl::ConcreteArgumentInteractor<TnlReturnType, TnlArgType>> interactor = std::make_shared<tnl::ConcreteArgumentInteractor<TnlReturnType, TnlArgType>>(std::move(transmitter), reference);		\
        TNL_INTERACTOR_NOT_USER_CALL_LOCK_GUARD_MUTEX() ;   																											\
		tnl_interactors_[static_cast<uint32_t>(e_tnl_icid)] = interactor ;																								\
	}																																									\

	#define TNL_INTERACTOR_NOT_USER_CALL_TRANSMITTER_REGISTER_WEAK_CHECK(weak_ref_object) {																							\
		if(!tnl::IsWeakPointer(weak_ref_object)){																														\
			tnl::DebugTrace("TNL_INTERACTOR_REGISTER error : [ %s ] is not weak pointer \n", #weak_ref_object);															\
			throw std::runtime_error("TNL_INTERACTOR_REGISTER error : argument is not weak pointer");																	\
		}																																								\
	}

}

