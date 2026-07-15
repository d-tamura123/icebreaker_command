#pragma once
#include "tnl_matrix.h"
#include "tnl_vector2f.h"

namespace tnl {

	//-----------------------------------------------------------------------------------------------------
	// 点と矩形(正方形)のの衝突検知
	// arg1...点のｘ座標
	// arg2...点のｙ座標
	// arg3...矩形の中心ｘ座標
	// arg4...矩形の中心ｙ座標
	// arg5...矩形のサイズ X
	// arg5...矩形のサイズ Y
	// ret....[ 衝突している : true ]   [ 衝突していない : false ]
	bool IsIntersectPointRect(const int point_x, const int point_y, const int rect_x, const int rect_y, const int rect_size_x, const int rect_size_y);

	//-----------------------------------------------------------------------------------------------------
	// 矩形と矩形の衝突検知 ※矩形の左上・右下の頂点座標指定
	// arg1...矩形Aの左のｘ座標
	// arg2...矩形Aの右のｘ座標
	// arg3...矩形Aの上のｙ座標
	// arg4...矩形Aの下のｙ座標
	// arg5...矩形Bの左のｘ座標
	// arg6...矩形Bの右のｘ座標
	// arg7...矩形Bの上のｙ座標
	// arg8...矩形Bの下のｙ座標
	// ret....[ 衝突している : true ]   [ 衝突していない : false ]
	bool IsIntersectRectPrimitive(int a_left, int a_right, int a_top, int a_bottom,
		int b_left, int b_right, int b_top, int b_bottom);
	bool IsIntersectRectPrimitive(float a_left, float a_right, float a_top, float a_bottom,
		float b_left, float b_right, float b_top, float b_bottom);

	//-----------------------------------------------------------------------------------------------------
	// 矩形と矩形の衝突検知　※矩形の中心と幅高さ指定
	// arg1...矩形Aのｘ座標
	// arg2...矩形Aのｙ座標
	// arg3...矩形Aの幅
	// arg4...矩形Aの高さ
	// arg5...矩形Bのｘ座標
	// arg6...矩形Bのｙ座標
	// arg7...矩形Bの幅
	// arg8...矩形Bの高さ
	// ret....[ 衝突している : true ]   [ 衝突していない : false ]
	bool IsIntersectRect(int a_rect_x, int a_rect_y, int a_rect_size_w, int a_rect_size_h,
		int b_rect_x, int b_rect_y, int b_rect_size_w, int b_rect_size_h);
	bool IsIntersectRect(float a_rect_x, float a_rect_y, float a_rect_size_w, float a_rect_size_h,
		float b_rect_x, float b_rect_y, float b_rect_size_w, float b_rect_size_h);

	//-----------------------------------------------------------------------------------------------------
	// 矩形と矩形の衝突検知　※矩形の中心と幅高さ指定
	// arg1...矩形Aの座標
	// arg2...矩形Aの幅
	// arg3...矩形Aの高さ
	// arg4...矩形Bの座標
	// arg5...矩形Bの幅
	// arg6...矩形Bの高さ
	// ret....[ 衝突している : true ]   [ 衝突していない : false ]
	inline bool IsIntersectRect(const tnl::Vector2f& a, float a_rect_size_w, float a_rect_size_h,
		const tnl::Vector2f& b, float b_rect_size_w, float b_rect_size_h) {
		return IsIntersectRect(a.x, a.y, a_rect_size_w, a_rect_size_h, b.x, b.y, b_rect_size_w, b_rect_size_h);
	}
	inline bool IsIntersectRect(const tnl::Vector2f& a, const tnl::Vector2f& a_size,
		const tnl::Vector2f& b, const tnl::Vector2f& b_size) {
		return IsIntersectRect(a.x, a.y, a_size.x, a_size.y, b.x, b.y, b_size.x, b_size.y);
	}
	inline bool IsIntersectRect(const tnl::Vector3& a, float a_rect_size_w, float a_rect_size_h,
		const tnl::Vector3& b, float b_rect_size_w, float b_rect_size_h) {
		return IsIntersectRect(a.x, a.y, a_rect_size_w, a_rect_size_h, b.x, b.y, b_rect_size_w, b_rect_size_h);
	}

	//----------------------------------------------------------------------------------------------
	// work... 矩形と円の衝突判定
	// arg1... 矩形の中心座標
	// arg2... 矩形サイズ
	// arg3... 円の座標
	// arg4... 円の半径
	// ret.... [衝突している : true] [衝突していない : false]
	bool IsIntersectRectCircle(const tnl::Vector2f& rect_center, const tnl::Vector2f& rect_size, const tnl::Vector2f& circle_pos, float circle_radius);


