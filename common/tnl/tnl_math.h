#pragma once

#include <random>
#include <vector>
#include <tuple>
#include <numbers>
#include "tnl_vector2i.h"
#include "tnl_vector3.h"

namespace tnl {

	extern std::mt19937 g_mersenne;

	//----------------------------------------------------------------------------------------------
	// 円周率
	const float PI = 3.14159265358979f;

	//----------------------------------------------------------------------------------------------
	// ラジアンからデグリーへの変換
	inline float ToDegree(float radian) { return (radian / PI * 180.0f); }

	//----------------------------------------------------------------------------------------------
	// デグリーからラジアンへの変換
	inline float ToRadian(float degree) { return (PI / 180.0f * degree); }

	//----------------------------------------------------------------------------------------------
	// 座標とサイズから AABB の max min を取得
	inline tnl::Vector3 ToMaxAABB(const tnl::Vector3& pos, const tnl::Vector3& size) { return pos + (size * 0.5f); }
	inline tnl::Vector3 ToMinAABB(const tnl::Vector3& pos, const tnl::Vector3& size) { return pos - (size * 0.5f); }

	//----------------------------------------------------------------------------------------------
	// メルセンヌツイスター乱数シード設定
	void SetSeedMersenneTwister32(int seed);	

	//----------------------------------------------------------------------------------------------
	// 浮動小数乱数の取得
	// tips... この関数を使用する場合 SetSeedMersenneTwister32 で乱数シードを設定してください
	template<class T>
	T GetRandomDistribution(T min, T max) {
		std::uniform_real_distribution<> param(static_cast<double>(min), static_cast<double>(max));
		return static_cast<T>(param(g_mersenne));
	};
	template<> int32_t GetRandomDistribution<int32_t>(int32_t min, int32_t max);

	//----------------------------------------------------------------------------------------------
	// work... 指定座標が平面の表裏どちらに存在するか判定する
	// arg1... 指定座標
	// arg2... 平面の法線
	// arg3... 平面上に存在するどこかの座標
	// ret.... 表...1 	裏...-1		平面上...0
	// tips... none
	int GetSidesPointAndPlane(const Vector3& v, const Vector3& pn, const Vector3& pv);

	//----------------------------------------------------------------------------------------------
	// work... 線分の始点と終点がワールド軸に並行なXYZいずれかの平面上に存在するか
	// arg1... 始点
	// arg2... 終点
	// ret.... [ 線分がXYZ平面上に存在する : true ] [ 存在しない : false ]
	bool IsParallelLineToXYZPlane(const tnl::Vector3& s, const tnl::Vector3& e);

	//----------------------------------------------------------------------------------------------
	enum class eParallelRegionPtRect { OUTSIDE = -1, UP, RIGHT, DOWN, LEFT, MAX };
	// work... 矩形の各辺から並行な領域で
	//         指定した座標が対角線から上下左右どこに存在するか取得する
	// arg1... 指定座標
	// arg2... 矩形の中心座標
	// arg3... 矩形の幅
	// arg4... 矩形の高さ
	// tips... 指定座標が領域外( 矩形内も含む )場合 OUTSIDE が帰ります
	eParallelRegionPtRect GetParallelRegionPointAndRect(const tnl::Vector2f& p, const tnl::Vector2f& rp, const int rect_w, const int rect_h) ;

	//----------------------------------------------------------------------------------------------
	enum class eDiagonalRegionPtRect { UP, RIGHT, DOWN, LEFT, MAX };
	// work... 矩形の中心から２本の無限に伸びる対角線を引いた時
	//         指定した座標が対角線から上下左右どこに存在するか取得する
	// arg1... 指定座標
	// arg2... 矩形の中心座標
	// arg3... 矩形の幅
	// arg4... 矩形の高さ
	eDiagonalRegionPtRect GetDiagonalRegionPointAndRect(const Vector2f& p, const Vector2f& rp, const int rect_w, const int rect_h);

