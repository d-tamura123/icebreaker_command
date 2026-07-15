#include <memory>
#include "tnl_vector3.h"
#include "tnl_intersect.h"
#include "tnl_math.h"
#include "tnl_matrix.h"
#include "tnl_quaternion.h"

namespace tnl {

	// 点と矩形の衝突判定
	bool IsIntersectPointRect(const int point_x, const int point_y, const int rect_x, const int rect_y, const int rect_size_x, const int rect_size_y) {
		int sx = rect_x - (rect_size_x >> 1);
		int sy = rect_y - (rect_size_y >> 1);
		int ex = sx + rect_size_x;
		int ey = sy + rect_size_y;
		if (point_x < sx) return false;
		if (point_x > ex) return false;
		if (point_y < sy) return false;
		if (point_y > ey) return false;
		return true;
	}

	// 矩形と矩形
	bool IsIntersectRectPrimitive(int a_left, int a_right, int a_top, int a_bottom,
		int b_left, int b_right, int b_top, int b_bottom) {
		if (a_right < b_left) return false;
		if (a_left > b_right) return false;
		if (a_top > b_bottom) return false;
		if (a_bottom < b_top) return false;
		return true;
	}
	bool IsIntersectRectPrimitive(float a_left, float a_right, float a_top, float a_bottom,
		float b_left, float b_right, float b_top, float b_bottom) {
		if (a_right < b_left) return false;
		if (a_left > b_right) return false;
		if (a_top > b_bottom) return false;
		if (a_bottom < b_top) return false;
		return true;
	}


	// 矩形と矩形
	bool IsIntersectRect(int a_rect_x, int a_rect_y, int a_rect_size_w, int a_rect_size_h,
		int b_rect_x, int b_rect_y, int b_rect_size_w, int b_rect_size_h) {

		int a_right = a_rect_x + (a_rect_size_w >> 1);
		int a_left = a_rect_x - (a_rect_size_w >> 1);
		int a_top = a_rect_y - (a_rect_size_h >> 1);
		int a_bottom = a_rect_y + (a_rect_size_h >> 1);

		int b_right = b_rect_x + (b_rect_size_w >> 1);
		int b_left = b_rect_x - (b_rect_size_w >> 1);
		int b_top = b_rect_y - (b_rect_size_h >> 1);
		int b_bottom = b_rect_y + (b_rect_size_h >> 1);

		return IsIntersectRectPrimitive(a_left, a_right, a_top, a_bottom, b_left, b_right, b_top, b_bottom);
	}
	bool IsIntersectRect(float a_rect_x, float a_rect_y, float a_rect_size_w, float a_rect_size_h,
		float b_rect_x, float b_rect_y, float b_rect_size_w, float b_rect_size_h) {

		float a_right = a_rect_x + (a_rect_size_w * 0.5f);
		float a_left = a_rect_x - (a_rect_size_w * 0.5f);
		float a_top = a_rect_y - (a_rect_size_h * 0.5f);
		float a_bottom = a_rect_y + (a_rect_size_h * 0.5f);

		float b_right = b_rect_x + (b_rect_size_w * 0.5f);
		float b_left = b_rect_x - (b_rect_size_w * 0.5f);
		float b_top = b_rect_y - (b_rect_size_h * 0.5f);
		float b_bottom = b_rect_y + (b_rect_size_h * 0.5f);

		return IsIntersectRectPrimitive(a_left, a_right, a_top, a_bottom, b_left, b_right, b_top, b_bottom);
	}


	// work... 矩形と円の衝突判定
	bool IsIntersectRectCircle(const tnl::Vector2f& rect_center, const tnl::Vector2f& rect_size, const tnl::Vector2f& circle_pos, float circle_radius) {
		tnl::Vector2f np = tnl::GetNearestRectPoint(rect_center, (float)rect_size.x, (float)rect_size.y, circle_pos);
		if ((np - circle_pos).length() < circle_radius) {
			return true;
		}
		return false;
	}

