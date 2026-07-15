#include "tnl_math.h"
#include "tnl_vector2f.h"
#include "tnl_vector3.h"
#include "tnl_vector2i.h"

namespace tnl {

	using namespace DirectX;

	//------------------------------------------------------------------------------------------------------------------------
	Vector2f::Vector2f(const tnl::Vector3& v) noexcept : DirectX::XMFLOAT2(v.x, v.y) {}
	//------------------------------------------------------------------------------------------------------------------------
	Vector2f::Vector2f(const tnl::Vector2i& v) noexcept : DirectX::XMFLOAT2((float)v.x, (float)v.y) {}


	//------------------------------------------------------------------------------------------------------------------------
	Vector2f Vector2f::UniformLerp(const Vector2f& s, const Vector2f& e, float time_limit, float ct) noexcept {
		Vector3 s3 = { s.x, s.y, 0 };
		Vector3 e3 = { e.x, e.y, 0 };
		Vector3 r = Vector3::UniformLerp(s3, e3, time_limit, ct);
		return { r.x, r.y };
	}

	//------------------------------------------------------------------------------------------------------------------------
	Vector2f Vector2f::SmoothLerp(const Vector2f& s, const Vector2f& e, float time_limit, float ct, int strength) noexcept {
		Vector3 s3 = { s.x, s.y, 0 };
		Vector3 e3 = { e.x, e.y, 0 };
		Vector3 r = Vector3::SmoothLerp(s3, e3, time_limit, ct, strength);
		return { r.x, r.y };
	}

	//------------------------------------------------------------------------------------------------------------------------
	Vector2f Vector2f::AccelLerp(const Vector2f& s, const Vector2f& e, float time_limit, float ct, int strength) noexcept {
		Vector3 s3 = { s.x, s.y, 0 };
		Vector3 e3 = { e.x, e.y, 0 };
		Vector3 r = Vector3::AccelLerp(s3, e3, time_limit, ct, strength);
		return { r.x, r.y };
	}
	//------------------------------------------------------------------------------------------------------------------------
	Vector2f Vector2f::DecelLerp(const Vector2f& s, const Vector2f& e, float time_limit, float ct) noexcept {
		Vector3 s3 = { s.x, s.y, 0 };
		Vector3 e3 = { e.x, e.y, 0 };
		Vector3 r = Vector3::DecelLerp(s3, e3, time_limit, ct);
		return { r.x, r.y };
	}


}