	//----------------------------------------------------------------------------------------------
	enum class eParallelRegionPtAABB {
		INSIDE
		, LEFT_FORWARD
		, LEFT_BACK
		, RIGHT_FORWARD
		, RIGHT_BACK
		, UP_FORWARD
		, UP_BACK
		, DOWN_FORWARD
		, DOWN_BACK
		, UP_LEFT
		, UP_RIGHT
		, DOWN_LEFT
		, DOWN_RIGHT
		, UP_LEFT_FORWARD
		, UP_LEFT_BACK
		, UP_RIGHT_FORWARD
		, UP_RIGHT_BACK
		, DOWN_LEFT_FORWARD
		, DOWN_LEFT_BACK
		, DOWN_RIGHT_FORWARD
		, DOWN_RIGHT_BACK
		, DOWN
		, RIGHT
		, UP
		, LEFT
		, BACK
		, FORWARD
		, MAX
	};
	// work... AABBの各面が向いている面範囲の領域に対して
	//         指定した座標が領域の上下左右前後どこに存在するか取得する
	// arg1... 指定座標
	// arg2... AABB max
	// arg3... AABB min
	eParallelRegionPtAABB GetParallelRegionPointAndAABB(const tnl::Vector3& p, const tnl::Vector3& max, const tnl::Vector3& min);

	//----------------------------------------------------------------------------------------------
	enum class eDiagonalRegionPtAABB { DOWN, RIGHT, UP, LEFT, BACK, FORWARD, MAX };
	// work... AABBの中心から全ての対角辺へ無限に伸びる平面を考えたとき
	//         指定した座標が対角辺領域の上下左右前後どこに存在するか取得する
	// arg1... 指定座標
	// arg2... AABB max
	// arg3... AABB min
	eDiagonalRegionPtAABB GetDiagonalRegionPointAndAABB( const Vector3& p, const Vector3& max, const Vector3& min );

	//----------------------------------------------------------------------------------------------
	// work... AABBにおいて与えられた点が対角辺領域に対応する面の法線を取得する
	// arg1... 点座標
	// arg2... AABB Max
	// arg3... AABB Min
	// arg4... 角を考慮した法線を取得するか
	// ret.... AABB 面の法線
	tnl::Vector3 GetDiagonalRegionSurfaceNormalAABB(const tnl::Vector3& p, const tnl::Vector3& aabb_max, const tnl::Vector3& aabb_min, bool enable_corner = false);

	//----------------------------------------------------------------------------------------------
	enum class eParallelRegionPtOBB { 
		INSIDE
		, LEFT_FORWARD
		, LEFT_BACK
		, RIGHT_FORWARD
		, RIGHT_BACK
		, UP_FORWARD
		, UP_BACK
		, DOWN_FORWARD
		, DOWN_BACK
		, UP_LEFT
		, UP_RIGHT
		, DOWN_LEFT
		, DOWN_RIGHT
		, UP_LEFT_FORWARD
		, UP_LEFT_BACK
		, UP_RIGHT_FORWARD
		, UP_RIGHT_BACK
		, DOWN_LEFT_FORWARD
		, DOWN_LEFT_BACK
		, DOWN_RIGHT_FORWARD
		, DOWN_RIGHT_BACK
		, DOWN
		, RIGHT
		, UP
		, LEFT
		, BACK
		, FORWARD
		, MAX
	};
	// work... OBB の各面が向いている面範囲の領域に対して
	//         指定した座標が指定した座標が領域の上下左右前後どこに存在するか取得する
	// arg1... 指定座標
	// arg2... OBB の中心座標
	// arg3... OBB の幅, 高さ, 奥行
	// arg4... OBB の姿勢
	eParallelRegionPtOBB GetParallelRegionPointAndOBB(const tnl::Vector3& p, const tnl::Vector3& op, const tnl::Vector3& size, const tnl::Quaternion& q);


	//----------------------------------------------------------------------------------------------
	enum class eDiagonalRegionPtOBB { DOWN, RIGHT, UP, LEFT, BACK, FORWARD, MAX };
	// work... OBBの中心から全ての対角辺へ無限に伸びる平面を考えたとき
	//         指定した座標が対角辺領域の上下左右前後どこに存在するか取得する
	// arg1... 指定座標
	// arg2... OBB の中心座標
	// arg3... OBB の幅, 高さ, 奥行
	// arg4... OBB の姿勢
	eDiagonalRegionPtOBB GetDiagonalRegionPointAndOBB(const Vector3& p, const Vector3& op, const Vector3& size, const Quaternion& q);