	//----------------------------------------------------------------------------------------------
	// work... 線分と円の衝突判定
	// arg1... 線分の始点座標
	// arg2... 線分の終点座標
	// arg3... 円の座標
	// arg4... 円の半径
	// ret.... [衝突している : true] [衝突していない : false]
	bool IsIntersectLineCircle(const tnl::Vector2f& line_s, const tnl::Vector2f& line_e, const tnl::Vector2f& circle_pos, float radius);

	//----------------------------------------------------------------------------------------------
	// work... 2D上の線分の判定
	// arg1... 線分Aの始点
	// arg2... 線分Aの終点
	// arg3... 線分Bの始点
	// arg4... 線分Bの終点
	// ret.... [衝突している : true] [衝突していない : false]
	// tips... none
	bool IsIntersectLine2D(const Vector3& p1, const Vector3& p2, const Vector3& p3, const Vector3& p4);

	//----------------------------------------------------------------------------------------------
	// work... 三角形と線分の衝突判定
	// arg1-3. 三角形を構成する頂点
	// arg5-5. 始点と終点 
	// ret.... [衝突している : true] [衝突していない : false]
	// tips... 2D 用
	bool IsIntersectLineTriangle2D(const Vector3& _v1, const Vector3& _v2, const Vector3& _v3, const Vector3& _s, const Vector3& _e);




	//----------------------------------------------------------------------------------------------
	// work... AABB 同士の衝突判定
	// arg1... A 座標
	// arg2... A のサイズ
	// arg3... B の座標
	// arg4... B のサイズ
	// ret.... [衝突している : true] [衝突していない : false]
	bool IsIntersectAABB(const tnl::Vector3& a, const tnl::Vector3& a_size, const tnl::Vector3& b, const tnl::Vector3& b_size);

	//-----------------------------------------------------------------------------------------------------
	// 球 と AABB の衝突検知
	// arg1... 球の座標
	// arg2... 球の半径
	// arg3... AABB Max
	// arg4... AABB Min
	// ret....[ 衝突している : true ]   [ 衝突していない : false ]
	bool IsIntersectSphereAABB(const tnl::Vector3& sphere_pos, float sphere_r, const tnl::Vector3& aabb_max, const tnl::Vector3& aabb_min);

	//-----------------------------------------------------------------------------------------------------
	// 球 と OBB の衝突検知
	// arg1... 球の座標
	// arg2... 球の半径
	// arg3... OBB の座標
	// arg4... OBB のサイズ
	// arg5... OBB の姿勢
	// ret....[ 衝突している : true ]   [ 衝突していない : false ]
	bool IsIntersectSphereOBB(const tnl::Vector3& sphere_pos, float sphere_r, const tnl::Vector3& obb_pos, const tnl::Vector3& obb_size, const tnl::Quaternion& obb_rot);

	//-----------------------------------------------------------------------------------------------------
	// 球と球の衝突検知
	// arg1...円Aの中心座標
	// arg2...円Aの半径
	// arg3...円Bの中心座標
	// arg4...円Bの半径
	// ret....[ 衝突している : true ]   [ 衝突していない : false ]
	bool IsIntersectSphere(const tnl::Vector3& a, float a_radius, const tnl::Vector3& b, float b_radius);

	//----------------------------------------------------------------------------------------------
	// work... 線分と平面の衝突判定
	// arg1... 線分の始点
	// arg2... 線分の終点
	// arg3... 平面上の座標 ( 0, 0, 0 以外 )
	// arg4... 平面の法線
	// arg5... 衝突点の受け取り用( 必要なければ省略可 )
	// ret.... [衝突している : true] [衝突していない : false]
	// tips... 衝突していない場合 arg5 で渡した引数は変化なし
	bool IsIntersectLinePlane(const tnl::Vector3 &line_start, const tnl::Vector3 &line_end,
		const tnl::Vector3 &plane_pos, const tnl::Vector3 &plane_normal,
		tnl::Vector3* intersect_position = nullptr);

	//----------------------------------------------------------------------------------------------
	// work... 三角形と点の判定
	// arg1-3. 三角形の頂点座標
	// ret.... [衝突している : true] [衝突していない : false]
	// tips... none
	bool IsIntersectTrianglePoint(const Vector3& v1, const Vector3& v2, const Vector3& v3, const Vector3& p);

	//----------------------------------------------------------------------------------------------
	// work... 三角形と線分の衝突判定
	// arg1-3. 三角形を構成する頂点
	// arg2-4. 始点と終点 
	// ret.... [衝突している : true] [衝突していない : false]
	// tips... 3D 用
	bool IsIntersectLineTriangle(const Vector3& v1, const Vector3& v2, const Vector3& v3, const Vector3& s, const Vector3& e);

