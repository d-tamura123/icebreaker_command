#pragma once
#include "tnl_build_setting.h"
#ifdef TNL_BUILD_SWITCH_DIRECTX12
#include <wrl.h>
template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
#endif

#include <memory>
template<class T> using Shared = std::shared_ptr<T>;
template<class T> using Weak = std::weak_ptr<T>;

namespace tnl {

// スマートポインタの判別用テンプレート
template<typename T>
struct is_smart_pointer : std::false_type {};

template<typename T>
struct is_smart_pointer<std::shared_ptr<T>> : std::true_type {};

template<typename T>
struct is_smart_pointer<std::unique_ptr<T>> : std::true_type {};


template<typename T>
struct is_weak_pointer : std::false_type {};

template<typename T>
struct is_weak_pointer<std::weak_ptr<T>> : std::true_type {};


}