	//----------------------------------------------------------------------------------------------
	// work... OBBにおいて与えられた点が対角辺領域に対応する面の法線を取得する
	// arg1... 点座標
	// arg2... OBB の座標
	// arg3... OBB のサイズ
	// arg3... OBB の姿勢
	// arg4... 角を考慮した法線を取得するか
	// ret.... OBB の面法線
	tnl::Vector3 GetDiagonalRegionSurfaceNormalOBB(const tnl::Vector3& p, const tnl::Vector3& obb_pos, const tnl::Vector3& obb_size, const tnl::Quaternion& obb_rot, bool enable_corner = false);

	//----------------------------------------------------------------------------------------------
	enum class eDiagonalRegionPtXzOBB{ FORWARD, RIGHT, BACK, LEFT, MAX };
	// work... OBB のローカル空間に対してY軸に垂直な平面を OBB の対角に２枚 クロスした時
	//         指定した座標が対角面から前後左右どこに存在するか取得する
	// arg1... 指定座標
	// arg2... OBB の中心座標
	// arg3... OBB の幅, 高さ, 奥行
	// arg4... OBB の姿勢
	eDiagonalRegionPtXzOBB GetXzDiagonalRegionPointAndOBB(const Vector3& p, const Vector3& op, const Vector3& size, const Quaternion& q);


	//-------------------------------------------------------------------------------------------------
	// 光線と点の距離
	// arg1... 点座標
	// arg2... 光線の原点
	// arg3... 光線の向き
	// ret.... 距離
	float GetDistancePointRay(const tnl::Vector3& point, const tnl::Vector3& ray_origin, const tnl::Vector3& ray_dir);

	//----------------------------------------------------------------------------------------------
	// work... 点とAABBのAABB内に存在する点への最近接点を取得
	// arg1... 点座標
	// arg2... AABB max
	// arg3... AABB min
	// ret... 最近接点
	tnl::Vector3 GetNearestPointAABB(const tnl::Vector3& point, const tnl::Vector3& aabb_max, const tnl::Vector3& aabb_min );

	//----------------------------------------------------------------------------------------------
	// work... 点とOBBのOBB内に存在する点への最近接点を取得
	// arg1... 点座標
	// arg2... OBB の中心座標
	// arg3... OBB のサイズ
	// arg4... OBB の姿勢
	// ret... 最近接点
	tnl::Vector3 GetNearestPointOBB(const tnl::Vector3& point, const tnl::Vector3& obb_pos, const tnl::Vector3& obb_size, const tnl::Quaternion& obb_rot);

	//----------------------------------------------------------------------------------------------
	// work... 点と矩形の矩形内に存在する点への最近接点を取得
	// arg1... 矩形座標
	// arg2... 矩形の幅
	// arg3... 矩形の高さ
	// arg4... 点座標
	// ret... 最近接点( 2D )
	tnl::Vector2f GetNearestRectPoint(const tnl::Vector2f& rect_pos, float w, float h, const tnl::Vector2f& point);

	//----------------------------------------------------------------------------------------------
	// name... GetNearestRectPoint
	// work... 点と長方形の長方形内に存在する点への最近接点を取得
	// arg1... 長方形の中心
	// arg2... 長方形のX軸の単位ベクトル
	// arg3... 長方形のY軸の単位ベクトル
	// arg4... 長方形の辺の長さの半分
	// arg5... 点座標
	// ret... 最近接点( 3D )
	tnl::Vector3 GetNearestRectPoint(const tnl::Vector3& rect_pos, const tnl::Vector3& axis_x, const tnl::Vector3& axis_y, const tnl::Vector3& rect_side_length, const tnl::Vector3& point);

	//----------------------------------------------------------------------------------------------
	// work... 点と三角形の三角形上に存在する最近点
	// arg1... 点座標
	// arg234. 三角形の頂点座標
	// arg5... 三角形上の座標
	tnl::Vector3 GetNearestPointTriangle(const tnl::Vector3& p, const tnl::Vector3& a, const tnl::Vector3& b, const tnl::Vector3& c);

	//----------------------------------------------------------------------------------------------
	// work... 点と平面の平面上に存在する最近点
	// arg1... 点座標
	// arg2... 平面の法線
	// arg3... 平面上の座標
	tnl::Vector3 GetNearestPointPlane(const tnl::Vector3& v, const tnl::Vector3& pn, const tnl::Vector3& pv);

	//----------------------------------------------------------------------------------------------
	// work... 点と光線の光線上に存在する最近点
	// arg1... 点座標
	// arg2... 光線の原点
	// arg3... 光線の向き
	// ret.... 最近点
	tnl::Vector3 GetNearestPointRay(const tnl::Vector3& point, const tnl::Vector3& ray_origin, const tnl::Vector3& ray_dir);

