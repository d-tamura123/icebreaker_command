#pragma once

#include <vector>
#include <tuple>
#include <numbers>

namespace tnl {

	//----------------------------------------------------------------------------------------------
	// ベクトル型インスタンスの中で
	// 最も base_point に近い座標を返す
	// arg1...... 基点座標
	// arg2以降.. 可変個指定のベクトルインスタンス
	// ret....... arg2 で指定したインスタンスの中で基点座標に最も近い座標
	// tips...... tnl::Vector3 or tnl::Vector2f を想定
	template <class Vector, typename ...Args>
	Vector GetNearPoint(const Vector& base_point, Args... other_points) {
		const Vector array[] = { static_cast<Vector>(other_points)... };
		Vector answer = array[0];
		float dist = FLT_MAX;
		for (Vector& point : array) {
			float d = (base_point - point).length();
			if (d > dist) continue;
			dist = d;
			answer = point;
		}
		return answer;
	}

	//----------------------------------------------------------------------------------------------
	// ベクトル型インスタンスの中で
	// 最も base_point に近い座標を返す
	// arg1... 基点座標
	// arg2... ベクトルの配列
	// arg3... 配列の長さ
	// ret.... arg2 で指定した配列の中で基点座標に最も近い座標
	// tips...... tnl::Vector3 or tnl::Vector2f を想定
	template <class Vector>
	Vector GetNearPoint(const Vector& base_point, const Vector* other_points, int32_t other_point_num) {
		Vector answer = other_points[0];
		float dist = FLT_MAX;
		for (int32_t i = 0; i < other_point_num; ++i) {
			float d = (base_point - other_points[i]).length();
			if (d > dist) continue;
			dist = d;
			answer = other_points[i];
		}
		return answer;
	}

	//----------------------------------------------------------------------------------------------
	// ベクトル型インスタンスの中で
	// 最も base_point に近い座標を返す
	// arg1... 基点座標
	// arg2... ベクトルの std::vector
	// ret.... arg2 で指定した配列の中で基点座標に最も近い座標
	// tips...... tnl::Vector3 or tnl::Vector2f を想定
	template <class Vector>
	Vector GetNearPoint(const Vector& base_point, const std::vector<Vector>& other_points) {
		Vector answer = other_points[0];
		float dist = FLT_MAX;
		for (int32_t i = 0; i < (int32_t)other_points.size(); ++i) {
			float d = (base_point - other_points[i]).length();
			if (d > dist) continue;
			dist = d;
			answer = other_points[i];
		}
		return answer;
	}

	//----------------------------------------------------------------------------------------------
	// ベクトル型の std::vector で
	// 最も base_point に近い座標の配列番号を返す
	// arg1... 基点座標
	// arg2... ベクトルの std::vector
	// tips...... tnl::Vector3 or tnl::Vector2f を想定
	template <class Vector>
	int32_t GetNearPointIndex(const Vector& base_point, const std::vector<Vector>& other_points) {
		int32_t index = 0;
		float dist = FLT_MAX;
		for (int32_t i = 0; i < (int32_t)other_points.size(); ++i) {
			float d = (base_point - other_points[i]).length();
			if (d > dist) continue;
			dist = d;
			index = i;
		}
		return index;
	}

	//----------------------------------------------------------------------------------------------
	// ベクトル型の 配列 で
	// 最も base_point に近い座標の配列番号を返す
	// arg1... 基点座標
	// arg2... ベクトルの配列
	// arg2... ベクトルの長さ
	// tips... tnl::Vector3 or tnl::Vector2f を想定
	template <class Vector>
	int32_t GetNearPointIndex(const Vector& base_point, const Vector* other_points, int32_t other_point_num) {
		int32_t index = 0;
		float dist = FLT_MAX;
		for (int32_t i = 0; i < other_point_num; ++i) {
			float d = (base_point - other_points[i]).length();
			if (d > dist) continue;
			dist = d;
			index = i;
		}
		return index;
	}


	//----------------------------------------------------------------------------------------------
	// ベクトル型インスタンスの中で
	// 最も base_point に遠い座標を返す
	// arg1...... 基点座標
	// arg2以降.. 可変個指定のベクトルインスタンス
	// ret....... arg2 で指定したインスタンスの中で基点座標に最も遠い座標
	// tips...... tnl::Vector3 or tnl::Vector2f を想定
	template <class Vector, typename ...Args>
	Vector GetFarPoint(const Vector& base_point, Args... other_points) {
		const Vector array[] = { static_cast<Vector>(other_points)... };
		Vector answer = array[0];
		float dist = 0;
		for (Vector& point : array) {
			float d = (base_point - point).length();
			if (d < dist) continue;
			dist = d;
			answer = point;
		}
		return answer;
	}

	//----------------------------------------------------------------------------------------------
	// ベクトル型インスタンスの中で
	// 最も base_point に遠い座標を返す
	// arg1... 基点座標
	// arg2... ベクトルの配列
	// arg3... 配列の長さ
	// ret.... arg2 で指定した配列の中で基点座標に最も遠い座標
	// tips...... tnl::Vector3 or tnl::Vector2f を想定
	template <class Vector>
	Vector GetFarPoint(const Vector& base_point, const Vector* other_points, int32_t other_point_num) {
		Vector answer = other_points[0];
		float dist = 0;
		for (int32_t i = 0; i < other_point_num; ++i) {
			float d = (base_point - other_points[i]).length();
			if (d < dist) continue;
			dist = d;
			answer = other_points[i];
		}
		return answer;
	}

	//----------------------------------------------------------------------------------------------
	// ベクトル型インスタンスの中で
	// 最も base_point に遠い座標を返す
	// arg1... 基点座標
	// arg2... ベクトルの std::vector
	// ret.... arg2 で指定した配列の中で基点座標に最も遠い座標
	// tips...... tnl::Vector3 or tnl::Vector2f を想定
	template <class Vector>
	Vector GetFarPoint(const Vector& base_point, const std::vector<Vector>& other_points) {
		Vector answer = other_points[0];
		float dist = 0;
		for (int32_t i = 0; i < (int32_t)other_points.size(); ++i) {
			float d = (base_point - other_points[i]).length();
			if (d < dist) continue;
			dist = d;
			answer = other_points[i];
		}
		return answer;
	}

	//----------------------------------------------------------------------------------------------
	// ベクトル型の std::vector で
	// 最も base_point に遠い座標の配列番号を返す
	// arg1... 基点座標
	// arg2... ベクトルの std::vector
	// tips...... tnl::Vector3 or tnl::Vector2f を想定
	template <class Vector>
	int32_t GetFarPointIndex(const Vector& base_point, const std::vector<Vector>& other_points) {
		int32_t index = 0;
		float dist = 0;
		for (int32_t i = 0; i < (int32_t)other_points.size(); ++i) {
			float d = (base_point - other_points[i]).length();
			if (d < dist) continue;
			dist = d;
			index = i;
		}
		return index;
	}

	//----------------------------------------------------------------------------------------------
	// ベクトル型の 配列 で
	// 最も base_point に遠い座標の配列番号を返す
	// arg1... 基点座標
	// arg2... ベクトルの配列
	// arg2... ベクトルの長さ
	// tips... tnl::Vector3 or tnl::Vector2f を想定
	template <class Vector>
	int32_t GetFarPointIndex(const Vector& base_point, const Vector* other_points, int32_t other_point_num) {
		int32_t index = 0;
		float dist = 0;
		for (int32_t i = 0; i < other_point_num; ++i) {
			float d = (base_point - other_points[i]).length();
			if (d < dist) continue;
			dist = d;
			index = i;
		}
		return index;
	}

}