	// work... 線分と円の衝突判定
	bool IsIntersectLineCircle(const tnl::Vector2f& line_s, const tnl::Vector2f& line_e, const tnl::Vector2f& circle_pos, float radius) {
		tnl::Vector3 nearest = tnl::GetNearestPointLine(tnl::Vector3(circle_pos), tnl::Vector3(line_s), tnl::Vector3(line_e));
		float dist = (tnl::Vector3(circle_pos) - nearest).length();
		return dist < radius;
	}

	// work... AABB と AABBの判定
	bool IsIntersectAABB(const tnl::Vector3& a, const tnl::Vector3& a_size, const tnl::Vector3& b, const tnl::Vector3& b_size)
	{
		tnl::Vector3 a_max = tnl::ToMaxAABB(a, a_size);
		tnl::Vector3 a_min = tnl::ToMinAABB(a, a_size);
		tnl::Vector3 b_max = tnl::ToMaxAABB(b, b_size);
		tnl::Vector3 b_min = tnl::ToMinAABB(b, b_size);
		if (a_max.x < b_min.x || a_min.x > b_max.x) return false;
		if (a_max.y < b_min.y || a_min.y > b_max.y) return false;
		if (a_max.z < b_min.z || a_min.z > b_max.z) return false;
		return true;
	}


	//----------------------------------------------------------------------------------------------
	bool IsIntersectSphere(const tnl::Vector3& a, float a_radius, const tnl::Vector3& b, float b_radius) {
		float l = (a - b).length();
		return l < (a_radius + b_radius) ? true : false;
	}