	//----------------------------------------------------------------------------------------------
	// work... 点と線分の線分上に存在する最近点
	// arg1... 点座標
	// arg2... 始点
	// arg3... 終点
	tnl::Vector3 GetNearestPointLine(const tnl::Vector3& v, const tnl::Vector3& a, const tnl::Vector3& b);

	//----------------------------------------------------------------------------------------------
	// work... 線分同士の最近点
	// arg1... 始点A
	// arg2... 始点A
	// arg3... 始点B
	// arg4... 始点B
	// arg5... 線分A内の最近点
	// arg5... 線分B内の最近点
	void GetNearestLines(const tnl::Vector3& s1, const tnl::Vector3& e1, const tnl::Vector3& s2, const tnl::Vector3& e2, tnl::Vector3& seg1_near, tnl::Vector3& seg2_near);

	//----------------------------------------------------------------------------------------------
	// 平面に存在するランダムな点を取得する
	// arg1... 平面を構成する平面内の点
	// arg2... 平面の法線
	// arg3... arg1 から平面内に存在する円の半径 ( この半径内でランダムな点が得られる )
	// ret.... ランダムな点座標
	tnl::Vector3 GetRandomPointInsidePlane(const tnl::Vector3& plane_position, const tnl::Vector3& plane_normal, float radius);

	//----------------------------------------------------------------------------------------------
	enum class eCorrTypeRect { PWRFL_A, PWRFL_B, BOTH, MAX };
	enum class eCorrResRect { LEFT, RIGHT, UP, DOWN, MAX };
	// 矩形同士の座標補正( 軽量版 )
	// arg1 補正対象の座標 ( この関数によって補正後の座標に更新 )
	// arg2 補正対象のサイズ
	// arg3 不動矩形の座標
	// arg4 不動矩形のサイズ
	// arg5 補正後に間に設ける空間 ( デフォルト 0.0f )
	// tips 補正対象の移動量が大きい場合や押し押されが必要な場合は Easy が付いていない補正関数を推奨
	tnl::eCorrResRect EasyCorrectPositionRect(
		tnl::Vector2f& corr_pos
		, const tnl::Vector2f& corr_size
		, const tnl::Vector2f& fixed_pos
		, const tnl::Vector2f& fixed_size
		, float correct_space = 0.0f);

	// 矩形同士の座標補正
	// arg1.2. 矩形 A, B の移動前の座標
	// arg3.4. 矩形 A, B のサイズ
	// arg5.6. 矩形 A, B の移動後の座標 ( この関数によって補正後の座標に更新 )
	// arg7.8. 横, 縦 方向の補正タイプ 
	// ....... [ PWRFL_A : A が 強い ] [ PWRFL_B : B が 強い ] [ BOTH : A と B は押し合う ]
	// arg9... 補正後に A と B の間に設ける空間 ( デフォルト 1.0f )
	// ret.... 矩形 A が B に対してどの方向へ補正されたか 
	eCorrResRect CorrectPositionRect(
		const tnl::Vector2f& a_before
		, const tnl::Vector2f& b_before
		, const tnl::Vector2f& a_size
		, const tnl::Vector2f& b_size
		, tnl::Vector2f& a_out
		, tnl::Vector2f& b_out
		, eCorrTypeRect correct_type_x
		, eCorrTypeRect correct_type_y
		, float correct_space = 1.0f);


	//----------------------------------------------------------------------------------------------
	enum class eCorrTypeAABB { PWRFL_A, PWRFL_B, BOTH, MAX };
	enum class eCorrResAABB { LEFT, RIGHT, UP, DOWN, FORWARD, BACK, MAX };
	// AABB 同士の座標補正
	// arg1.2.		A, B の移動前の座標
	// arg3.4.		A, B のサイズ
	// arg5.6.		A, B の移動後の座標 ( この関数によって補正後の座標に更新 )
	// arg7.8.9.	横, 縦, 奥 方向の補正タイプ
	// .......		[ PWRFL_A : A が 強い ] [ PWRFL_B : B が 強い ] [ BOTH : A と B は押し合う ]
	// arg10..		補正後に A と B の間に設ける空間 ( デフォルト 1.0f )
	// ret....		矩形 A が B に対してどの方向へ補正されたか
	eCorrResAABB CorrectPositionAABB(
		const tnl::Vector3& a_before
		, const tnl::Vector3& b_before
		, const tnl::Vector3& a_size
		, const tnl::Vector3& b_size
		, tnl::Vector3& a_out
		, tnl::Vector3& b_out
		, eCorrTypeAABB correct_type_x
		, eCorrTypeAABB correct_type_y
		, eCorrTypeAABB correct_type_z
		, float correct_space = 1.0f);