	//-------------------------------------------------------------------------------------------------
	// 光線と球の衝突判定
	// arg1... 光線の始点
	// arg2... 光線の方向ベクトル ( 正規化ベクトル )
	// arg3... 球の中心座標
	// arg4... 球の半径
	// arg5... 光線の始点から交点までの距離
	// arg6... 交点
	// ret.... [衝突している : true] [衝突していない : false]
	bool IsIntersectRaySphere( const tnl::Vector3& p, const tnl::Vector3& d, const tnl::Vector3& sc, float sr, float* pt = nullptr, tnl::Vector3* intersect_point = nullptr);

	//-------------------------------------------------------------------------------------------------
	// 線分と球の衝突判定
	// arg1... 線分の始点
	// arg2... 線分の終点
	// arg3... 球の中心座標
	// arg4... 球の半径
	// arg5... 線分の始点(t = 0.0f) から終点(t = 1.0f)方向で衝突が起こった地点
	// arg6... 交点
	// ret.... [衝突している : true] [衝突していない : false]
	bool IsIntersectLineSphere(const tnl::Vector3& s, const tnl::Vector3& e, const tnl::Vector3& sc, float sr, float* pt = nullptr, tnl::Vector3* intersect_point = nullptr);

	//-------------------------------------------------------------------------------------------------
	// 線分と円柱の衝突判定
	// arg1... 線分の始点
	// arg2... 線分の終点
	// arg3... 円柱の始点
	// arg4... 円柱の終点
	// arg5... 円柱の半径
	// arg6... 線分の始点(t = 0.0f) から終点(t = 1.0f)方向で衝突が起こった地点
	// ret.... [衝突している : true] [衝突していない : false]
	bool IsIntersectLineCylinder(const tnl::Vector3& sa, const tnl::Vector3& sb, const tnl::Vector3& p, const tnl::Vector3& q, float r, float* pt = nullptr);

	//-------------------------------------------------------------------------------------------------
	// 線分とカプセルの衝突判定
	// arg1... 線分の始点
	// arg2... 線分の終点
	// arg3... カプセルの始点
	// arg4... カプセルの終点
	// arg5... カプセルの半径
	// arg6... 線分の始点(t = 0.0f) から終点(t = 1.0f)方向で衝突が起こった地点
	// ret.... [衝突している : true] [衝突していない : false]
	bool IsIntersectLineCapsule(const tnl::Vector3& sa, const tnl::Vector3& sb, const tnl::Vector3& p, const tnl::Vector3& q, float r, float* pt = nullptr);

	//----------------------------------------------------------------------------------------------
	// work... 光線とAABB の衝突判定
	// arg1... 光線の始点
	// arg2... 光線ベクトル
	// arg3... 左奥上の座標
	// arg4... 右前下の座標
	// arg5... 始点から交点までの距離
	// arg6... 演算結果の交点
	// arg7... 交点が存在する面の法線
	// ret.... [衝突している : true] [衝突していない : false]
	bool IsIntersectRayAABB(const Vector3& pos, const Vector3& dir, const Vector3& aabb_max, const Vector3& aabb_min, float* pt = nullptr, Vector3* intersect_pos = nullptr, Vector3* intersect_surface_nml = nullptr);

	//----------------------------------------------------------------------------------------------
	// work... レイとOBB の衝突判定
	// arg1... レイ座標 ( 線分なら始点 )
	// arg2... レイベクトル
	// arg3... OBB の座標
	// arg4... OBB のサイズ
	// arg5... OBB の姿勢
	// arg7... 演算結果の交点
	// arg8... 交点が存在する面の法線
	// ret.... [衝突している : true] [衝突していない : false]
	bool IsIntersectRayOBB( const Vector3& pos, const Vector3& dir, const Vector3& obb_pos, const Vector3& obb_size, const Quaternion& obb_rot, Vector3* intersect_pos = nullptr, Vector3* intersect_surface_nml = nullptr);

	//----------------------------------------------------------------------------------------------
	// work... 線分とAABB の衝突判定
	// arg1... 始点
	// arg2... 終点
	// arg3... AABB の max
	// arg4... AABB min
	// arg5... 始点から交点までの距離
	// arg6... 演算結果の交点
	// arg7... 交点が存在する面の法線
	// ret.... [衝突している : true] [衝突していない : false]
	bool IsIntersectLineAABB(const Vector3& s, const Vector3& e, const Vector3& aabb_max, const Vector3& aabb_min, float* pt = nullptr, Vector3* intersect_pos = nullptr, Vector3* intersect_surface_nml = nullptr);

