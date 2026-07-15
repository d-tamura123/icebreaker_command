#include "tnl_math.h"
#include "tnl_vector2i.h"
#include "tnl_vector3.h"
#include "tnl_vector2f.h"

namespace tnl {

	using namespace DirectX;

	//------------------------------------------------------------------------------------------------------------------------
	Vector2i::Vector2i(const tnl::Vector3& v) noexcept : DirectX::XMINT2((int)v.x, (int)v.y) {}
	//------------------------------------------------------------------------------------------------------------------------
	Vector2i::Vector2i(const tnl::Vector2f& v) noexcept : DirectX::XMINT2((int)v.x, (int)v.y) {}


}