	//----------------------------------------------------------------------------------------------
	// 矩形と円の座標補正 ( 円のみ補正 )
	// arg1...		矩形の中心座標
	// arg2...		矩形サイズ
	// arg3...		円の移動前座標
	// arg4...		円の半径
	// arg5...		円の現在座標 ( 補正座標で上書きされます )
	// arg6...		補正後間に設ける空間 ( デフォルト 1.0f )
	// ret....		円が矩形上の最近点からどの方向へ補正されたか ( 正規化ベクトル )
	tnl::Vector2f CorrectPositionRectCircle(
		const tnl::Vector2f& rect_center
		, const tnl::Vector2f& rect_size
		, const tnl::Vector2f& circle_before
		, const float circle_radius
		, tnl::Vector2f& circle_out
		, float correct_space = 1.0f);


	//----------------------------------------------------------------------------------------------
	// 線分と円の座標補正 ( 円のみ補正 )
	// arg1...		線分の始点座標
	// arg2...		線分の終点座標
	// arg3...		円の移動前座標
	// arg4...		円の半径
	// arg5...		円の現在座標 ( 補正座標で上書きされます )
	// arg6...		補正後間に設ける空間 ( デフォルト 1.0f )
	// ret....		円が線分の最近点からどの方向へ補正されたか ( 正規化ベクトル )
	tnl::Vector2f CorrectPositionLineCircle(
		const tnl::Vector2f& line_s
		, const tnl::Vector2f& line_e
		, const tnl::Vector2f& circle_before
		, const float circle_radius
		, tnl::Vector2f& circle_out
		, float correct_space = 1.0f);


	//----------------------------------------------------------------------------------------------
	// 正六角形の辺の長さを幅を元に取得
	// arg1... 正六角形の幅
	// ret.... 辺の長さ
	float GetHexagonEdgeLenght(float width) ;


	//----------------------------------------------------------------------------------------------
	 // 正六角形の高さを幅から取得
	 // arg1... 正六角形の幅
	 // ret.... 辺の長さ
	float GetHexagonHeight(float width);


	//----------------------------------------------------------------------------------------------
	// 正六角形の頂点座標を取得
	// arg1... 正六角形の幅
	// ret.... 頂点座標配列
	std::vector<tnl::Vector3> GetHexagonVertices(float width);


	//----------------------------------------------------------------------------------------------
	// 三角形の頂点から外接円の中心座標と半径を求める
	// arg... 三角形の頂点座標 a, b, c
	// ret... <0>...中心座標  <1>... 半径
	std::tuple<tnl::Vector3, float> CircumscribedCircle(const tnl::Vector3& a, const tnl::Vector3& b, const tnl::Vector3& c);

	//----------------------------------------------------------------------------------------------
	// 直角三角形において与えられた高さと角度から斜辺を計算する
	// arg1... 底辺と斜辺からなる角度 (ラジアン)
	// arg2... 高さ
	// ret.... 斜辺の長さ
	inline float RightTriangleHypotByHeight(float theta, float height) {
		if (height <= 0.0 || theta <= 0.0 || theta >= std::numbers::pi / 2.0f) return -1;
		return height / sinf(theta);
	}
	//----------------------------------------------------------------------------------------------
	// 直角三角形において与えられた底辺と角度から斜辺の長さを計算する
	// arg1... 底辺と斜辺からなる角度 (ラジアン)
	// arg2... 底辺の長さ
	// ret.... 斜辺の長さ
	inline float RightTriangleHypotByBase(float theta, float base) {
		if (base <= 0.0 || theta <= 0.0 || theta >= std::numbers::pi / 2.0f) return -1;
		return base / cosf(theta);
	}	
	//----------------------------------------------------------------------------------------------
	// 直角三角形において与えられた高さと角度から底辺の長さを計算する
	// arg1... 底辺と斜辺からなる角度 (ラジアン)
	// arg2... 高さ
	// ret.... 底辺の長さ
	inline float RightTriangleBaseByHeight(float theta, float height) {
		if (height <= 0.0 || theta <= 0.0 || theta >= std::numbers::pi / 2.0f) return -1;
		return height / tanf(theta);
	}
	//----------------------------------------------------------------------------------------------
	// 直角三角形において与えられた角度と底辺の長さから高さを計算する
	// arg1... 底辺と斜辺からなる角度 (ラジアン)
	// arg2... 底辺の長さ
	// ret.... 高さ
	inline float RightTriangleHeightByBase(float theta, float base) {
		if (base <= 0.0 || theta <= 0.0 || theta >= std::numbers::pi / 2.0f) return -1;
		return base * tanf(theta);
	}

