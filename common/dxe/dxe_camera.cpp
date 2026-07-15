#pragma warning(disable:4819)
#include "DxLib.h"
#include "../tnl/tnl_vector2i.h"
#include "../tnl/tnl_quaternion.h"
#include "dxe_build_setting.h"
#include "dxe_camera.h"

namespace dxe {

	//------------------------------------------------------------------------------------------------------------
	Camera::Camera(float screen_w, float screen_h, const eDimension dimension, const eLocate2D locate2d) {
		screen_w_ = screen_w;
		screen_h_ = screen_h;
		aspect_ = (float)screen_w_ / (float)screen_h_;
		dimension_ = dimension;
		locate2d_ = locate2d;
		if (dimension == eDimension::Type3D) {
			position_ = { 0, 200, -500 };
			target_ = { 0, 0, 0 };
		}
		else {
			position_ = { 0, 0, 0 };
			target_ = { 0, 0, 10 };
		}
	}


	//------------------------------------------------------------------------------------------------------------
	tnl::Vector2f Camera::getZoomOffsetPosition2D() {
		tnl::Vector2f v;
		float sw = screen_w_;
		float sw2 = screen_w_ * 0.5f;
		float sh = screen_h_;
		float sh2 = screen_h_ * 0.5f;
		float rw = sw / (sw * zoom_);
		float w = -(sw2 * (rw - 1.0f));
		float rh = sh / (sh * zoom_);
		float h = -(sh2 * (rw - 1.0f));
		if (eLocate2D::Y_DOWN == locate2d_) {
			return tnl::Vector2f(w, h);
		}
		return tnl::Vector2f(w, -h);
	}

	//------------------------------------------------------------------------------------------------------------
	tnl::Vector2f Camera::convertWorldToScreenPosition2D(const tnl::Vector2f& world_position, const tnl::Vector2f& affect_weight) {
		tnl::Vector2f v;
		//tnl::Vector2f position = tnl::Vector2f(position_) + ( getZoomOffsetPosition2D() * affect_weight );
		//if (eLocate2D::Y_DOWN == locate2d_) {
		//	v = world_position - (tnl::Vector2f(position) * affect_weight) + tnl::Vector2f(screen_w_ * 0.5f, screen_h_ * 0.5f);
		//}
		//else {
		//	v.x = world_position.x - (position.x * affect_weight.x) + (screen_w_ * 0.5f);
		//	v.y = -world_position.y - (-position.y * affect_weight.y) + (screen_h_ * 0.5f);
		//}

		tnl::Vector2f position = ( tnl::Vector2f(position_) * affect_weight ) + getZoomOffsetPosition2D();
		if (eLocate2D::Y_DOWN == locate2d_) {
			v = world_position - (tnl::Vector2f(position)) + tnl::Vector2f(screen_w_ * 0.5f, screen_h_ * 0.5f);
		}
		else {
			v.x = world_position.x - (position.x) + (screen_w_ * 0.5f);
			v.y = -world_position.y - (-position.y) + (screen_h_ * 0.5f);
		}

		return v * zoom_;
	}

	//------------------------------------------------------------------------------------------------------------
	tnl::Vector2f Camera::convertScreenToWorldPosition2D(const tnl::Vector2f& screen_position, const tnl::Vector2f& affect_weight) {
		tnl::Vector2f v;
		tnl::Vector2f position = ( tnl::Vector2f(position_) * affect_weight ) * zoom_ ;
		tnl::Vector2f scr_position = screen_position ;

		if (eLocate2D::Y_DOWN == locate2d_) {
			v = scr_position + ( tnl::Vector2f(position) ) - tnl::Vector2f(screen_w_ * 0.5f, screen_h_ * 0.5f);
		}
		else {
			v.x = scr_position.x + ( position.x) - (screen_w_ * 0.5f);
			v.y = -(scr_position.y - (position.y) - (screen_h_ * 0.5f));
		}

		return v * (1.0f / zoom_);
	}

	//------------------------------------------------------------------------------------------------------------
	void Camera::update() {
		if (eDimension::Type3D == dimension_) {
			view_ = tnl::Matrix::LookAtLH(position_, target_, up_);
			proj_ = tnl::Matrix::PerspectiveFovLH(angle_, aspect_, near_, far_);
		}
		else {
			target_ = { position_.x, position_.y, 10 };
			view_ = tnl::Matrix::LookAtLH({ position_.x, position_.y, -10 }, { target_.x, target_.y, 0 }, up_);
			proj_ = tnl::Matrix::OrthoLH((float)screen_w_, (float)screen_h_, 1, 1000);
		}

	}