	//----------------------------------------------------------------------------------------------
	bool IsIntersectLinePlane(const tnl::Vector3 &line_start, const tnl::Vector3 &line_end, const tnl::Vector3 &plane_pos, const tnl::Vector3 &plane_normal, tnl::Vector3 *intersect_position) {

		if (1 == GetSidesPointAndPlane(line_start, plane_normal, plane_pos) && 1 == GetSidesPointAndPlane(line_end, plane_normal, plane_pos)) return false;
		if (-1 == GetSidesPointAndPlane(line_start, plane_normal, plane_pos) && -1 == GetSidesPointAndPlane(line_end, plane_normal, plane_pos)) return false;

		tnl::Vector3 ln = line_end - line_start;
		tnl::Vector3 pa = line_start - plane_pos;
		tnl::Vector3 pb = line_end - plane_pos;
		float na = fabsf(pa.dot(plane_normal));
		float nb = fabsf(pb.dot(plane_normal));

		if(intersect_position) *intersect_position = line_start + ln * (na / (na + nb));

		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool IsIntersectLine2D(const Vector3& p1, const Vector3& p2, const Vector3& p3, const Vector3& p4)
	{

		// x 座標によるチェック
		if (p1.x >= p2.x) {
			if ((p1.x < p3.x && p1.x < p4.x) || (p2.x > p3.x && p2.x > p4.x)) {
				return false;
			}
		}
		else {
			if ((p2.x < p3.x && p2.x < p4.x) || (p1.x > p3.x && p1.x > p4.x)) {
				return false;
			}
		}

		// y 座標によるチェック
		if (p1.y >= p2.y) {
			if ((p1.y < p3.y && p1.y < p4.y) || (p2.y > p3.y && p2.y > p4.y)) {
				return false;
			}
		}
		else {
			if ((p2.y < p3.y && p2.y < p4.y) || (p1.y > p3.y && p1.y > p4.y)) {
				return false;
			}
		}

		if (((p1.x - p2.x) * (p3.y - p1.y) + (p1.y - p2.y) * (p1.x - p3.x)) * ((p1.x - p2.x) * (p4.y - p1.y) + (p1.y - p2.y) * (p1.x - p4.x)) > 0) return false;
		if (((p3.x - p4.x) * (p1.y - p3.y) + (p3.y - p4.y) * (p3.x - p1.x)) * ((p3.x - p4.x) * (p2.y - p3.y) + (p3.y - p4.y) * (p3.x - p2.x)) > 0) return false;

		return true;
	}


	//----------------------------------------------------------------------------------------------
	bool IsIntersectLineTriangle2D(const Vector3& v1, const Vector3& v2, const Vector3& v3, const Vector3& s, const Vector3& e)
	{
		if (IsIntersectLine2D(v1, v2, s, e)) return true;
		if (IsIntersectLine2D(v2, v3, s, e)) return true;
		if (IsIntersectLine2D(v3, v1, s, e)) return true;
		if (IsIntersectTrianglePoint(v1, v2, v3, s)) return true;
		if (IsIntersectTrianglePoint(v1, v2, v3, e)) return true;
		return false;
	}


	//-----------------------------------------------------------------------------------------------------
	bool IsIntersectSphereAABB(const tnl::Vector3& sphere_pos, float sphere_r, const tnl::Vector3& aabb_max, const tnl::Vector3& aabb_min) {
		if (tnl::eParallelRegionPtAABB::INSIDE == tnl::GetParallelRegionPointAndAABB(sphere_pos, aabb_max, aabb_min)) {
			return true;
		}

		tnl::Vector3 np = tnl::GetNearestPointAABB(sphere_pos, aabb_max, aabb_min);
		if ((np - sphere_pos).length() < sphere_r) return true;

		return false;
	}

	//-----------------------------------------------------------------------------------------------------
	bool IsIntersectSphereOBB(const tnl::Vector3& sphere_pos, float sphere_r, const tnl::Vector3& obb_pos, const tnl::Vector3& obb_size, const tnl::Quaternion& obb_rot) {
		tnl::Vector3 lp = sphere_pos - obb_pos;
		tnl::Vector3 ip = tnl::Vector3::InverseTransformCoord(lp, obb_rot) + obb_pos;
		tnl::Vector3 obb_max = tnl::ToMaxAABB(obb_pos, obb_size);
		tnl::Vector3 obb_min = tnl::ToMinAABB(obb_pos, obb_size);
		return IsIntersectSphereAABB(ip, sphere_r, obb_max, obb_min);
	}

	//----------------------------------------------------------------------------------------------
	bool IsIntersectTrianglePoint(const Vector3& v1, const Vector3& v2, const Vector3& v3, const Vector3& p)
	{
		Vector3 a = v1 - p;
		Vector3 b = v2 - p;
		Vector3 c = v3 - p;

		float ab = Vector3::Dot(a, b);
		float ac = Vector3::Dot(a, c);
		float bc = Vector3::Dot(b, c);
		float cc = Vector3::Dot(c, c);

		if (0.0f > ((bc * ac) - (cc * ab))) return false;
		float bb = Vector3::Dot(b, b);
		if (0.0f > ((ab * bc) - (ac * bb))) return false;

		return true;
	}


	//----------------------------------------------------------------------------------------------
	bool IsIntersectLineTriangle(const Vector3& v1, const Vector3& v2, const Vector3& v3, const Vector3& s, const Vector3& e)
	{

		Vector3 ab = v2 - v1;
		Vector3 ac = v3 - v1;
		Vector3 pq = s - e;

		Vector3 n = Vector3::Cross(ab, ac);

		float d = Vector3::Dot(pq, n);
		if (d <= 0.0f) return false;

		Vector3 ap = s - v1;
		float t = Vector3::Dot(ap, n);
		if (t < 0.0f) return false;
		if (t > d) return false;

		Vector3 e_ = Vector3::Cross(pq, ap);
		float v = Vector3::Dot(ac, e_);
		if (v < 0.0f || v > d) return false;
		float w = -Vector3::Dot(ab, e_);
		if (w < 0.0f || v + w > d) return false;

		return true;
	}

	//-------------------------------------------------------------------------------------------------
	bool IsIntersectRaySphere(
		const tnl::Vector3& p
		, const tnl::Vector3& d
		, const tnl::Vector3& sc
		, float sr
		, float* pt
		, tnl::Vector3* intersect_point) {

		float t = 0;
		tnl::Vector3 q;
		tnl::Vector3 m = p - sc;
		float b = tnl::Vector3::Dot(m, d);
		float c = tnl::Vector3::Dot(m, m) - sr * sr;
		if (c > 0.0f && b > 0.0f) return false;
		float discr = b * b - c;
		if (discr < 0.0f) return false;
		t = -b - sqrt(discr);

		if (t < 0.0f) t = 0.0f;
		q = p + d * t;

		if (pt) *pt = t;
		if (intersect_point) *intersect_point = q;
		return true;
	}

	//-------------------------------------------------------------------------------------------------
	bool IsIntersectLineSphere(
		const tnl::Vector3& s
		, const tnl::Vector3& e
		, const tnl::Vector3& sc
		, float sr
		, float* pt
		, tnl::Vector3* intersect_point) {

		tnl::Vector3 p = s;
		tnl::Vector3 d = (e - s) / (e - s).length();
		float t = 0;
		tnl::Vector3 q;

		if (!IsIntersectRaySphere(p, d, sc, sr, &t, &q)) return false;
		if (t > (e - s).length()) return false;

		if (pt) *pt = t / (e - s).length();
		if (intersect_point) *intersect_point = q;
		return true;
	}


	//-------------------------------------------------------------------------------------------------
	// 線分と円柱の衝突判定
	bool IsIntersectLineCylinder(
		const tnl::Vector3& sa
		, const tnl::Vector3& sb
		, const tnl::Vector3& p
		, const tnl::Vector3& q
		, float r
		, float* pt) {

		float t = 0.0f;
		tnl::Vector3 d = q - p, m = sa - p, n = sb - sa;
		float md = tnl::Vector3::Dot(m, d);
		float nd = tnl::Vector3::Dot(n, d);
		float dd = tnl::Vector3::Dot(d, d);

		if (md < 0.0f && md + nd < 0.0f) return false;
		if (md > dd && md + nd > dd) return false;
		float nn = tnl::Vector3::Dot(n, n);
		float mn = tnl::Vector3::Dot(m, n);
		float a = dd * nn - nd * nd;
		float k = tnl::Vector3::Dot(m, m) - r * r;
		float c = dd * k - md * md;

		if (fabs(a) < FLT_EPSILON) {
			if (c > 0.0f) return false;
			if (md < 0.0f) t = -mn / nn;
			else if (md > dd) t = (nd - mn) / nn;
			else t = 0.0f;
			if (pt != nullptr) *pt = t;
			return true;
		}
		float b = dd * mn - nd * md;
		float discr = b * b - a * c;
		if (discr < 0.0f) return false;
		t = (-b - sqrt(discr)) / a;
		if (t < 0.0f || t > 1.0f) return false;

		if (md + t * nd < 0.0f) {
			if (nd <= 0.0f) return false;
			t = -md / nd;
			if (k + 2 * t * (mn + t * nn) <= 0.0f) {
				if (pt != nullptr) *pt = t;
				return true;
			}
			else return false;
		}
		else if (md + t * nd > dd) {
			if (nd >= 0.0f) return false;
			t = (dd - md) / nd;
			if (k + dd - 2 * md + t * (2 * (mn - nd) + t * nn) <= 0.0f) {
				if (pt != nullptr) *pt = t;
				return true;
			}
			else return false;
		}
		if (pt != nullptr) *pt = t;
		return true;
	}


	//-------------------------------------------------------------------------------------------------
	bool IsIntersectLineCapsule(
		const tnl::Vector3& sa
		, const tnl::Vector3& sb
		, const tnl::Vector3& p
		, const tnl::Vector3& q
		, float r
		, float* pt) {

		float t = 0;
		if (IsIntersectLineCylinder(sa, sb, p, q, r, &t)) {
			if (pt != nullptr) *pt = t;
			return true;
		}
		if (IsIntersectLineSphere(sa, sb, p, r, &t)) {
			if (pt != nullptr) *pt = t;
			return true;
		}
		if (IsIntersectLineSphere(sa, sb, q, r, &t)) {
			if (pt != nullptr) *pt = t;
			return true;
		}
		return false;
	}

	//----------------------------------------------------------------------------------------------
	bool IsIntersectRayAABB(const Vector3& pos, const Vector3& dir, const Vector3& aabb_max, const Vector3& aabb_min, float *pt, Vector3* intersect_pos, Vector3* intersect_surface_nml) {

		float tmin = 0.0f;
		float tmax = FLT_MAX;
		float p[3] = { pos.x, pos.y, pos.z };
		float d[3] = { dir.x, dir.y, dir.z };
		float max[3] = { aabb_max.x, aabb_max.y, aabb_max.z };
		float min[3] = { aabb_min.x, aabb_min.y, aabb_min.z };
		for (int i = 0; i < 3; ++i) {
			if (fabs(d[i]) < FLT_EPSILON) {
				if (p[i] < min[i] || p[i] > max[i]) return false;
			}
			else {
				float ood = 1.0f / d[i];
				float t1 = (min[i] - p[i]) * ood;
				float t2 = (max[i] - p[i]) * ood;
				if (t1 > t2) std::swap(t1, t2);
				if (t1 > tmin) tmin = t1;
				if (t2 < tmax) tmax = t2;
				if (tmin > tmax) return false;
			}
		}
		if (pt) *pt = tmin;

		tnl::Vector3 ip = tnl::Vector3(p[0], p[1], p[2]) + (tnl::Vector3(d[0], d[1], d[2]) * tmin);
		if (intersect_pos) { *intersect_pos = ip; }
		if (intersect_surface_nml) {
			*intersect_surface_nml = GetDiagonalRegionSurfaceNormalAABB({ ip.x, ip.y, ip.z }, aabb_max, aabb_min);
		}

		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool IsIntersectRayOBB(const Vector3& pos, const Vector3& dir, const Vector3& obb_pos, const Vector3& obb_size, const Quaternion& obb_rot, Vector3* intersect_pos, Vector3* intersect_surface_nml) {

		tnl::Vector3 lp = pos - obb_pos;
		tnl::Vector3 ip = tnl::Vector3::InverseTransformCoord(lp, obb_rot);
		tnl::Vector3 idir = tnl::Vector3::InverseTransformCoord(dir, obb_rot) ;
		tnl::Vector3 obb_max = tnl::ToMaxAABB({ 0, 0, 0 }, obb_size);
		tnl::Vector3 obb_min = tnl::ToMinAABB({ 0, 0, 0 }, obb_size);

		float t = 0.0f;
		tnl::Vector3 intersect;
		tnl::Vector3 surface_nml;
		if (!IsIntersectRayAABB(ip, idir, obb_max, obb_min, &t, &intersect, &surface_nml)) {
			return false;
		}

		intersect = tnl::Vector3::TransformCoord(intersect, obb_rot) + obb_pos ;

		//surface_nml = tnl::Vector3::TransformCoord(surface_nml, obb_rot);
		surface_nml = GetDiagonalRegionSurfaceNormalOBB(intersect, obb_pos, obb_size, obb_rot);

/*
		Vector3 aabb_max = tnl::ToMaxAABB(obb_pos, obb_size);
		Vector3 aabb_min = tnl::ToMinAABB(obb_pos, obb_size);

		Matrix m = obb_rot.getMatrix();

		// 光線を境界ボックスの空間へ移動
		Matrix invMat = Matrix::Inverse(m);

		Vector3 p_l, dir_l;
		p_l = Vector3::TransformCoord(pos, invMat);
		invMat._41 = 0.0f;
		invMat._42 = 0.0f;
		invMat._43 = 0.0f;
		dir_l = Vector3::TransformCoord(dir, invMat);

		// 交差判定
		float p[3], d[3], min[3], max[3];
		memcpy(p, &p_l, sizeof(Vector3));
		memcpy(d, &dir_l, sizeof(Vector3));
		memcpy(min, &aabb_min, sizeof(Vector3));
		memcpy(max, &aabb_max, sizeof(Vector3));

		float t = -FLT_MAX;
		float t_max = FLT_MAX;

		for (int i = 0; i < 3; ++i) {
			if (fabs(d[i]) < FLT_EPSILON) {
				if (p[i] < min[i] || p[i] > max[i])
					return false; // 交差していない
			}
			else {
				// スラブとの距離を算出
				// t1が近スラブ、t2が遠スラブとの距離
				float odd = 1.0f / d[i];
				float t1 = (min[i] - p[i]) * odd;
				float t2 = (max[i] - p[i]) * odd;
				if (t1 > t2) {
					float tmp = t1; t1 = t2; t2 = tmp;
				}

				if (t1 > t) t = t1;
				if (t2 < t_max) t_max = t2;

				// スラブ交差チェック
				if (t >= t_max)
					return false;
			}
		}
		*/

		// 交点
		if (intersect_pos) { *intersect_pos = intersect; }
		if (intersect_surface_nml) { *intersect_surface_nml = surface_nml; }

		return true;
	}


	//----------------------------------------------------------------------------------------------
	bool IsIntersectLineAABB(const Vector3& s, const Vector3& e, const Vector3& aabb_max, const Vector3& aabb_min, float* pt, Vector3* intersect_pos, Vector3* intersect_surface_nml) {
		float t = 0;
		tnl::Vector3 intersect;
		tnl::Vector3 intersect_nml;
		if (!IsIntersectRayAABB(s, Vector3::Normalize(e - s), aabb_max, aabb_min, &t, &intersect, &intersect_nml)) {
			return false;
		}
		if ((intersect - s).length() > (e - s).length()) return false;
		if (pt) *pt = t / (e-s).length() ;
		if (intersect_pos) *intersect_pos = intersect;
		if (intersect_surface_nml) *intersect_surface_nml = intersect_nml;

		return true;
	}


	//----------------------------------------------------------------------------------------------
	bool IsIntersectLineOBB(const Vector3& s, const Vector3& e, const Vector3& obb_pos, const Vector3& obb_size, const Quaternion& obb_rot, Vector3* intersect_pos, Vector3* intersect_surface_nml) {

		Vector3 aabb_max = tnl::ToMaxAABB(obb_pos, obb_size);
		Vector3 aabb_min = tnl::ToMinAABB(obb_pos, obb_size);

		tnl::Vector3 intersect;
		tnl::Vector3 intersect_nml;
		if (!IsIntersectRayOBB(s, Vector3::Normalize(e - s), obb_pos, obb_size, obb_rot, &intersect, &intersect_nml)) {
			return false;
		}

		tnl::Vector3 np = GetNearestPointLine(intersect, s, e);
		if ((np - intersect).length() > 0.01f) return false;

		if (intersect_pos) *intersect_pos = intersect;
		if (intersect_surface_nml) *intersect_surface_nml = intersect_nml;
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool IsIntersectLineOBB2D(const Vector3& s, const Vector3& e, const Vector3& pos, const Vector3& obb_pos, const Vector3& obb_size, const Quaternion& obb_rot, Vector3* intersect_pos){
		tnl::Vector3 ss = pos - s;
		tnl::Vector3 ee = pos - e;
		return tnl::IsIntersectLineOBB(ss, ee, obb_pos, obb_size, obb_rot, intersect_pos) ;
	}

	//-------------------------------------------------------------------------------------------------
	bool IsIntersectCapsulePlane(const tnl::Vector3& cap_st, const tnl::Vector3& cap_en, float cap_r, const tnl::Vector3& plane_pos, const tnl::Vector3& plane_nml, float* t ) {

		tnl::Vector3 v = cap_en - cap_st;
		if (v.length() < FLT_EPSILON) return false;

		float pd = tnl::Vector3::Dot(plane_nml, plane_pos);
		float dist = tnl::Vector3::Dot(plane_nml, cap_st) - pd;

		if (fabs(dist) <= cap_r) {
			if (t) *t = 0.0f;
			return true;
		}
		else {
			float denom = tnl::Vector3::Dot(plane_nml, v);
			if (denom * dist >= 0.0f) {
				return false;
			}
			else {
				float r = cap_r * (dist > 0.0f ? 1.0f : -1.0f);
				float p = (r - dist) / denom;
				if (p < -FLT_EPSILON || p > 1.0f + FLT_EPSILON) return false;
				if (t) *t = p;
				return true;
			}
		}

	}


	//-------------------------------------------------------------------------------------------------
	bool IsIntersectCapsuleSphere(
		const tnl::Vector3& cap_st
		, const tnl::Vector3& cap_en
		, float cap_r
		, const tnl::Vector3& sph_pos
		, float sph_r
		, float* t) {

		if (tnl::IsIntersectSphere(cap_st, cap_r, sph_pos, sph_r)) {
			if (t) *t = 0.0f;
			return true;
		}

		tnl::Vector3 np = tnl::GetNearestPointLine(sph_pos, cap_st, cap_en);

		if ((np - sph_pos).length() < cap_r + sph_r) {
			if (t) {
				np = tnl::GetNearestPointRay(sph_pos, cap_st, tnl::Vector3::Normalize(cap_en - cap_st));
				float hypot = cap_r + sph_r;
				float height = (np - sph_pos).length();
				float base = tnl::RightTriangleBaseByHeightHypot(height, hypot);
				tnl::Vector3 p = np + tnl::Vector3::Normalize(cap_st - np) * base;
				*t = (cap_st - p).length() / (cap_st - cap_en).length();
			}
			return true;
		}
		return false;
	}

	//-------------------------------------------------------------------------------------------------
	static tnl::Vector3 Corner(const tnl::Vector3& aabb_max, const tnl::Vector3& aabb_min, int n) {
		tnl::Vector3 p;
		p.x = ((n & 1) ? aabb_max.x : aabb_min.x);
		p.y = ((n & 2) ? aabb_max.y : aabb_min.y);
		p.z = ((n & 4) ? aabb_max.z : aabb_min.z);
		return p;
	}
	bool IsIntersectCapsuleAABB(
		const tnl::Vector3& cap_st
		, const tnl::Vector3& cap_en
		, const float cap_r
		, const tnl::Vector3& aabb_max
		, const tnl::Vector3& aabb_min
		, float* pt) {

		tnl::Vector3 d = cap_en - cap_st;
		tnl::Vector3 e_min = aabb_min;
		tnl::Vector3 e_max = aabb_max;
		e_min.x -= cap_r; e_min.y -= cap_r; e_min.z -= cap_r;
		e_max.x += cap_r; e_max.y += cap_r; e_max.z += cap_r;

		float t = 0;
		tnl::Vector3 p;
		if (!tnl::IsIntersectLineAABB(cap_st, cap_en, e_max, e_min, &t, &p) || t > 1.0f) {
			return false;
		}

		int32_t u = 0, v = 0;
		if (p.x < aabb_min.x) u |= 1;
		if (p.x > aabb_max.x) v |= 1;
		if (p.y < aabb_min.y) u |= 2;
		if (p.y > aabb_max.y) v |= 2;
		if (p.z < aabb_min.z) u |= 4;
		if (p.z > aabb_max.z) v |= 4;

		int32_t m = u + v;
		if (m == 7) {
			float tmin = FLT_MAX;
			if (tnl::IsIntersectLineCapsule(cap_st, cap_en, Corner(aabb_max, aabb_min, v), Corner(aabb_max, aabb_min, v ^ 1), cap_r, &t))
				tmin = std::min(t, tmin);
			if (tnl::IsIntersectLineCapsule(cap_st, cap_en, Corner(aabb_max, aabb_min, v), Corner(aabb_max, aabb_min, v ^ 2), cap_r, &t))
				tmin = std::min(t, tmin);
			if (tnl::IsIntersectLineCapsule(cap_st, cap_en, Corner(aabb_max, aabb_min, v), Corner(aabb_max, aabb_min, v ^ 4), cap_r, &t))
				tmin = std::min(t, tmin);
			if (tmin == FLT_MAX) return false;
			t = tmin;
			if (pt) *pt = t;
			return true;
		}

		if ((m & (m - 1)) == 0) {
			if (pt) *pt = t;
			return true;
		}
		if (tnl::IsIntersectLineCapsule(cap_st, cap_en, Corner(aabb_max, aabb_min, u ^ 7), Corner(aabb_max, aabb_min, v), cap_r, &t)) {
			if (pt) *pt = t;
			return true;
		}
		return false;
	}

	//-------------------------------------------------------------------------------------------------
	bool IsIntersectCapsuleOBB(
		const tnl::Vector3& cap_st
		, const tnl::Vector3& cap_en
		, const float cap_r
		, const tnl::Vector3& obb_center
		, const tnl::Vector3& obb_size
		, const tnl::Quaternion& obb_rot
		, float* pt ) {

		tnl::Vector3 s = cap_st - obb_center;
		tnl::Vector3 e = cap_en - obb_center;
		s = tnl::Vector3::InverseTransformCoord(s, obb_rot);
		e = tnl::Vector3::InverseTransformCoord(e, obb_rot);

		return tnl::IsIntersectCapsuleAABB(s, e, cap_r, tnl::ToMaxAABB({ 0, 0, 0 }, obb_size), tnl::ToMinAABB({ 0, 0, 0 }, obb_size), pt);
	}

	//----------------------------------------------------------------------------------------------
	bool IsIntersectRayTorus(const Vector3& s, const Vector3& dir, const Vector3& tp, const Quaternion& tq, const float tube_r, const float swept_r) {

		tnl::Vector3 o = tnl::Vector3::TransformCoord(s-tp, tq);
		tnl::Vector3 d = tnl::Vector3::TransformCoord(dir, tq);

		double ox = o.x;
		double oy = o.y;
		double oz = o.z;

		double dx = d.x;
		double dy = d.y;
		double dz = d.z;

		// define the coefficients of the quartic equation
		double sum_d_sqrd = dx * dx + dy * dy + dz * dz;
		double e = ox * ox + oy * oy + oz * oz -
			swept_r * swept_r - tube_r * tube_r;
		double f = ox * dx + oy * dy + oz * dz;
		double four_a_sqrd = 4.0 * swept_r * swept_r;

		std::vector<double> coeffs = {
			e * e - four_a_sqrd * (tube_r * tube_r - oy * oy),
				4.0 * f * e + 2.0 * four_a_sqrd * oy * dy,
				2.0 * sum_d_sqrd * e + 4.0 * f * f + four_a_sqrd * dy * dy,
				4.0 * sum_d_sqrd * f,
				sum_d_sqrd * sum_d_sqrd };

		std::vector<double> solution = Solve4(coeffs);

		// ray misses the torus
		if (solution.empty()) return false;
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool IsIntersectRayTorus(const Vector3& s, const Vector3& dir, const Vector3& tp, const Quaternion& tq, const float tube_r, const float swept_r, Vector3* intersect_pos) {

		tnl::Vector3 o = tnl::Vector3::TransformCoord(s - tp, tq);
		tnl::Vector3 d = tnl::Vector3::TransformCoord(dir, tq);

		double ox = o.x;
		double oy = o.y;
		double oz = o.z;

		double dx = d.x;
		double dy = d.y;
		double dz = d.z;

		// define the coefficients of the quartic equation
		double sum_d_sqrd = dx * dx + dy * dy + dz * dz;
		double e = ox * ox + oy * oy + oz * oz -
			swept_r * swept_r - tube_r * tube_r;
		double f = ox * dx + oy * dy + oz * dz;
		double four_a_sqrd = 4.0 * swept_r * swept_r;

		std::vector<double> coeffs = {
			e * e - four_a_sqrd * (tube_r * tube_r - oy * oy),
				4.0 * f * e + 2.0 * four_a_sqrd * oy * dy,
				2.0 * sum_d_sqrd * e + 4.0 * f * f + four_a_sqrd * dy * dy,
				4.0 * sum_d_sqrd * f,
				sum_d_sqrd * sum_d_sqrd };

		std::vector<double> solution = Solve4(coeffs);

		// ray misses the torus
		if (solution.empty()) return false;

		double mint = DBL_MAX;
		for (auto t : solution) {
			if ((t > DBL_EPSILON ) && (t < mint)) {
				mint = t;
			}
		}

		*intersect_pos = ( s + dir * (float)mint );
		return true;
	}


}