	//----------------------------------------------------------------------------------------------
	// 直角三角形において与えられた高さと斜辺から底辺を計算する
	// arg1... 高さ
	// arg2... 斜辺の長さ
	// ret.... 底辺の長さ
	inline float RightTriangleBaseByHeightHypot(float height, float hypot) {
		return sqrt(hypot * hypot - height * height);
	}

	//----------------------------------------------------------------------------------------------
	// 直角三角形において与えられた底辺と斜辺から高さを計算する
	// arg1... 底辺の長さ
	// arg2... 斜辺の長さ
	// ret.... 高さ
	inline float RightTriangleHeightByBaseHypot(float base, float hypot) {
		return sqrt(hypot * hypot - base * base);
	}

	//----------------------------------------------------------------------------------------------
	// 直角三角形において与えられた底辺と高さから斜辺を計算する
	// arg1... 底辺の長さ
	// arg2... 高さ
	// ret.... 斜辺の長さ
	inline float RightTriangleHypotByBaseHeight(float base, float height) {
		return sqrt(base * base + height * height);
	}

	//----------------------------------------------------------------------------------------------
	// ブレセンハムの線分座標テーブル取得
	// arg1... 始点
	// arg2... 終点
	// ret.... 線分座標テーブル
	std::vector<tnl::Vector2i> GetBresenhamsLine(const tnl::Vector2i& st, const tnl::Vector2i& en);


	//----------------------------------------------------------------------------------------------
	// 点座標から光線を伸ばした時、多角形を構成する辺に対して交差した回数を取得
	// arg1... 多角形の頂点
	// arg2... 点座標
	// ret.... 交差回数
	uint32_t CountPolygonIntersections(const std::vector<tnl::Vector2i>& polygon_vertexs, const tnl::Vector2i& point);


	//----------------------------------------------------------------------------------------------
	// 繋がった線の集合に対して余った部分をカットする
	// arg1... 線を構成する点座標群
	// arg2... 端点が繋がっていない場合に繋げるかどうか
	void CutOverLineLoop( std::vector<tnl::Vector2i>& line_points, bool is_add_loop_point = true );


	//----------------------------------------------------------------------------------------------
	// 質量と摩擦から重心と移動ベクトルの更新を行います
	// arg1... 重心円の半径
	// arg2... 質量 ( 0 ～ 100.0f )
	// arg3... 摩擦係数 ( 0 ～ 1.0f )
	// arg4... 前回の移動ベクトル ( この関数で更新されます )
	// arg5... 今回の移動ベクトル ( この関数で更新されます )
	// arg6... 重心座標
	// tips... 簡易計算であり物理学に準拠したものではありません
	void EasyAdjustObjectVelocity( float centroid_radius, float mass, float friction,
									tnl::Vector3& past_move_vel, tnl::Vector3& move_vel, tnl::Vector3& center_of_gravity);

	//----------------------------------------------------------------------------------------------
	enum class eOscStart { STOK, CENTER };
	// 単振動による振幅位置の計算
	// arg1... 振幅開始位置 [ STOK : 端 ] [ CENTER : 中央 ]
	// arg2... 減衰 : 大きいほど強く減衰する
	// arg3... 振幅速度 : 大きいほど早く振幅する
	// arg4... 経過時間
	// ret.... 振幅位置
	// tips... STOK を指定した場合 : 0 ～ 2 の間で振幅し 1 に収束
	// ....... CENTER を指定した場合 : -1 ～ +1 の間で振幅し 0 に収束
	// ....... STOK CENTER 共に 0 から正方向へ値が振幅
	float SingleOscillationy( eOscStart osc_start, float decay, float period, float progress_time);