	//------------------------------------------------------------------------------------------------------------
	tnl::Vector3 Camera::getFlustumNormal(eFlustum flustum) {

		if (flustum == eFlustum::Near) return forward();
		if (flustum == eFlustum::Far) return -forward();

		tnl::Vector3 cross_v[4];
		cross_v[static_cast<int>(eFlustum::Left)] = tnl::Vector3::Cross(forward(), left());
		cross_v[static_cast<int>(eFlustum::Right)] = -tnl::Vector3::Cross(forward(), left());
		cross_v[static_cast<int>(eFlustum::Bottom)] = right();
		cross_v[static_cast<int>(eFlustum::Top)] = left();

		tnl::Vector2i screen_v[4] = {
			{ 0,					DXE_WINDOW_HEIGHT / 2 },
			{ DXE_WINDOW_WIDTH,		DXE_WINDOW_HEIGHT / 2 },
			{ DXE_WINDOW_WIDTH / 2,	DXE_WINDOW_HEIGHT },
			{ DXE_WINDOW_WIDTH / 2,	0 }
		};

		tnl::Vector3 ray_nml = tnl::Vector3::CreateScreenRay(
			(float)(screen_v[static_cast<int>(flustum)].x),
			(float)(screen_v[static_cast<int>(flustum)].y),
			screen_w_, screen_h_, view_, proj_);
		return tnl::Vector3::Cross(ray_nml, cross_v[static_cast<int>(flustum)]);
	}


	//------------------------------------------------------------------------------------------------------------
	void Camera::render( float scale, uint32_t color ) const {

		// オブジェクトのワールド行列の作成
		tnl::Matrix mt_trans;
		tnl::Matrix mt_rot;
		tnl::Matrix mt_obj_world;
		mt_trans = tnl::Matrix::Translation(TNL_DEP_V3(position_));

		tnl::Quaternion rot = tnl::Quaternion::LookAt(position_, target_, up_);
		mt_rot = rot.getMatrix();
		MATRIX im;
		mt_obj_world = mt_rot * mt_trans;
		memcpy(im.m, mt_obj_world.m, sizeof(float) * 16);
		SetTransformToWorld(&im);

		float w = 10 * 0.5f * scale;
		float h = 10 * 0.5f * scale;
		float d = 20 * 0.5f * scale;
		{
			tnl::Vector3 v[8] = {
				{ -w,  h,  d }, {  w,  h,  d },  {  w,  h,  -d }, { -w,  h, -d },
				{ -w, -h,  d }, {  w, -h,  d },  {  w, -h,  -d }, { -w, -h, -d }
			};
			uint32_t col = color;
			for (int i = 0; i < 3; ++i) DrawLine3D({ v[i].x, v[i].y , v[i].z }, { v[i + 1].x, v[i + 1].y , v[i + 1].z }, col);
			for (int i = 0; i < 3; ++i) DrawLine3D({ v[4 + i].x, v[4 + i].y , v[4 + i].z }, { v[4 + i + 1].x, v[4 + i + 1].y , v[4 + i + 1].z }, col);
			for (int i = 0; i < 4; ++i) DrawLine3D({ v[i].x, v[i].y , v[i].z }, { v[i + 4].x, v[i + 4].y , v[i + 4].z }, col);
			DrawLine3D({ v[3].x, v[3].y , v[3].z }, { v[0].x, v[0].y , v[0].z }, col);
			DrawLine3D({ v[7].x, v[7].y , v[7].z }, { v[4].x, v[4].y , v[4].z }, col);
		}

		float ds = 5 * scale;
		float d2 = d + ds ;
		float d3 = d ;
		float ww = w / 2;
		float hh = h / 2;
		{
			tnl::Vector3 v[8] = {
				{ -w,  h,  d2 }, {  w,  h,  d2 },  {  ww,  hh,  d3 }, { -ww,  hh, d3 },
				{ -w, -h,  d2 }, {  w, -h,  d2 },  {  ww, -hh,  d3 }, { -ww, -hh, d3 }
			};
			uint32_t col = color;
			for (int i = 0; i < 3; ++i) DrawLine3D({ v[i].x, v[i].y , v[i].z }, { v[i + 1].x, v[i + 1].y , v[i + 1].z }, col);
			for (int i = 0; i < 3; ++i) DrawLine3D({ v[4 + i].x, v[4 + i].y , v[4 + i].z }, { v[4 + i + 1].x, v[4 + i + 1].y , v[4 + i + 1].z }, col);
			for (int i = 0; i < 4; ++i) DrawLine3D({ v[i].x, v[i].y , v[i].z }, { v[i + 4].x, v[i + 4].y , v[i + 4].z }, col);
			DrawLine3D({ v[3].x, v[3].y , v[3].z }, { v[0].x, v[0].y , v[0].z }, col);
			DrawLine3D({ v[7].x, v[7].y , v[7].z }, { v[4].x, v[4].y , v[4].z }, col);
		}

	}

}