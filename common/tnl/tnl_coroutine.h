#pragma once
#include <mutex>
#include <functional>
#include "tnl_using.h"
#include "../boost/preprocessor.hpp"

//-----------------------------------------------------------------------------------------------------------------------------
//
// 疑似コルーチンシステム - tnl::Coroutine
//
// このクラスとマクロ群は、C++20 以前の言語仕様や Visual Studio の制限下でも
// コルーチンに似た振る舞いを実現するための擬似的なコルーチンフレームワークです。
// 主にゲームやリアルタイムアプリケーションにおいて、状態を保持したまま
// 処理を時間的に分割して実行する用途に適しています。
//
// 使用例：
//
// class Foo {
// public:
//     TNL_COROUTINE(funcA, funcB); // コルーチン対象関数名を列挙
//     void funcA();
//     void funcB();
// };
//
// void Foo::funcA() {
//     TNL_CO_BEGIN(funcA);
//     TNL_CO_TIME_YIELD_RETURN(1.0f, deltaTime, [](){ /* 1秒間処理 */ });
//     TNL_CO_END();
// }
//
// ゲームループで毎フレーム funcA() を呼び出すと、内部で状態が保存・再開されます。
// 
//-----------------------------------------------------------------------------------------------------------------------------

namespace tnl {

	class Coroutine final {
	public:
		Coroutine() = default;
		~Coroutine() = default;