	//----------------------------------------------------------------------------------------------
	// 等間隔な値の補間
	// arg1... 始値
	// arg2... 終値
	// arg3... 補間補間完了までの時間
	// arg4... 経過時間
	// ret.... 補間値
	float UniformLerp(float s, float e, float time_limit, float ct);


	//----------------------------------------------------------------------------------------------
	 // sin 波 -90 ~ +90度までを利用した値の補間
	 // arg1... 始値
	 // arg2... 終値
	 // arg3... 補間完了までの時間
	 // arg4... 経過時間
	 // arg5... 内部で時間に対する補間を行う回数 ( デフォルトは0回 最大5回 )
	 // ret.... 補間値
	 // tips... 通常の sin 波による補間では間隔がやや単調だと思う場合
	 // ....... arg5 の引数で補間の間隔を広げ、よりエッジの効いた補間を行うことができます
	float SmoothLerp(float s, float e, float time_limit, float ct, int strength = 0);


	//----------------------------------------------------------------------------------------------
	// 等加速直線運動を利用した値の補間
	// arg1... 始値
	// arg2... 終値
	// arg3... 補間完了までの時間
	// arg4... 経過時間
	// arg5... 内部で時間に対する補間を行う回数 ( デフォルトは0回 最大5回 )
	// ret.... 補間値
	// tips... 通常の 等加速度による補間では間隔がやや単調だと思う場合
	// ....... arg5 の引数で指数関数的に速度が上がるよう調整できます
	float AccelLerp(float s, float e, float time_limit, float ct, int strength = 0);

	//----------------------------------------------------------------------------------------------
	// 等減速直線運動を利用した値の補間
	// arg1... 始値
	// arg2... 終値
	// arg3... 補間完了までの時間
	// arg4... 経過時間
	// ret.... 補間値
	float DecelLerp(float s, float e, float time_limit, float ct);


	//----------------------------------------------------------------------------------------------
	// ベジェスプライン補間
	// arg1... 補間始点
	// arg2... anker
	// arg3... anker
	// arg4... 補間終点
	// arg5... 0.0f ～ 1.0f
	// ret.... 補間座標
	Vector3 BezierSpline(const Vector3& _a1, const Vector3& _a2, const Vector3& _a3, const Vector3& _a4, float t);

	//----------------------------------------------------------------------------------------------
	//
	// 三次スプライン補間
	//
	class CubicSpline final {
	private:
		std::vector<tnl::Vector3> a_;
		std::vector<tnl::Vector3> b_;
		std::vector<tnl::Vector3> c_;
		std::vector<tnl::Vector3> d_;
		std::vector<tnl::Vector3> w_;

	public:
		CubicSpline() {};
		CubicSpline(const std::vector<tnl::Vector3>& v);

		//---------------------------------------------------------------------------
		// 補間座標の取得
		// arg1... 0.0f ～ 1.0f
		// ret.... 補間座標
		inline tnl::Vector3 getPosition(float t) {
			float rt = float(a_.size() - 1) * t;
			int p = int(floor(rt));
			float dt = rt - p;
			return a_[p] + (b_[p] + (c_[p] + d_[p] * dt) * dt) * dt;
		}

	};

	//----------------------------------------------------------------------------------------------
	//
	// 座標群線形補間
	//
	class PointsLerp final {
	private:
		float all_length_ = 0;
		std::vector< tnl::Vector3 > points_;
		std::vector< float > section_lengths_;
		float getLengthRate(float t) const noexcept;
		float getLengthPoint(int pn) const noexcept;
		int getPrevPoint(float t) const noexcept;

	public:

		PointsLerp(const std::vector< tnl::Vector3 >& points);

		//---------------------------------------------------------------------------
		// 補間座標の取得
		// arg1... 0.0f ～ 1.0f
		// ret.... 補間座標
		inline tnl::Vector3 getPosition(const float t) {
			int n = getPrevPoint(t);
			float len = getLengthRate(t) - getLengthPoint(n);
			tnl::Vector3 v = tnl::Vector3::Normalize((points_[n + 1] - points_[n]));
			return points_[n] + (v * len);
		}

	};


	//epsilon surrounding for near zero values 
	const double EQN_EPS = 1e-9;
	inline double IsZeroD(double x) { return ((x) > -EQN_EPS && (x) < EQN_EPS); }
	std::vector<double> Solve2(std::vector<double>& c);
	std::vector<double> Solve3(std::vector<double>& c);
	std::vector<double> Solve4(std::vector<double>& c);

}