	//----------------------------------------------------------------------------------------------
	// work... 線分とOBB の衝突判定
	// arg1... 始点
	// arg2... 終点
	// arg3... OBB の座標
	// arg4... OBB のサイズ
	// arg5... OBB の姿勢
	// arg6... 演算結果の交点
	// arg7... 交点が存在する面の法線
	// ret.... [衝突している : true] [衝突していない : false]
	// tips... OBB は原点に配置されていることを想定しています
	bool IsIntersectLineOBB(const Vector3& s, const Vector3& e, const Vector3& obb_pos, const Vector3& obb_size, const Quaternion& obb_rot, Vector3* intersect_pos = nullptr, Vector3* intersect_surface_nml = nullptr);

	//----------------------------------------------------------------------------------------------
	// work... 線分とOBB の衝突判定 2D用
	// arg1... 始点
	// arg2... 終点
	// arg3... ボックス座標
	// arg4... OBB の座標
	// arg5... OBB のサイズ
	// arg6... OBB の姿勢
	// arg7... 演算結果の交点
	// ret.... [衝突している : true] [衝突していない : false]
	bool IsIntersectLineOBB2D(const Vector3& s, const Vector3& e, const Vector3& pos, const Vector3& obb_pos, const Vector3& obb_size, const Quaternion& obb_rot, Vector3* intersect_pos = nullptr);

	//-------------------------------------------------------------------------------------------------
	// カプセルと平面の衝突判定
	// arg1... カプセルの始点
	// arg2... カプセルの終点
	// arg3... カプセルの半径
	// arg4... 平面上の座標
	// arg5... 平面の法線
	// arg6... カプセルの始点(t = 0.0f) から終点(t = 1.0f)方向で衝突が起こった地点
	// ret.... [衝突している : true] [衝突していない : false]
	bool IsIntersectCapsulePlane(const tnl::Vector3& cap_st, const tnl::Vector3& cap_en, float cap_r, const tnl::Vector3& plane_pos, const tnl::Vector3& plane_nml, float* t = nullptr);

	//-------------------------------------------------------------------------------------------------
	// カプセルと球の衝突判定
	// arg1... カプセルの始点
	// arg2... カプセルの終点
	// arg3... カプセルの半径
	// arg4... 球 の中心座標
	// arg5... 球 の半径
	// arg6... カプセルの始点(t = 0.0f) から終点(t = 1.0f)方向で衝突が起こった地点
	// ret.... [衝突している : true] [衝突していない : false]
	bool IsIntersectCapsuleSphere(const tnl::Vector3& cap_st, const tnl::Vector3& cap_en, float cap_r, const tnl::Vector3& sph_pos, float sph_r, float* t = nullptr);

	//-------------------------------------------------------------------------------------------------
	// カプセルとAABBの衝突判定
	// arg1... カプセルの始点
	// arg2... カプセルの終点
	// arg3... カプセルの半径
	// arg4... AABB MAX
	// arg5... AABB MIN
	// arg6... カプセルの始点(t = 0.0f) から終点(t = 1.0f)方向で衝突が起こった地点
	// ret.... [衝突している : true] [衝突していない : false]
	bool IsIntersectCapsuleAABB(const tnl::Vector3& cap_st, const tnl::Vector3& cap_en, const float cap_r, const tnl::Vector3& aabb_max, const tnl::Vector3& aabb_min, float* pt = nullptr);

	//-------------------------------------------------------------------------------------------------
	// カプセルとOBBの衝突判定
	// arg1... カプセルの始点
	// arg2... カプセルの終点
	// arg3... カプセルの半径
	// arg4... OBB の中心座標
	// arg5... OBB のサイズ
	// arg6... OBB の姿勢
	// arg7... カプセルの始点(t = 0.0f) から終点(t = 1.0f)方向で衝突が起こった地点
	// ret.... [衝突している : true] [衝突していない : false]
	bool IsIntersectCapsuleOBB( const tnl::Vector3& cap_st, const tnl::Vector3& cap_en, const float cap_r, const tnl::Vector3& obb_center, const tnl::Vector3& obb_size, const tnl::Quaternion& obb_rot, float* pt = nullptr);

	//----------------------------------------------------------------------------------------------
	// work... レイとトーラスの衝突判定
	// arg1... レイ座標
	// arg2... レイベクトル
	// arg3... トーラス中心座標
	// arg4... トーラス姿勢
	// arg5... トーラスの輪の半径
	// arg6... トーラス全体の半径
	// ret.... [衝突している : true] [衝突していない : false]
	bool IsIntersectRayTorus(const Vector3& s, const Vector3& dir, const Vector3& tp, const Quaternion& tq, const float tube_r, const float swept_r);
	bool IsIntersectRayTorus(const Vector3& s, const Vector3& dir, const Vector3& tp, const Quaternion& tq, const float tube_r, const float swept_r, Vector3* intersect_pos = nullptr );

}