		// ------------------------------------------------------------------------------------
		// マクロ展開の補助関数：引数リストを文字列化（BOOST使用）
		// ------------------------------------------------------------------------------------
		#define TNL_CO_TO_STRINGIZE(r, data, elem) BOOST_PP_STRINGIZE(elem)
		#define TNL_CO_STRINGIZE_ARGS(...) \
					BOOST_PP_SEQ_TRANSFORM(TNL_CO_TO_STRINGIZE, _, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

		// ------------------------------------------------------------------------------------
		// マクロ：コルーチンマップを生成（引数名ごとに Shared<Coroutine> を登録）
		// ------------------------------------------------------------------------------------
		#define TNL_CO_MAKE_CHASH(...) \
			[]() -> std::unordered_map<std::string_view, Shared<tnl::Coroutine>> { \
				std::unordered_map<std::string_view, Shared<tnl::Coroutine>> m; \
				constexpr const char* arr[] = { BOOST_PP_SEQ_ENUM(TNL_CO_STRINGIZE_ARGS(__VA_ARGS__)) }; \
				for (size_t i = 0; i < sizeof(arr)/sizeof(arr[0]); ++i) { \
					m[arr[i]] = std::make_shared<tnl::Coroutine>(); \
				} \
				return m; \
			}()

		// ------------------------------------------------------------------------------------
		// クラス内で使用：指定した関数群のための Coroutine オブジェクト群を保持
		// ------------------------------------------------------------------------------------
		#define TNL_COROUTINE(...) \
				Shared<tnl::Coroutine> tnl_current_coroutine_ = nullptr ;\
				std::unordered_map<std::string_view, Shared<tnl::Coroutine>> tnl_co_chash_ = TNL_CO_MAKE_CHASH(__VA_ARGS__);\


		// ------------------------------------------------------------------------------------
		// コルーチン処理開始マクロ：該当Coroutineを現在のものとしてロック＋選択
		// ------------------------------------------------------------------------------------
		#define TNL_CO_BEGIN( func_name ) {										\
			if(tnl_co_chash_.find(#func_name) == tnl_co_chash_.end() ){			\
				throw std::runtime_error("tnl coroutine not found");			\
			}																	\
			std::lock_guard<std::mutex> lock(tnl_co_chash_[#func_name]->mutex_);\
			if(tnl_current_coroutine_ != tnl_co_chash_[#func_name] ){			\
				tnl_current_coroutine_ = tnl_co_chash_[#func_name] ;			\
			}

		// ------------------------------------------------------------------------------------
		// コルーチン処理終了マクロ：呼び出し回数リセットして return
		// tips.. コルーチン使用メソッドの最後に必ずコールすること
		// ------------------------------------------------------------------------------------
		#define TNL_CO_END()				tnl_current_coroutine_->_reset_call_count_(); } return ;


		// コルーチン実行フレーム数の取得
		#define TNL_CO_PROG_FRAME()			tnl_current_coroutine_->_get_prog_frame_()

		// コルーチン実行時間の取得
		#define TNL_CO_PROG_TIME()			tnl_current_coroutine_->_get_prog_time_()

		// 当該コルーチンの破棄
		#define TNL_CO_BREAK()				{ tnl_current_coroutine_->_break_() ; return ; }


		// ------------------------------------------------------------------------------------
		// コルーチン内部処理実装マクロ( フレーム数指定 )
		// arg1... 実行フレーム数 (マイナスの値で無限ループ)
		// arg2... デルタタイム
		// arg3... コルーチンで実行するユーザ定義処理( void() ラムダ式 )
		// tips... 無限ループを指定した場合でも内部で経過フレーム数をカウントしています
		// ....... フレーム数カウントは int32_t を超えると 0 にリセットされることに注意
		// ------------------------------------------------------------------------------------
		#define TNL_CO_FRAME_YIELD_RETURN( lim_frame, delta_time, logic )		\
			if (tnl_current_coroutine_->_yield_by_frame_( lim_frame, delta_time, logic ) ) return ;

		// ------------------------------------------------------------------------------------
		// コルーチン内部処理実装マクロ( 時間指定 )
		// arg1... 実行時間
		// arg2... デルタタイム
		// arg3... コルーチンで実行するユーザ定義処理( void() ラムダ式 )
		// tips... 時間指定した場合でも内部で経過フレーム数をカウントしています
		// ....... フレーム数カウントは int32_t を超えると 0 にリセットされることに注意
		// ------------------------------------------------------------------------------------
		#define TNL_CO_TIME_YIELD_RETURN( lim_time, delta_time, logic )		\
			if (tnl_current_coroutine_->_yield_by_time_( lim_time, delta_time, logic ) ) return ;


		// コルーチン内部(TNL_CO_FRAME_YIELD_RETURN もしくは TNL_CO_TIME_YIELD_RETURN) の最初のフレームなら true
		#define TNL_CO_YIELD_IS_START()			( 0 == tnl_current_coroutine_->_get_prog_frame_() )
		// コルーチン内部(TNL_CO_FRAME_YIELD_RETURN) の最後のフレームなら true
		#define TNL_CO_FRAME_YIELD_IS_END()		( tnl_current_coroutine_->_get_limit_frame_() == (tnl_current_coroutine_->_get_prog_frame_()+1) )
		// コルーチン内部(TNL_CO_TIME_YIELD_RETURN) の最後のフレームなら true
		#define TNL_CO_TIME_YIELD_IS_END()		( tnl_current_coroutine_->_get_limit_time_() <= tnl_current_coroutine_->_get_prog_time_() )


		// ------------------------------------------------------------------------------------
		// 各種リセット用マクロ：単体、インスタンス、ポインタ経由、全体
		// ------------------------------------------------------------------------------------
		#define TNL_CO_RESET(func_name)									\
		{																\
			if(tnl_co_chash_.find(#func_name) == tnl_co_chash_.end() ){ \
				throw std::runtime_error("tnl coroutine not found");	\
			}															\
			std::lock_guard<std::mutex> lock(tnl_co_chash_[#func_name]->mutex_);\
			tnl_co_chash_[#func_name]->_reset_() ;\
		}

		#define TNL_CO_RESET_FROM_INSTANCE(instance, func_name)			\
		{																\
			if(instance.tnl_co_chash_.find(#func_name) == instance.tnl_co_chash_.end() ){ \
				throw std::runtime_error("tnl coroutine not found");	\
			}															\
			std::lock_guard<std::mutex> lock(instance.tnl_co_chash_[#func_name]->mutex_);\
			instance.tnl_co_chash_[#func_name]->_reset_() ;\
		}

		#define TNL_CO_RESET_FROM_POINTER(pointer, func_name)			\
		{																\
			if(pointer->tnl_co_chash_.find(#func_name) == pointer->tnl_co_chash_.end() ){ \
				throw std::runtime_error("tnl coroutine not found");	\
			}															\
			std::lock_guard<std::mutex> lock(pointer->tnl_co_chash_[#func_name]->mutex_);\
			pointer->tnl_co_chash_[#func_name]->_reset_() ;\
		}

		#define TNL_CO_RESET_ALL()										\
		{																\
			for(auto co : tnl_co_chash_){								\
				std::lock_guard<std::mutex> lock(co->mutex_);			\
				co->_reset()_;											\
			}															\
		}

		#define TNL_CO_RESET_ALL_FROM_INSTANCE( instance )				\
		{																\
			for(auto it : instance.tnl_co_chash_){						\
				std::lock_guard<std::mutex> lock(it.second->mutex_);	\
				it.second->_reset_();									\
			}															\
		}

		#define TNL_CO_RESET_ALL_FROM_POINTER( instance )				\
		{																\
			for(auto it : pointer->tnl_co_chash_){						\
				std::lock_guard<std::mutex> lock(it.second->mutex_);	\
				it.second->_reset()_;									\
			}															\
		}





		//------------------------------------------------------------------------------------------------------------------------
		// 
		// 
		// ユーザによる直接コールは禁止
		//
		// 
		inline void		_break_() { is_break_ = true; }
		inline int32_t	_get_prog_frame_() { return frame_; }
		inline int32_t  _get_limit_frame_() { return limit_frame_; }
		inline float	_get_prog_time_() { return time_; }
		inline float	_get_limit_time_() { return limit_time_; }
		inline void		_reset_call_count_() { call_count_ = 0; }
		inline bool		_yield_by_frame_(int32_t limit_frame, float delta_time, const std::function<void()>& logic) {
			limit_frame_ = limit_frame;
			if (call_count_++ == call_through_) {
				int32_t limit = (0 > limit_frame) ? INT32_MAX : limit_frame;
				if (frame_ >= limit) return true;
				call_count_ = 0;
				time_ += delta_time;
				logic();
				frame_++;
				if (limit_frame < 0 && INT32_MAX == frame_) frame_ = 0;
				if (limit_frame < 0 && !is_break_) return true;
				if (frame_ < limit && !is_break_) return true;
				call_through_++;
				frame_ = 0;
				time_ = 0;
				is_break_ = false;
				return true;
			}
			return false;
		}
		inline bool		_yield_by_time_(float limit_time, float delta_time, const std::function<void()>& logic) {
			limit_time_ = limit_time;
			if (call_count_++ == call_through_) {
				if (time_ >= limit_time) return true;
				call_count_ = 0;
				time_ += delta_time;
				logic();
				frame_++;
				if (INT32_MAX == frame_) frame_ = 0;
				if (time_ < limit_time && !is_break_) return true;
				call_through_++;
				frame_ = 0;
				time_ = 0;
				is_break_ = false;
				return true;
			}
			return false;
		}
		inline void		_reset_() {
			call_count_ = 0;
			call_through_ = 0;
			frame_ = 0;
			time_ = 0;
			is_break_ = false;
		}

		mutable std::mutex mutex_;
	private:
		uint32_t call_count_ = 0;
		uint32_t call_through_ = 0;
		int32_t frame_ = 0;
		int32_t limit_frame_ = 0;
		float time_ = 0;
		float limit_time_ = 0;
		bool is_break_ = false;
	};
}
