#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "dxe.h"
#include "dxe_data.h"
#include "dxe_build_setting.h"

namespace dxe {

	int32_t g_statck_string_max_width = 0;
	tnl::Vector2i g_statck_string_position;

	std::string g_drag_file_path;
	float g_delta_time = 0;
	float g_unlimit_delta_time = 0;
	uint32_t g_delta_sum_integer_part = 0;
	float g_delta_sum_fractional_part = 0;

	int g_dotted_line_image_1 = 0;
	int g_dotted_line_image_2 = 0;

	int g_application_start_frame_count = 0;
	std::unordered_map<std::string, const char*> ref_file_path_dictionary ;
	std::unordered_map<std::string, const char*> ref_font_name_to_path_dictionary ;


	//------------------------------------------------------------------------------------------------------------------------------
	void Initialize(HINSTANCE hinstance, HWND hwnd, HDC hdc, uint32_t window_w, uint32_t window_h) {

		// メルセンヌツイスター初期化
		tnl::SetSeedMersenneTwister32(static_cast<int>(time(0) % INT_MAX));

		// tnl 入力制御初期化
		tnl::Input::Initialize(hinstance, hwnd, hdc, DXE_WINDOW_WIDTH, DXE_WINDOW_HEIGHT);

		// dxe 入力制御 初期化
		dxe::Input::Initialize();

		// dxe ウィンドウモード設定 ( DirectX 初期化設定 )
		dxe::SetWindowMode(DxLib::GetWindowModeFlag());

		// DXライブラリ拡張 DirectX 初期化
		dxe::DirectXInitialize();

		// ImGui 初期化
		dxe::ImGuiInitialize(
			ImGuiConfigFlags_NavEnableKeyboard
			| ImGuiConfigFlags_ViewportsEnable
		);


		const int PIXEL_NUM = 2500;
		unsigned char img1_buff[4 * PIXEL_NUM] = { 0 };
		int* p = (int*)img1_buff;
		for (int i = 0; i < (PIXEL_NUM / 5); ++i) {
			*p = (0 == (i % 2)) ? -1 : 0; ++p;
			*p = (0 == (i % 2)) ? -1 : 0; ++p;
			*p = (0 == (i % 2)) ? -1 : 0; ++p;
			*p = (0 == (i % 2)) ? -1 : 0; ++p;
			*p = (0 == (i % 2)) ? -1 : 0; ++p;
		}
		uint32_t size = 0;
		auto col_buff = tnl::CreateFormatTga32(img1_buff, PIXEL_NUM, 1, &size);
		g_dotted_line_image_1 = CreateGraphFromMem((void*)col_buff.get(), size);

		unsigned char img2_buff[4 * PIXEL_NUM] = { 0 };
		p = (int*)img2_buff;
		*p = -1; ++p;
		memcpy(p, ((int*)img1_buff) + 5, sizeof(int) * (PIXEL_NUM - 6));
		col_buff = tnl::CreateFormatTga32(img2_buff, PIXEL_NUM, 1, &size);
		g_dotted_line_image_2 = CreateGraphFromMem((void*)col_buff.get(), size);

	}

	//------------------------------------------------------------------------------------------------------------------------------
	void Release() {
		DeleteGraph(g_dotted_line_image_1);
		DeleteGraph(g_dotted_line_image_2);
		dxe::ImGuiRelease();
		dxe::Input::Destroy();
		dxe::Texture::DestroyAllTextures();
		dxe::DirectXRelease();
	}

	//------------------------------------------------------------------------------------------------------------------------------
	void Update( float delta_time ) {
		// ゲームメインルーチン
		dxe::SetDeltaTime(delta_time);

		g_delta_sum_fractional_part += delta_time ;
		while (g_delta_sum_fractional_part >= 1.0f) {
			float int_part = 0;
			g_delta_sum_fractional_part = std::modf(g_delta_sum_fractional_part, &int_part);
			g_delta_sum_integer_part += static_cast<uint32_t>(int_part);
		}

		// キー状態の更新
		tnl::Input::Update();
		dxe::Input::Update();

		// ウインドウモード切替
		if (tnl::Input::IsKeyDown(eKeys::KB_LALT, eKeys::KB_RALT)) {
			if (tnl::Input::IsKeyDownTrigger(eKeys::KB_RETURN)) {
				dxe::SetWindowMode(!dxe::GetWindowMode());
			}
		}

		g_statck_string_max_width = 0;
		g_statck_string_position = { 0, 0 };
	}

	//------------------------------------------------------------------------------------------------------------------------------
	bool IsRunApplication() {
		if (g_application_start_frame_count < DXE_APPLICATION_START_FRAME) {
			g_application_start_frame_count++;
			return false;
		}
		return true;
	}

	//------------------------------------------------------------------------------------------------------------------------------
	uint32_t GetDeltaSumIntegerPart() {
		return g_delta_sum_integer_part;
	}

	//------------------------------------------------------------------------------------------------------------------------------
	float GetDeltaSumFractionalPart() {
		return g_delta_sum_fractional_part;
	}


	//------------------------------------------------------------------------------------------------------------------------------
	void GetRectOriginLocatePosition(float& x, float& y, int size_x, int size_y, eRectOrigin e_location) {
		float sx = x - (size_x * 0.5f);
		float sy = y - (size_y * 0.5f);

		switch (e_location) {
		case eRectOrigin::LEFT_TOP:
			sx += (size_x * 0.5f);
			sy += (size_y * 0.5f);
			break;
		case eRectOrigin::LEFT_CENTER:
			sx += (size_x * 0.5f);
			break;
		case eRectOrigin::LEFT_BOTTOM:
			sx += (size_x * 0.5f);
			sy -= (size_y * 0.5f);
			break;
		case eRectOrigin::CENTER_TOP:
			sy += (size_y * 0.5f);
			break;
		case eRectOrigin::CENTER_BOTTOM:
			sy -= (size_y * 0.5f);
			break;
		case eRectOrigin::RIGHT_TOP:
			sx -= (size_x * 0.5f);
			sy += (size_y * 0.5f);
			break;
		case eRectOrigin::RIGHT_CENTER:
			sx -= (size_x * 0.5f);
			break;
		case eRectOrigin::RIGHT_BOTTOM:
			sx -= (size_x * 0.5f);
			sy -= (size_y * 0.5f);
			break;
		}
		x = sx;
		y = sy;
	}

	//------------------------------------------------------------------------------------------------------------------------------
	tnl::Vector2f ComputeRectPositionByOrigin(const tnl::Vector2f& center, const tnl::Vector2f& rect_size, dxe::eRectOrigin origin) {
		float x = rect_size.x * 0.5f;
		float y = rect_size.y * 0.5f;
		tnl::Vector2f offset[static_cast<int32_t>(dxe::eRectOrigin::MAX)] = {
			{-x, -y}, {-x, 0}, {-x, y}
			, {0, -y}, {0, 0}, {0, y}
			, {x, -y}, {x, 0}, {x, y}
		};
		return center + offset[static_cast<int32_t>(origin)];
	}


	//------------------------------------------------------------------------------------------------------------------------------
	void DrawString(const tnl::Vector2f& pos, int color, eRectOrigin e_location, const char* str, ...) {

		char buff[1024] = { 0 };
		va_list argptr;
		va_start(argptr, str);
		vsprintf_s(buff, str, argptr);
		va_end(argptr);

		int font_size = GetFontSize();
		int size_x, size_y, line_count;
		GetDrawStringSize(&size_x, &size_y, &line_count, buff, (int)strlen(buff));

		float sx = pos.x, sy = pos.y;
		GetRectOriginLocatePosition(sx, sy, size_x, size_y, e_location);

		//DrawBox(pos, 5, 5, true, dxe::Colors::Red);

		DrawFormatStringF(sx, sy, color, buff);
	}

	//------------------------------------------------------------------------------------------------------------------------------
	void DrawStringToHandle(const tnl::Vector2f& pos, int color, int font_hdl, eRectOrigin e_location, const char* str, ...) {
		char buff[1024] = { 0 };
		va_list argptr;
		va_start(argptr, str);
		vsprintf_s(buff, str, argptr);
		va_end(argptr);

		int font_size = GetFontSize();
		int size_x, size_y, line_count;
		GetDrawStringSizeToHandle(&size_x, &size_y, &line_count, buff, (int)strlen(buff), font_hdl);

		float sx = pos.x, sy = pos.y;
		GetRectOriginLocatePosition(sx, sy, size_x, size_y, e_location);

		DxLib::DrawStringFToHandle(sx, sy, buff, color, font_hdl, GetColor(255, 0, 0));
	}

	//------------------------------------------------------------------------------------------------------------------------------
	void DrawStackString(int color, const char* str, ...) {
		char buff[1024] = { 0 };
		va_list argptr;
		va_start(argptr, str);
		vsprintf_s(buff, str, argptr);
		va_end(argptr);

		int font_size = GetFontSize();
		int size_x, size_y, line_count;
		GetDrawStringSize(&size_x, &size_y, &line_count, buff, (int)strlen(buff));

		if (size_x > g_statck_string_max_width) {
			g_statck_string_max_width = size_x;
		}

		if (g_statck_string_position.y + size_y >= DXE_WINDOW_HEIGHT) {
			g_statck_string_position.y = 0;
			g_statck_string_position.x += g_statck_string_max_width + font_size;
			g_statck_string_max_width = 0;
		}

		DrawFormatString(g_statck_string_position.x, g_statck_string_position.y, color, buff);

		g_statck_string_position.y += size_y;

	}


	//------------------------------------------------------------------------------------------------------------------------------
	void DrawFpsIndicator(const tnl::Vector2i& pos) {
		static float sum_time = 0;
		static float sum_fps = 0;
		static float fps_rate = 0;
		static float unlim_sum_fps = 0;
		static float unlim_fps = 0;

		static float width = 0;
		static int call_num = 0;
		static int font_hdl = 0;

		if (0 == font_hdl) {
			font_hdl = CreateFontToHandle(_T("DrawFpsIndicator"), 10, 9, DX_FONTTYPE_NORMAL, GetDefaultFontHandle());
		}

		sum_time += dxe::GetDeltaTime();
		sum_fps += GetFPS();
		g_unlimit_delta_time = ( g_unlimit_delta_time > FLT_EPSILON) ? g_unlimit_delta_time : 1.0f ;
		unlim_sum_fps += 1000.0f / g_unlimit_delta_time;
		call_num++;
		if (sum_time > 0.1f) {
			fps_rate = sum_fps / call_num;
			fps_rate /= DXE_FIX_FPS;
			sum_time = 0;
			sum_fps = 0;
			unlim_fps = unlim_sum_fps / (float)call_num;
			unlim_sum_fps = 0;
			call_num = 0;
		}
		width += ((198 * fps_rate) - width) * 0.05f;
		int x = pos.x;
		int y = pos.y;
		DxLib::DrawBox(x, y, x + 320, y + 10, 0, true);
		DxLib::DrawBox(x + 1, y + 1, x + 199, y + 9, -1, false);
		DxLib::DrawBox(x + 2, y + 2, x + (int)width, y + 8, 0xff77ff77, true);

		dxe::DrawStringToHandle(tnl::Vector2f( (float)pos.x + 210.0f, (float)pos.y), -1, font_hdl, eRectOrigin::LEFT_TOP, "%.1f / %.1f fps", DXE_FIX_FPS * fps_rate, unlim_fps);
	}


	//------------------------------------------------------------------------------------------------------------------------------
	void DrawLine(const tnl::Vector2f& p, const tnl::Vector2f& v, const float length, const int color) {
		tnl::Vector2f n = v;
		n.normalize();
		n *= length;
		DxLib::DrawLine((int)p.x, (int)p.y, (int)p.x + (int)n.x, (int)p.y + (int)n.y, color, false);
	}

	//------------------------------------------------------------------------------------------------------------------------------
	//void DrawArrow(const tnl::Vector2f& root_position, const tnl::Vector2f& dir, float size, int color) {

	//	int sx = (int)root_position.x;
	//	int sy = (int)root_position.y;
	//	int ex = (int)(root_position.x + dir.x * size);
	//	int ey = (int)(root_position.y + dir.y * size);

	//	DxLib::DrawLine(sx, sy, ex, ey, color);

	//	tnl::Vector3 s = { (float)sx, (float)(sy), 0 };
	//	tnl::Vector3 e = { (float)ex, (float)(ey), 0 };
	//	tnl::Vector3 ves = s - e;
	//	ves.normalize();

	//	tnl::Vector3 va = tnl::Vector3::TransformCoord(ves, tnl::Quaternion::RotationAxis({ 0, 0, 1 }, tnl::ToRadian(45)));
	//	tnl::Vector3 vb = tnl::Vector3::TransformCoord(ves, tnl::Quaternion::RotationAxis({ 0, 0, 1 }, tnl::ToRadian(-45)));
	//	va *= size * 0.45f;
	//	vb *= size * 0.45f;

	//	sx = ex;
	//	sy = ey;
	//	ex = sx + (int)va.x;
	//	ey = sy + (int)va.y;
	//	DxLib::DrawLine(sx, sy, ex, ey, color);
	//	ex = sx + (int)vb.x;
	//	ey = sy + (int)vb.y;
	//	DxLib::DrawLine(sx, sy, ex, ey, color);
	//}

	//------------------------------------------------------------------------------------------------------------------------------
	void DrawArrow(const tnl::Vector2f& start, float length, const tnl::Vector2f& v, int color, float umbrella_length) {
		float umbrella = (umbrella_length < 0) ? length * 0.3f : umbrella_length;
		tnl::Vector3 v1 = tnl::Vector3(v);
		tnl::Vector2f v2 = tnl::Vector2f(v1.cross({ 0, 0,  1 }));
		tnl::Vector2f v3 = tnl::Vector2f(v1.cross({ 0, 0, -1 }));
		tnl::Vector2f end = start + tnl::Vector2f::Normalize(v) * length;
		v2 += tnl::Vector2f::Normalize(start - end) * 2.0f;
		v3 += tnl::Vector2f::Normalize(start - end) * 2.0f;
		v2.normalize();
		v3.normalize();
		dxe::DrawLine(start, v, length, color);
		dxe::DrawLine(end, v2, umbrella, color);
		dxe::DrawLine(end, v3, umbrella, color);
	}

	//------------------------------------------------------------------------------------------------------------------------------
	void DrawArrow(const tnl::Vector2f& start, const tnl::Vector2f& end, int color, float umbrella_length) {
		tnl::Vector2f to_end = end - start;
		float length = to_end.length();
		DrawArrow(start, length, tnl::Vector2f::Normalize(to_end), color, umbrella_length);
	}

	//------------------------------------------------------------------------------------------------------------------------------
	void DrawBox(const tnl::Vector2f& p, const float width, const float height, const bool fill, const int color) {
		float lx = p.x - width * 0.5f;
		float rx = p.x + width * 0.5f;
		float up = p.y - height * 0.5f;
		float down = p.y + height * 0.5f;
		DxLib::DrawBox((int)lx, (int)up, (int)rx, (int)down, color, fill);
	}

	//------------------------------------------------------------------------------------------------------------------------------
	void DrawRotaBox(const tnl::Vector2f& pos, float w, float h, float rad, int color) {

		tnl::Vector3 pos3 = { pos.x, pos.y, 0 };
		float hw = w / 2.0f;
		float hh = h / 2.0f;
		tnl::Vector3 v[4];
		tnl::Quaternion q = tnl::Quaternion::RotationAxis({ 0, 0, 1 }, rad);
		v[0] = tnl::Vector3(-hw, -hh, 0);
		v[1] = tnl::Vector3(hw, -hh, 0);
		v[2] = tnl::Vector3(hw, hh, 0);
		v[3] = tnl::Vector3(-hw, hh, 0);
		v[0] = pos3 + tnl::Vector3::TransformCoord(v[0], q);
		v[1] = pos3 + tnl::Vector3::TransformCoord(v[1], q);
		v[2] = pos3 + tnl::Vector3::TransformCoord(v[2], q);
		v[3] = pos3 + tnl::Vector3::TransformCoord(v[3], q);

		DxLib::DrawLine((int)v[0].x, (int)v[0].y, (int)v[1].x, (int)v[1].y, color);
		DxLib::DrawLine((int)v[1].x, (int)v[1].y, (int)v[2].x, (int)v[2].y, color);
		DxLib::DrawLine((int)v[2].x, (int)v[2].y, (int)v[3].x, (int)v[3].y, color);
		DxLib::DrawLine((int)v[3].x, (int)v[3].y, (int)v[0].x, (int)v[0].y, color);

	}

	//------------------------------------------------------------------------------------------------------------------------------
	void DrawHexagon(const tnl::Vector2f& pos, float width, int color ) {

		// 正六角形に収まる円の半径
		float radius = width / 2;
		// 円を囲う正六角形の辺の長さ
		float edge = radius / sqrt(3.0f) * 2.0f;
		// 正六角形の中心からの高さ
		float height = tnl::Vector2f(-radius, edge * 0.5f).length();

		tnl::Vector2f vertex[6];
		// ６角形の左上
		vertex[0] = pos + tnl::Vector2f(-radius, edge * 0.5f);
		// ６角形の上
		vertex[1] = pos + tnl::Vector2f(0, height);
		// ６角形の右上
		vertex[2] = pos + tnl::Vector2f(radius, edge * 0.5f);
		// ６角形の右下
		vertex[3] = pos + tnl::Vector2f(radius, -edge * 0.5f);
		// ６角形の下
		vertex[4] = pos + tnl::Vector2f(0, -height);
		// ６角形の左下
		vertex[5] = pos + tnl::Vector2f(-radius, -edge * 0.5f);

		//DrawCircle(pos.x, pos.y, radius, 0xffff00ff, false);
		for (int i = 0; i < 5; ++i) {
			DxLib::DrawLine((int)vertex[i].x, (int)vertex[i].y, (int)vertex[i + 1].x, (int)vertex[i + 1].y, color);
		}
		DxLib::DrawLine((int)vertex[5].x, (int)vertex[5].y, (int)vertex[0].x, (int)vertex[0].y, color);
	}

	//------------------------------------------------------------------------------------------------------------------------------
	void DrawCapsule(const Shared<dxe::Camera>& camera, const tnl::Vector3& s, const tnl::Vector3& e, float radius, const int color)
	{

		MATRIX view, proj;
		memcpy(view.m, camera->getViewMatrix().m, sizeof(float) * 16);
		memcpy(proj.m, camera->getProjectionMatrix().m, sizeof(float) * 16);
		SetCameraViewMatrix(view);
		SetupCamera_ProjectionMatrix(proj);

		// オブジェクトのワールド行列の作成
		MATRIX im = MGetIdent();
		SetTransformToWorld(&im);

		tnl::Vector3 normal = tnl::Vector3::Normalize(e - s);
		tnl::Vector3 cv = tnl::Vector3::Cross(normal, tnl::Vector3::left());
		if (cv.length() <= FLT_EPSILON) cv = tnl::Vector3::Cross(normal, tnl::Vector3::up());
		if (cv.length() <= FLT_EPSILON) cv = tnl::Vector3::Cross(normal, tnl::Vector3::forward());

		const int BODY_V = 8;

		// 胴体の描画
		tnl::Quaternion q = tnl::Quaternion::LookAt({ 0, 0, 0 }, cv, normal);
		tnl::Vector3 v[BODY_V];
		for (int i = 0; i < BODY_V; ++i) {
			v[i] = tnl::Vector3::TransformCoord({ 0, 0, 1 }, q);
			q *= tnl::Quaternion::RotationAxis(normal, tnl::ToRadian(360.0f / (float)BODY_V));
		}
		tnl::Vector3 start;
		tnl::Vector3 end;
		for (int i = 0; i < BODY_V; ++i) {
			start = (s + v[i] * radius);
			end = (e + v[i] * radius);
			DrawLine3D({ TNL_DEP_V3(start) }, { TNL_DEP_V3(end) }, color);
		}

		tnl::Vector3 iv;
		tnl::Vector3 axis;
		tnl::Vector3 lv[BODY_V + 1];

		// 上の半球描画
		for (int k = 0; k < BODY_V; ++k) {
			q = tnl::Quaternion::LookAt({ 0, 0, 0 }, cv, normal);
			iv = tnl::Vector3::InverseTransformCoord(v[k], q);
			axis = tnl::Vector3::Cross(v[k], normal);

			for (int i = 0; i < BODY_V + 1; ++i) {
				lv[i] = tnl::Vector3::TransformCoord(iv, q);
				q *= tnl::Quaternion::RotationAxis(axis, tnl::ToRadian(180.0f / (float)BODY_V));
			}
			for (int i = 0; i < BODY_V; ++i) {
				start = e + lv[i] * radius;
				end = e + lv[i + 1] * radius;
				DrawLine3D({ TNL_DEP_V3(start) }, { TNL_DEP_V3(end) }, color);
			}
		}

		// 下の半球描画
		for (int k = 0; k < BODY_V; ++k) {

			q = tnl::Quaternion::LookAt({ 0, 0, 0 }, cv, normal);
			iv = tnl::Vector3::InverseTransformCoord(v[k], q);
			axis = tnl::Vector3::Cross(v[k], normal);

			for (int i = 0; i < BODY_V + 1; ++i) {
				lv[i] = tnl::Vector3::TransformCoord(iv, q);
				q *= tnl::Quaternion::RotationAxis(axis, -tnl::ToRadian(180.0f / (float)BODY_V));
			}
			for (int i = 0; i < BODY_V; ++i) {
				start = s + lv[i] * radius;
				end = s + lv[i + 1] * radius;
				DrawLine3D({ TNL_DEP_V3(start) }, { TNL_DEP_V3(end) }, color);
			}
		}

	}


	//------------------------------------------------------------------------------------------------------------------------------
	void DrawCylinder(const Shared<dxe::Camera>& camera, const tnl::Vector3& s, const tnl::Vector3& e, float radius, const int color)
	{

		MATRIX view, proj;
		memcpy(view.m, camera->getViewMatrix().m, sizeof(float) * 16);
		memcpy(proj.m, camera->getProjectionMatrix().m, sizeof(float) * 16);
		SetCameraViewMatrix(view);
		SetupCamera_ProjectionMatrix(proj);

		// オブジェクトのワールド行列の作成
		MATRIX im = MGetIdent();
		SetTransformToWorld(&im);

		tnl::Vector3 normal = tnl::Vector3::Normalize(e - s);
		tnl::Vector3 cv = tnl::Vector3::Cross(normal, tnl::Vector3::left());
		if (cv.length() <= FLT_EPSILON) cv = tnl::Vector3::Cross(normal, tnl::Vector3::up());
		if (cv.length() <= FLT_EPSILON) cv = tnl::Vector3::Cross(normal, tnl::Vector3::forward());

		const int BODY_V = 8;

		// 胴体の描画
		tnl::Quaternion q = tnl::Quaternion::LookAt({ 0, 0, 0 }, cv, normal);
		tnl::Vector3 v[BODY_V];
		for (int i = 0; i < BODY_V; ++i) {
			v[i] = tnl::Vector3::TransformCoord({ 0, 0, 1 }, q);
			q *= tnl::Quaternion::RotationAxis(normal, tnl::ToRadian(360.0f / (float)BODY_V));
		}
		tnl::Vector3 start;
		tnl::Vector3 end;
		for (int i = 0; i < BODY_V; ++i) {
			start = (s + v[i] * radius);
			end = (e + v[i] * radius);
			DrawLine3D({ TNL_DEP_V3(start) }, { TNL_DEP_V3(end) }, color);
		}

		const int RING_V = 16;
		tnl::Vector3 rv[RING_V];
		q = tnl::Quaternion::LookAt({ 0, 0, 0 }, cv, normal);
		for (int i = 0; i < RING_V; ++i) {
			rv[i] = tnl::Vector3::TransformCoord({ 0, 0, 1 }, q);
			q *= tnl::Quaternion::RotationAxis(normal, tnl::ToRadian(360.0f / (float)RING_V));
		}
		for (int i = 0; i < RING_V; ++i) {
			start = (s + rv[i] * radius);
			end = (s + rv[(i + 1) % RING_V] * radius);
			DrawLine3D({ TNL_DEP_V3(start) }, { TNL_DEP_V3(end) }, color);
		}

		for (int i = 0; i < RING_V; ++i) {
			start = (e + rv[i] * radius);
			end = (e + rv[(i + 1) % RING_V] * radius);
			DrawLine3D({ TNL_DEP_V3(start) }, { TNL_DEP_V3(end) }, color);
		}

	}

	//------------------------------------------------------------------------------------------------------------------------------
	void DrawPlane(const Shared<dxe::Camera>& camera, const tnl::Vector3& p, const tnl::Vector3& pn, float size, int color, int circle_num ) {

		tnl::Vector3 cv = tnl::Vector3::Cross(pn, tnl::Vector3::left());
		if (cv.length() <= FLT_EPSILON) cv = tnl::Vector3::Cross(pn, tnl::Vector3::up());
		if (cv.length() <= FLT_EPSILON) cv = tnl::Vector3::Cross(pn, tnl::Vector3::forward());

		for (int32_t i = 0; i < circle_num; ++i) {
			float hh = size / (float)circle_num * (float)(i + 1);
			tnl::Quaternion q = tnl::Quaternion::LookAt(p, p - pn * 10, cv);
			for (int32_t k = 0; k <= 20; ++k) {
				tnl::Vector3 s = p + tnl::Vector3::TransformCoord({ 0, hh, 0 }, q);
				q *= tnl::Quaternion::RotationAxis(tnl::Vector3::TransformCoord({ 0,0,1 }, q), tnl::ToRadian(360.0f / 20.0f));
				tnl::Vector3 e = p + tnl::Vector3::TransformCoord({ 0, hh, 0 }, q);
				dxe::DrawLine(camera, s, e, color);
			}
		}

	}

	//------------------------------------------------------------------------------------------------------------------------------
	void DrawArrow(const Shared<dxe::Camera>& camera, const tnl::Vector3& root, const tnl::Vector3& dir, float length, const int color ) {

		MATRIX view, proj;
		memcpy(view.m, camera->getViewMatrix().m, sizeof(float) * 16);
		memcpy(proj.m, camera->getProjectionMatrix().m, sizeof(float) * 16);
		SetCameraViewMatrix(view);
		SetupCamera_ProjectionMatrix(proj);

		MATRIX im;
		CreateIdentityMatrix(&im);
		SetTransformToWorld(&im);

		tnl::Vector3 en = root + dir * length;
		DrawLine3D({ root.x, root.y, root.z }, { en.x, en.y, en.z }, color);

		tnl::Vector3 cv = tnl::Vector3::Cross(dir, tnl::Vector3::left());
		if (cv.length() <= FLT_EPSILON) cv = tnl::Vector3::Cross(dir, tnl::Vector3::up());
		if (cv.length() <= FLT_EPSILON) cv = tnl::Vector3::Cross(dir, tnl::Vector3::forward());

		tnl::Quaternion q = tnl::Quaternion::LookAt(en, root, cv);

		tnl::Vector3 v0 = tnl::Vector3::Normalize(tnl::Vector3::TransformCoord({ 1,0,3 }, q));
		tnl::Vector3 v1 = tnl::Vector3::Normalize(tnl::Vector3::TransformCoord({ -1,0,3 }, q));
		tnl::Vector3 v2 = tnl::Vector3::Normalize(tnl::Vector3::TransformCoord({ 0,1,3 }, q));
		tnl::Vector3 v3 = tnl::Vector3::Normalize(tnl::Vector3::TransformCoord({ 0,-1,3 }, q));

		tnl::Vector3 ev0 = en + v0 * length * 0.2f;
		tnl::Vector3 ev1 = en + v1 * length * 0.2f;
		tnl::Vector3 ev2 = en + v2 * length * 0.2f;
		tnl::Vector3 ev3 = en + v3 * length * 0.2f;

		DrawLine3D({ TNL_DEP_V3(en) }, { TNL_DEP_V3(ev0) }, color);
		DrawLine3D({ TNL_DEP_V3(en) }, { TNL_DEP_V3(ev1) }, color);
		DrawLine3D({ TNL_DEP_V3(en) }, { TNL_DEP_V3(ev2) }, color);
		DrawLine3D({ TNL_DEP_V3(en) }, { TNL_DEP_V3(ev3) }, color);

		DrawLine3D({ TNL_DEP_V3(ev0) }, { TNL_DEP_V3(ev2) }, color);
		DrawLine3D({ TNL_DEP_V3(ev1) }, { TNL_DEP_V3(ev2) }, color);
		DrawLine3D({ TNL_DEP_V3(ev1) }, { TNL_DEP_V3(ev3) }, color);
		DrawLine3D({ TNL_DEP_V3(ev3) }, { TNL_DEP_V3(ev0) }, color);

	}


	//------------------------------------------------------------------------------------------------------------------------------
	void DrawLine(const Shared<dxe::Camera>& camera, const tnl::Vector3& st, const tnl::Vector3& en, const int color ) {

		MATRIX view, proj;
		memcpy(view.m, camera->getViewMatrix().m, sizeof(float) * 16);
		memcpy(proj.m, camera->getProjectionMatrix().m, sizeof(float) * 16);
		SetCameraViewMatrix(view);
		SetupCamera_ProjectionMatrix(proj);

		MATRIX im;
		CreateIdentityMatrix(&im);
		SetTransformToWorld(&im);

		DrawLine3D({ st.x, st.y, st.z }, { en.x, en.y, en.z }, color );
	}

	//------------------------------------------------------------------------------------------------------------------------------
	void DrawrUledLine(const tnl::Vector2f& s, const tnl::Vector2f& e, float space, int color) {
		space = (space <= 3.0f) ? 3 : space;
		tnl::Vector2f p = s;
		tnl::Vector2f v = tnl::Vector2f::Normalize(e - s);
		tnl::Vector2f ss, ee;
		while (1) {
			ss = p;
			ee = p + v * space;
			DxLib::DrawLine((int)ss.x, (int)ss.y, (int)ee.x, (int)ee.y, color);
			p += (v * space * 2.0f);
			if ((p - s).length() > (e - s).length()) break;
			if ((p + (v * space) - s).length() > (e - s).length()) {
				DxLib::DrawLine((int)p.x, (int)p.y, (int)e.x, (int)e.y, color);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------
	void DrawrUledLine(const tnl::Vector2f& s, const tnl::Vector2f& dir, float length, float space, int color) {
		DrawrUledLine(s, s + tnl::Vector2f::Normalize(dir) * length, space, color);
	}

	//------------------------------------------------------------------------------------------------------------------------------
	void DrawUledBox(const tnl::Vector2f& lt, const tnl::Vector2f& rt, const tnl::Vector2f& lb, const tnl::Vector2f& rb, float space, int color) {
		DrawrUledLine(lt, rt, space, color);
		DrawrUledLine(rt, rb, space, color);
		DrawrUledLine(rb, lb, space, color);
		DrawrUledLine(lb, lt, space, color);
	}

	//------------------------------------------------------------------------------------------------------------------------------
	void DrawUledBox(const tnl::Vector2f& c, const tnl::Vector2f& size, float space, int color) {
		float hfw = size.x * 0.5f;
		float hfh = size.y * 0.5f;
		tnl::Vector2f lt = { c.x - hfw, c.y - hfh };
		tnl::Vector2f rt = { c.x + hfw, c.y - hfh };
		tnl::Vector2f lb = { c.x - hfw, c.y + hfh };
		tnl::Vector2f rb = { c.x + hfw, c.y + hfh };
		DrawUledBox(lt, rt, lb, rb, space, color);
	}


	//------------------------------------------------------------------------------------------------------------------------------
	void SetDottedColor(int color) {
		GraphFilter(g_dotted_line_image_1, DX_GRAPH_FILTER_BRIGHT_CLIP, DX_CMP_GREATER, 5, TRUE, color, 255);
		GraphFilter(g_dotted_line_image_2, DX_GRAPH_FILTER_BRIGHT_CLIP, DX_CMP_GREATER, 5, TRUE, color, 255);
	}

	//------------------------------------------------------------------------------------------------------------------------------
	void DrawDottedLine(const tnl::Vector2f& start, const tnl::Vector2f& end, bool emphasis) {
		tnl::Vector2f v = tnl::Vector2f::Normalize(end - start);
		float angle = v.angle({ 1.0f, 0.0f });
		angle = (start.y > end.y) ? -angle : angle;
		if (!emphasis) {
			DrawRectRotaGraph2F(start.x, start.y, 0, 0, (int)(start - end).length(), 1, 0, 0, 1.0f, angle, g_dotted_line_image_1, true);
			return;
		}
		if (GetDeltaSumFractionalPart() < 0.5f) {
			DrawRectRotaGraph2F(start.x, start.y, 0, 0, (int)(start - end).length(), 1, 0, 0, 1.0f, angle, g_dotted_line_image_1, true);
		}
		else {
			DrawRectRotaGraph2F(start.x, start.y, 0, 0, (int)(start - end).length(), 1, 0, 0, 1.0f, angle, g_dotted_line_image_2, true);
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------
	void DrawDottedLine(const tnl::Vector2f& start, const tnl::Vector2f& dir, float length, bool emphasis) {
		dxe::DrawDottedLine(start, start + tnl::Vector2f::Normalize(dir) * length, emphasis);
	}

	//------------------------------------------------------------------------------------------------------------------------------
	void DrawDottedBox(const tnl::Vector2f& lt, const tnl::Vector2f& rb, bool emphasis) {
		tnl::Vector2f rt = { rb.x, lt.y };
		tnl::Vector2f lb = { lt.x, rb.y };
		dxe::DrawDottedLine(lt, rt, emphasis);
		dxe::DrawDottedLine(rt, rb, emphasis);
		dxe::DrawDottedLine(rb, lb, emphasis);
		dxe::DrawDottedLine(lb, lt, emphasis);
	}

	//------------------------------------------------------------------------------------------------------------------------------
	void DrawDottedBox(const tnl::Vector2f& center, float width, float height, bool emphasis) {
		float hfw = width * 0.5f;
		float hfh = height * 0.5f;
		tnl::Vector2f lt = { center.x - hfw, center.y - hfh };
		tnl::Vector2f rb = { center.x + hfw, center.y + hfh };
		DrawDottedBox(lt, rb, emphasis);
	}

	//------------------------------------------------------------------------------------------------------------------------------
	void DrawDottedGrid(const tnl::Vector2f& position, const float square_size, int w_num, int h_num, const Shared<dxe::Camera>& camera) {

		if (!camera) {
			float l = square_size * h_num;
			for (int i = 0; i < w_num + 1; ++i) {
				dxe::DrawDottedLine({ position.x + i * square_size, 0 }, { 0, 1 }, l);
			}
			l = square_size * w_num;
			for (int i = 0; i < h_num + 1; ++i) {
				dxe::DrawDottedLine({ 0, position.y + i * square_size }, { 1, 0 }, l);
			}
		}
		else {
			float x = -camera->getPositionX() + (camera->getScreenWidth() * 0.5f);
			float y = -camera->getPositionY() + (camera->getScreenHeight() * 0.5f);
			float l = square_size * h_num;
			for (int i = 0; i < w_num + 1; ++i) {
				dxe::DrawDottedLine({ x + position.x + i * square_size, 0 }, { 0, 1 }, l);
			}
			l = square_size * w_num;
			for (int i = 0; i < h_num + 1; ++i) {
				dxe::DrawDottedLine({ 0, y + position.y + i * square_size }, { 1, 0 }, l);
			}
		}

	}

	//------------------------------------------------------------------------------------------------------------------------------
	void DrawGridGround(const Shared<dxe::Camera>& camera, const float square_size, int row_num, int color) {

		MATRIX view, proj;
		memcpy(view.m, camera->getViewMatrix().m, sizeof(float) * 16);
		memcpy(proj.m, camera->getProjectionMatrix().m, sizeof(float) * 16);
		SetCameraViewMatrix(view);
		SetupCamera_ProjectionMatrix(proj);

		MATRIX im;
		CreateIdentityMatrix(&im);
		SetTransformToWorld(&im);

		row_num = (1 == row_num % 2) ? row_num + 1 : row_num;

		float l = square_size * row_num * 0.5f;
		float n = -l;
		for (int i = 0; i < row_num + 1; ++i) {
			if ((row_num >> 1) == i) {
				DrawLine3D({ n, 0, l }, { n, 0, -l }, 0xff0000ff);
				DrawLine3D({ l, 0, n }, { -l, 0, n }, 0xffff0000);
			}
			else {
				DrawLine3D({ n, 0, l }, { n, 0, -l }, color);
				DrawLine3D({ l, 0, n }, { -l, 0, n }, color);
			}
			n += square_size;
		}
		DrawLine3D({ 0, l, 0 }, { 0, -l, 0 }, 0xff00ff00);

	}


	//----------------------------------------------------------------------------------------------------------------------------------------------------
	void DrawHexagonGround(const Shared<dxe::Camera>& camera, const float hex_width, int row_num, bool is_draw_center_line, int color) {

		MATRIX view, proj;
		memcpy(view.m, camera->getViewMatrix().m, sizeof(float) * 16);
		memcpy(proj.m, camera->getProjectionMatrix().m, sizeof(float) * 16);
		SetCameraViewMatrix(view);
		SetupCamera_ProjectionMatrix(proj);

		MATRIX im;
		CreateIdentityMatrix(&im);
		SetTransformToWorld(&im);


		float radius = hex_width / 2;
		float edge = radius / sqrt(3.0f) * 2.0f;
		float height = tnl::Vector3(-radius, edge * 0.5f, 0).length();

		tnl::Vector3 vertex[6];
		vertex[0] = tnl::Vector3(-radius, 0, edge * 0.5f);
		vertex[1] = tnl::Vector3(0, 0, height);
		vertex[2] = tnl::Vector3(radius, 0, edge * 0.5f);
		vertex[3] = tnl::Vector3(radius, 0, -edge * 0.5f);
		vertex[4] = tnl::Vector3(0, 0, -height);
		vertex[5] = tnl::Vector3(-radius, 0, -edge * 0.5f);

		tnl::Vector3 s = { -hex_width * row_num / 2, 0, (hex_width / 4) + height * row_num / 2 };

		for (int i = 0; i < row_num; ++i) {

			for (int k = 0; k < row_num; ++k) {

				float px = s.x + (k * radius * 2.0f);
				float pz = s.z - (i * height * 1.5f);
				// 偶数段は右にずらして描画
				px += (i % 2) * radius;

				tnl::Vector3 n = { px, 0, pz };

				for (int i = 0; i < 5; ++i) {
					DrawLine3D(DxLib::VECTOR( TNL_DEP_V3( ( n + vertex[i] )) ), DxLib::VECTOR(TNL_DEP_V3((n + vertex[i + 1]))), color);
				}
				DrawLine3D(DxLib::VECTOR(TNL_DEP_V3((n + vertex[5]))), DxLib::VECTOR(TNL_DEP_V3((n + vertex[0]))), color);
			}
		}

		if (!is_draw_center_line) return;
		DrawLine3D({ 0, 1000, 0 }, { 0, -1000, 0 }, 0xff00ff00);
		DrawLine3D({ -1000, 0, 0 }, { 1000, 0, 0 }, 0xffff0000);
		DrawLine3D({ 0, 0, 1000 }, { 0, 0, -1000 }, 0xff0000ff);

	}


	//------------------------------------------------------------------------------------------------------------------------------
	void DrawAxis(const Shared<dxe::Camera>& camera, const tnl::Vector3& pos, const tnl::Quaternion& rot, const float length)
	{
		MATRIX view, proj;
		memcpy(view.m, camera->getViewMatrix().m, sizeof(float) * 16);
		memcpy(proj.m, camera->getProjectionMatrix().m, sizeof(float) * 16);
		SetCameraViewMatrix(view);
		SetupCamera_ProjectionMatrix(proj);

		// オブジェクトのワールド行列の作成
		tnl::Matrix mt_trans;
		tnl::Matrix mt_rot;
		tnl::Matrix mt_obj_world;
		mt_trans = tnl::Matrix::Translation(pos.x, pos.y, pos.z);
		mt_rot = rot.getMatrix();
		MATRIX im;
		mt_obj_world = mt_rot * mt_trans;
		memcpy(im.m, mt_obj_world.m, sizeof(float) * 16);
		SetTransformToWorld(&im);

		tnl::Vector3 up(0, length, 0);
		tnl::Vector3 right(length, 0, 0);
		tnl::Vector3 depth(0, 0, length);
		DrawLine3D({ 0, 0, 0 }, { up.x, up.y, up.z }, 0xff00ff00);
		DrawLine3D({ 0, 0, 0 }, { right.x, right.y, right.z }, 0xffff0000);
		DrawLine3D({ 0, 0, 0 }, { depth.x, depth.y, depth.z }, 0xff0000ff);
	}


	//------------------------------------------------------------------------------------------------------------------------------
	void DrawOBB(const Shared<dxe::Camera>& camera, const tnl::Vector3& pos, const tnl::Vector3& size, const tnl::Quaternion& rot, const int color)
	{

		MATRIX view, proj;
		memcpy(view.m, camera->getViewMatrix().m, sizeof(float) * 16);
		memcpy(proj.m, camera->getProjectionMatrix().m, sizeof(float) * 16);
		SetCameraViewMatrix(view);
		SetupCamera_ProjectionMatrix(proj);

		// オブジェクトのワールド行列の作成
		tnl::Matrix mt_trans;
		tnl::Matrix mt_rot;
		tnl::Matrix mt_obj_world;
		mt_trans = tnl::Matrix::Translation(pos.x, pos.y, pos.z);
		mt_rot = rot.getMatrix();
		MATRIX im;
		mt_obj_world = mt_rot * mt_trans;
		memcpy(im.m, mt_obj_world.m, sizeof(float) * 16);
		SetTransformToWorld(&im);


		float w = size.x * 0.5f;
		float h = size.y * 0.5f;
		float d = size.z * 0.5f;
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


	//------------------------------------------------------------------------------------------------------------------------------
	void DrawAABB(const Shared<dxe::Camera>& camera, const tnl::Vector3& pos, const tnl::Vector3& size, const int color)
	{
		MATRIX view, proj;
		memcpy(view.m, camera->getViewMatrix().m, sizeof(float) * 16);
		memcpy(proj.m, camera->getProjectionMatrix().m, sizeof(float) * 16);
		SetCameraViewMatrix(view);
		SetupCamera_ProjectionMatrix(proj);

		// オブジェクトのワールド行列の作成
		tnl::Matrix mt_trans;
		tnl::Matrix mt_obj_world;
		mt_trans = tnl::Matrix::Translation(pos.x, pos.y, pos.z);
		MATRIX im;
		mt_obj_world = mt_trans;
		memcpy(im.m, mt_obj_world.m, sizeof(float) * 16);
		SetTransformToWorld(&im);

		float w = size.x * 0.5f;
		float h = size.y * 0.5f;
		float d = size.z * 0.5f;
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


	//------------------------------------------------------------------------------------------------------------------------------
	void DrawBdSphere(const Shared<dxe::Camera>& camera, const tnl::Vector3& pos, const float radius)
	{
		MATRIX view, proj;
		memcpy(view.m, camera->getViewMatrix().m, sizeof(float) * 16);
		memcpy(proj.m, camera->getProjectionMatrix().m, sizeof(float) * 16);
		SetCameraViewMatrix(view);
		SetupCamera_ProjectionMatrix(proj);

		// オブジェクトのワールド行列の作成
		tnl::Matrix mt_trans;
		tnl::Matrix mt_rot;
		tnl::Matrix mt_obj_world;
		mt_trans = tnl::Matrix::Translation(pos.x, pos.y, pos.z);
		MATRIX im;
		mt_obj_world = mt_rot * mt_trans;
		memcpy(im.m, mt_obj_world.m, sizeof(float) * 16);
		SetTransformToWorld(&im);

		const int ring_vtx_num = 16;
		int s = ring_vtx_num - 1;
		int e = 0;

		tnl::Vector3 vx[ring_vtx_num];
		tnl::Vector3 vy[ring_vtx_num];
		tnl::Vector3 vz[ring_vtx_num];

		// X軸リング
		for (int i = 0; i < ring_vtx_num; ++i) {
			vx[i].x = 0;
			vx[i].y = cos(tnl::ToRadian(360.0f / ring_vtx_num) * i) * radius;
			vx[i].z = sin(tnl::ToRadian(360.0f / ring_vtx_num) * i) * radius;
		}
		for (int i = 0; i < (ring_vtx_num - 1); ++i) {
			DrawLine3D({ vx[i].x, vx[i].y, vx[i].z },
				{ vx[i + 1].x, vx[i + 1].y, vx[i + 1].z }, 0xffff0000);
		}
		DrawLine3D({ vx[s].x, vx[s].y, vx[s].z },
			{ vx[e].x, vx[e].y, vx[e].z }, 0xffff0000);


		// Y軸リング
		for (int i = 0; i < ring_vtx_num; ++i) {
			vy[i].x = sin(tnl::ToRadian(360.0f / ring_vtx_num) * i) * radius;
			vy[i].y = 0;
			vy[i].z = cos(tnl::ToRadian(360.0f / ring_vtx_num) * i) * radius;
		}
		for (int i = 0; i < (ring_vtx_num - 1); ++i) {
			DrawLine3D({ vy[i].x, vy[i].y, vy[i].z },
				{ vy[i + 1].x, vy[i + 1].y, vy[i + 1].z }, 0xff00ff00);
		}
		DrawLine3D({ vy[s].x, vy[s].y, vy[s].z },
			{ vy[e].x, vy[e].y, vy[e].z }, 0xff00ff00);

		// Z軸リング
		for (int i = 0; i < ring_vtx_num; ++i) {
			vz[i].x = sin(tnl::ToRadian(360.0f / ring_vtx_num) * i) * radius;
			vz[i].y = cos(tnl::ToRadian(360.0f / ring_vtx_num) * i) * radius;
			vz[i].z = 0;
		}
		for (int i = 0; i < (ring_vtx_num - 1); ++i) {
			DrawLine3D({ vz[i].x, vz[i].y, vz[i].z },
				{ vz[i + 1].x, vz[i + 1].y, vz[i + 1].z }, 0xff2255ff);
		}
		DrawLine3D({ vz[s].x, vz[s].y, vz[s].z },
			{ vz[e].x, vz[e].y, vz[e].z }, 0xff2266ff);

	}

	//------------------------------------------------------------------------------------------------------------------------------
	std::string GetDragFilePathReadTrigger() {
		std::string path = g_drag_file_path;
		g_drag_file_path.clear();
		return path;
	}

	//------------------------------------------------------------------------------------------------------------------------------
	void SetFilePathDictionary(const std::unordered_map<std::string, const char*>& dictionary) {
		for (const auto& [key, value] : dictionary) {
			ref_file_path_dictionary[key] = value;
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------
	void SetFontNameToPathDictionary(const std::unordered_map<std::string, const char*>& dictionary) {
		for (const auto& [key, value] : dictionary) {
			ref_font_name_to_path_dictionary[key] = value;
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------
	bool IsEnableFilePath(const char* file_name) {
		return (ref_file_path_dictionary.end() != ref_file_path_dictionary.find(file_name) );
	}

	//------------------------------------------------------------------------------------------------------------------------------
	bool IsEnableFontPath(const char* font_name) {
		return (ref_font_name_to_path_dictionary.end() != ref_font_name_to_path_dictionary.find(font_name));
	}

	//------------------------------------------------------------------------------------------------------------------------------
	const char* GetPathFromFileName(const char* file_name) {
		auto it = ref_file_path_dictionary.find(file_name);
		if (it == ref_file_path_dictionary.end()) {
			std::string error_file = file_name;
			throw std::runtime_error(std::string("dxe::GetPathFromFileName Not Found Error : ") + error_file);
		}
		return it->second ;
	}

	//------------------------------------------------------------------------------------------------------------------------------
	const char* GetPathFromFontName(const char* font_name) {
		auto it = ref_font_name_to_path_dictionary.find(font_name);
		if (it == ref_font_name_to_path_dictionary.end()) {
			std::string error_file = font_name;
			throw std::runtime_error(std::string("dxe::GetFontPathFromName Not Found Error : ") + error_file);
		}
		return it->second;
	}


	//------------------------------------------------------------------------------------------------------------------------------
	void DrawDefaultLightGuiController() {

		class DefaultLightParam {
		public :
			DefaultLightParam() {
				DxLib::VECTOR dir = GetLightDirection();
				DxLib::COLOR_F diff = GetLightDifColor();
				DxLib::COLOR_F ambi = GetLightAmbColor();
				DxLib::COLOR_F spec = GetLightSpcColor();

				dir_ = { dir.x, dir.y, dir.z };
				diff_ = { diff.r, diff.g, diff.b };
				ambi_ = { ambi.r, ambi.g, ambi.b };
				spec_ = { spec.r, spec.g, spec.b };
			}
			tnl::Vector3 dir_ = { 1.0f, -1.0f, 1.0f };
			tnl::Vector3 diff_ = { 1.0f, 1.0f, 1.0f };
			tnl::Vector3 ambi_ = { 0.33f, 0.33f, 0.33f };
			tnl::Vector3 spec_ = { 1.0f, 1.0f, 1.0f };

			tnl::Vector3 getDir() { return dir_; }
			tnl::Vector3 getDiff() { return diff_; }
			tnl::Vector3 getAmbi() { return ambi_; }
			tnl::Vector3 getSpec() { return spec_; }

			void setDir(const tnl::Vector3& dir) { 
				dir_ = dir ; 
				SetLightDirection({ dir_.x, dir_.y, dir_.z });
			}
			void setDiff(const tnl::Vector3& diff) { 
				diff_ = diff ; 
				SetLightDifColor({ diff_.x, diff_.y, diff_.z, 1.0f });
			}
			void setAmbi(const tnl::Vector3& ambi) { 
				ambi_ = ambi ; 
				SetLightAmbColor({ ambi_.x, ambi_.y, ambi_.z, 1.0f });
			}
			void setSpec(const tnl::Vector3& spec) { 
				spec_ = spec; 
				SetLightSpcColor({ spec_.x, spec_.y, spec_.z, 1.0f });
			}

		};

		static bool is_btn_save_forcus_ = false;
		static bool is_btn_save_select_ = false;
		static Shared<DefaultLightParam> param = nullptr ;
		static Shared<dxe::GuiValueSlider< DefaultLightParam, tnl::Vector3 >> dir_ = nullptr;
		static Shared<dxe::GuiValueSlider< DefaultLightParam, tnl::Vector3 >> diff_ = nullptr ;
		static Shared<dxe::GuiValueSlider< DefaultLightParam, tnl::Vector3 >> ambi_ = nullptr ;
		static Shared<dxe::GuiValueSlider< DefaultLightParam, tnl::Vector3 >> global_emis_ = nullptr ;
		static Shared<dxe::GuiValueSlider< DefaultLightParam, tnl::Vector3 >> spec_ = nullptr ;
		if (!param) {
			param = std::make_shared<DefaultLightParam>();
			dir_  = Shared<dxe::GuiValueSlider< DefaultLightParam, tnl::Vector3 >>(
					new dxe::GuiValueSlider< DefaultLightParam, tnl::Vector3 >
					( param.get()
					, &DefaultLightParam::getDir
					, &DefaultLightParam::setDir
					, { DXE_WINDOW_WIDTH - 330, 30 }
					, { -1, -1, -1 }
					, { 1, 1, 1 }
					, "direction"));
			diff_ = Shared<dxe::GuiValueSlider< DefaultLightParam, tnl::Vector3 >>(
					new dxe::GuiValueSlider< DefaultLightParam, tnl::Vector3 >
					( param.get()
					, &DefaultLightParam::getDiff
					, &DefaultLightParam::setDiff
					, { DXE_WINDOW_WIDTH - 330, 90 }
					, { 0, 0, 0 }
					, { 1, 1, 1 }
					, "diffuse"));
			ambi_ = Shared<dxe::GuiValueSlider< DefaultLightParam, tnl::Vector3 >>(
					new dxe::GuiValueSlider< DefaultLightParam, tnl::Vector3 >
					( param.get()
					, &DefaultLightParam::getAmbi
					, &DefaultLightParam::setAmbi
					, { DXE_WINDOW_WIDTH - 330, 150 }
					, { 0, 0, 0 }
					, { 1, 1, 1 }
					, "ambient"));
			spec_ = Shared<dxe::GuiValueSlider< DefaultLightParam, tnl::Vector3 >>(
					new dxe::GuiValueSlider< DefaultLightParam, tnl::Vector3 >
					( param.get()
					, &DefaultLightParam::getSpec
					, &DefaultLightParam::setSpec
					, { DXE_WINDOW_WIDTH - 330, 210 }
					, { 0, 0, 0 }
					, { 1, 1, 1 }
					, "specular"));
		}

		dir_->update();
		diff_->update();
		ambi_->update();
		spec_->update();

		dir_->draw();
		diff_->draw();
		ambi_->draw();
		spec_->draw();

		tnl::Vector3 ms = tnl::Input::GetMousePosition();
		int tx = DXE_WINDOW_WIDTH - 150 ;
		int ty = 0;
		int bx = DXE_WINDOW_WIDTH ;
		int by = 20;

		is_btn_save_forcus_ = false;
		if (ms.x > tx && ms.x < bx && ms.y > ty && ms.y < by) {
			is_btn_save_forcus_ = true;
			if (tnl::Input::IsMouseTrigger(eMouseTrigger::IN_LEFT)) {
				is_btn_save_select_ = true;
			}
		}
		if (is_btn_save_select_) {
			if (tnl::Input::IsMouseTrigger(eMouseTrigger::OUT_LEFT)) {
				is_btn_save_select_ = false;

				FILE* fp = nullptr;
				fopen_s(&fp, "directional_light_parameter.bin", "wb");
				if (fp) {
					fwrite(&param->dir_, sizeof(tnl::Vector3), 1, fp);
					fwrite(&param->diff_, sizeof(tnl::Vector3), 1, fp);
					fwrite(&param->ambi_, sizeof(tnl::Vector3), 1, fp);
					fwrite(&param->spec_, sizeof(tnl::Vector3), 1, fp);
					fclose(fp);
				}
			}
		}

		int color = (is_btn_save_forcus_) ? -1 : 0x88888888;
		color = (is_btn_save_select_) ? 0xff00ff00 : color;
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
		DxLib::DrawBox(DXE_WINDOW_WIDTH - 330, ty, bx, by, 0, true);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
		DrawFormatString((DXE_WINDOW_WIDTH - 320), ty, -1, "light parameter");

		DxLib::DrawBox(tx - 5, ty, bx, by, color, false);
		DrawFormatString(tx, ty, color, "save to file");

	}


	//------------------------------------------------------------------------------------------------------------------------------
	void LockCursorToWindow() {
		HWND hwnd = GetMainWindowHandle(); // ウィンドウハンドルを取得
		RECT rect;
		GetClientRect(hwnd, &rect); // クライアント領域のサイズを取得

		POINT ul{};
		ul.x = rect.left;
		ul.y = rect.top;

		POINT lr{};
		lr.x = rect.right;
		lr.y = rect.bottom;

		MapWindowPoints(hwnd, nullptr, &ul, 1); // クライアント領域の左上のスクリーン座標への変換
		MapWindowPoints(hwnd, nullptr, &lr, 1); // クライアント領域の右下のスクリーン座標への変換

		RECT clipRect{};
		clipRect.left = ul.x;
		clipRect.top = ul.y;
		clipRect.right = lr.x;
		clipRect.bottom = lr.y;

		ClipCursor(&clipRect); // カーソルを制限
	}


	//------------------------------------------------------------------------------------------------------------------------------
	void SetDefaultLightParameter(const std::string& file_path) {

		tnl::Vector3 dir = { 1.0f, -1.0f, 1.0f };
		tnl::Vector3 diff = { 1.0f, 1.0f, 1.0f };
		tnl::Vector3 ambi = { 0.33f, 0.33f, 0.33f };
		tnl::Vector3 spec = { 1.0f, 1.0f, 1.0f };

		FILE* fp = nullptr;
		fopen_s(&fp, file_path.c_str(), "rb");
		if (fp) {
			fread(&dir, sizeof(tnl::Vector3), 1, fp);
			fread(&diff, sizeof(tnl::Vector3), 1, fp);
			fread(&ambi, sizeof(tnl::Vector3), 1, fp);
			fread(&spec, sizeof(tnl::Vector3), 1, fp);
			fclose(fp);

			SetLightDirection({ dir.x, dir.y, dir.z });
			SetLightDifColor({ diff.x, diff.y, diff.z, 1.0f });
			SetLightAmbColor({ ambi.x, ambi.y, ambi.z, 1.0f });
			SetLightSpcColor({ spec.x, spec.y, spec.z, 1.0f });
		}

	}

	void GetLookAtCameraFactor(const float axis_x_angle, const float axis_y_angle, const float distance, tnl::Vector3& out_target_to_position) {
		tnl::Quaternion  q = tnl::Quaternion::RotationAxis({ 0, 1, 0 }, axis_y_angle);
		tnl::Vector3 vz = tnl::Vector3::TransformCoord({ 0, 0, 1 }, q);
		q *= tnl::Quaternion::RotationAxis(tnl::Vector3::Cross({ 0, 1, 0 }, vz), axis_x_angle);
		out_target_to_position = tnl::Vector3::TransformCoord({ 0, 0, -distance }, q);
	}


	void GetFreeLookCameraFactor(const tnl::Vector3& position, const tnl::Vector3& target, float& out_axis_x_angle, float& out_axis_y_angle, float& out_distance) {
		tnl::Vector3 invlook = position - target;
		out_distance = invlook.length();
		tnl::Vector3 vx = tnl::Vector3::Normalize(invlook.xz());
		tnl::Vector3 vy = tnl::Vector3::Normalize(invlook);
		out_axis_y_angle = (invlook.x < 0) ? vx.angle({ 0, 0, -1 }) : -vx.angle({ 0, 0, -1 });
		out_axis_x_angle = (invlook.y < 0) ? -vy.angle(vx) : vy.angle(vx);
	}

	void GetTransformCameraFactor(const tnl::Vector3& position, const tnl::Vector3& target, const tnl::Vector3& up, tnl::Quaternion& out_rot) {
		out_rot = tnl::Quaternion::LookAt(position, target, up);
	}


#if 1

	static bool g_is_window_mode = false;
	static bool g_is_exit_application = false;
	static bool g_is_visible_mouse_pointer = true;

	static ComPtr<ID3D11DepthStencilState> g_state_depth_stencil[static_cast<int>(eDepthStenclil::MAX)] ;
	static ComPtr<ID3D11BlendState> g_blend_states[static_cast<int>(eBlendState::MAX)];
	static ComPtr<ID3D11SamplerState> g_sampler_states[static_cast<int>(eSamplerState::MAX)];
	static ComPtr<ID3D11RasterizerState> g_rasterizer_states[static_cast<int>(eRasterizerState::MAX)];

	ID3D11DepthStencilState* GetDepthStencilState(const eDepthStenclil mode) { return g_state_depth_stencil[static_cast<int>(mode)].Get(); }
	ID3D11BlendState* GetBlendState(const eBlendState state) { return g_blend_states[static_cast<int>(state)].Get(); }
	ID3D11SamplerState* GetSamplerState(const eSamplerState state) { return g_sampler_states[static_cast<int>(state)].Get(); }
	ID3D11RasterizerState* GetRasterizerState(const eRasterizerState state) { return g_rasterizer_states[static_cast<int>(state)].Get(); }


	void SetWindowMode(bool is_window) {
		g_is_window_mode = is_window;
		DxLib::ChangeWindowMode(g_is_window_mode);
		SetVisibleMousePointer(GetVisibleMousePointer());
	}
	bool GetWindowMode() {
		return g_is_window_mode;
	}
	void ExitApplication() {
		g_is_exit_application = true;
	}
	bool IsExitApplication() { return g_is_exit_application ; }

	void SetVisibleMousePointer(bool visible) {
		g_is_visible_mouse_pointer = visible;
		SetMouseDispFlag(g_is_visible_mouse_pointer);
	}

	bool GetVisibleMousePointer() {
		return g_is_visible_mouse_pointer ;
	}



	void CreateDepthStencil(const eDepthStenclil e_ds, const D3D11_DEPTH_WRITE_MASK depth_write_mask, bool depth_enable) {
		HRESULT hr = E_FAIL;
		ID3D11Device* pd3dDevice = (ID3D11Device*)DxLib::GetUseDirect3D11Device();
		D3D11_DEPTH_STENCIL_DESC dsState;
		::ZeroMemory(&dsState, sizeof(D3D11_DEPTH_STENCIL_DESC));
		dsState.DepthWriteMask = depth_write_mask;
		dsState.DepthFunc = D3D11_COMPARISON_LESS;
		dsState.DepthEnable = depth_enable;
		dsState.StencilEnable = FALSE;
		ID3D11DepthStencilState* dss = nullptr;
		hr = pd3dDevice->CreateDepthStencilState(&dsState, &dss);		
		g_state_depth_stencil[static_cast<int>(e_ds)].Attach(dss);
		if (S_OK != hr) {
			tnl::DebugTrace("Error : dxe::DirectXInitialize CreateDepthStencilState \n");
			return;
		}
	}

	void CreateBlendState(const eBlendState state) {
		HRESULT hr = E_FAIL;
		ID3D11Device* pd3dDevice = (ID3D11Device*)DxLib::GetUseDirect3D11Device();

		//-------------------------------------------------------------------------------------------------
		// ブレンドステートの作成
		D3D11_RENDER_TARGET_BLEND_DESC desc;
		desc.BlendEnable = TRUE;
		switch (state) {
		case eBlendState::NORMAL:
			desc.BlendEnable = FALSE;
			break;
		case eBlendState::ALPHA:
			desc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
			desc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			break;
		case eBlendState::ADD:
			desc.SrcBlend = D3D11_BLEND_ONE;
			desc.DestBlend = D3D11_BLEND_ONE;
			break;
		case eBlendState::SUB:
			desc.SrcBlend = D3D11_BLEND_ZERO;
			desc.DestBlend = D3D11_BLEND_INV_SRC_COLOR;
			break;
		case eBlendState::MUL:
			desc.SrcBlend = D3D11_BLEND_DEST_COLOR;
			desc.DestBlend = D3D11_BLEND_ZERO;
			break;
		}
		desc.BlendOp = D3D11_BLEND_OP_ADD;
		desc.SrcBlendAlpha = D3D11_BLEND_ONE;
		desc.DestBlendAlpha = D3D11_BLEND_ZERO;
		desc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		desc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		D3D11_BLEND_DESC blend_desc;
		::ZeroMemory(&blend_desc, sizeof(D3D11_BLEND_DESC));
		::CopyMemory(&blend_desc.RenderTarget[0], &desc, sizeof(D3D11_RENDER_TARGET_BLEND_DESC));
		ID3D11BlendState* bs = nullptr;
		hr = pd3dDevice->CreateBlendState(&blend_desc, &bs);
		g_blend_states[static_cast<int>(state)].Attach(bs);

		if (S_OK != hr) {
			tnl::DebugTrace("-----------------------------------------------------------------\n");
			tnl::DebugTrace("Error : Particle Create Blend State \n");
			tnl::DebugTrace("-----------------------------------------------------------------\n");
			return;
		}
	}


	void CreateSamplerState(const eSamplerState state) {

		HRESULT hr = E_FAIL;
		ID3D11Device* pd3dDevice = (ID3D11Device*)DxLib::GetUseDirect3D11Device();

		//-------------------------------------------------------------------------------------------------
		// サンプラーステートの作成
		D3D11_SAMPLER_DESC samplerDesc;
		::ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));

		switch (state) {
		case eSamplerState::ANISOTROPIC:
			samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;        
			samplerDesc.MaxAnisotropy = 16;                        // サンプリングに異方性補間を使用している場合の限界値。有効な値は 1 ～ 16 。
			break;
		case eSamplerState::BILINEAR:
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.MaxAnisotropy = 1;                        
			break;
		case eSamplerState::NEAREST:
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			samplerDesc.MaxAnisotropy = 1;
			break;
		}

		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;     // 0 ～ 1 の範囲外にある u テクスチャー座標の描画方法
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;     // 0 ～ 1 の範囲外にある v テクスチャー座標の描画方法
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;     // 0 ～ 1 の範囲外にある w テクスチャー座標の描画方法
		samplerDesc.MipLODBias = 0;                            // 計算されたミップマップ レベルからのバイアス
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;  // 比較オプション。

		samplerDesc.BorderColor[0] = 0;
		samplerDesc.BorderColor[1] = 0;
		samplerDesc.BorderColor[2] = 0;
		samplerDesc.BorderColor[3] = 0;

		samplerDesc.MinLOD = 0;                                // アクセス可能なミップマップの下限値
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;                // アクセス可能なミップマップの上限値
		ID3D11SamplerState* st = nullptr;
		hr = pd3dDevice->CreateSamplerState(&samplerDesc, &st);
		g_sampler_states[static_cast<int>(state)].Attach(st);

		if (S_OK != hr) {
			tnl::DebugTrace("-----------------------------------------------------------------\n");
			tnl::DebugTrace("Error : Instancing Create Sampler State \n");
			tnl::DebugTrace("-----------------------------------------------------------------\n");
			return;
		}


	}


	void CreateRasterizerState(const eRasterizerState state) {

		HRESULT hr = E_FAIL;
		ID3D11Device* pd3dDevice = (ID3D11Device*)DxLib::GetUseDirect3D11Device();

		ID3D11RasterizerState* rs = nullptr;
		D3D11_RASTERIZER_DESC desc = {};

		switch (state) {
		case eRasterizerState::CULL_NONE :
			desc.CullMode = D3D11_CULL_NONE;
			desc.FillMode = D3D11_FILL_SOLID;
			break;
		case eRasterizerState::CULL_FRONT:
			desc.CullMode = D3D11_CULL_FRONT;
			desc.FillMode = D3D11_FILL_SOLID;
			break;
		case eRasterizerState::CULL_BACK:
			desc.CullMode = D3D11_CULL_BACK;
			desc.FillMode = D3D11_FILL_SOLID;
			break;
		case eRasterizerState::WIREFRAME:
			desc.CullMode = D3D11_CULL_NONE;
			desc.FillMode = D3D11_FILL_WIREFRAME;
			break;
		}

		desc.FrontCounterClockwise = true;
		desc.ScissorEnable = false;
		desc.MultisampleEnable = false;
		hr = pd3dDevice->CreateRasterizerState(&desc, &rs);
		if (S_OK != hr) {
			tnl::DebugTrace("-----------------------------------------------------------------\n");
			tnl::DebugTrace("Error : Particle Create Rasterizer State \n");
			tnl::DebugTrace("-----------------------------------------------------------------\n");
			return;
		}
		g_rasterizer_states[static_cast<int>(state)].Attach(rs);

	}



	//------------------------------------------------------------------------------------------------------------------------------
	float GetDeltaTime() {
		return g_delta_time;
	}
	void SetDragFilePath(const std::string& drag_file_path) {
		g_drag_file_path = drag_file_path;
	}
	void SetDeltaTime(float delta_time) {
		g_delta_time = delta_time;
	}
	void SetUnlimitDeltaTime(float delta_time) {
		g_unlimit_delta_time = delta_time;
	}


	//------------------------------------------------------------------------------------------------------------------------------
	std::string GetOpenReadFilePath(const std::string& candidate_ext) {
		char fullPath[MAX_PATH] = { 0 };
		char fileName[MAX_PATH] = { 0 };

		OPENFILENAME ofn = {}; // ゼロ初期化
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = GetMainWindowHandle();

		// --- フィルタ文字列の構築 ---
		std::string filterDesc = candidate_ext;
		std::vector<char> filter;
		filter.insert(filter.end(), filterDesc.begin(), filterDesc.end());
		filter.push_back('\0');
		filter.insert(filter.end(), candidate_ext.begin(), candidate_ext.end());
		filter.push_back('\0');
		filter.push_back('\0'); // 終端はダブルヌル

		ofn.lpstrFilter = filter.data();
		ofn.lpstrFile = fullPath;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrFileTitle = fileName;
		ofn.nMaxFileTitle = MAX_PATH;
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR; // NOCHANGEDIR推奨
		ofn.lpstrTitle = "Import";

		// GetOpenFileNameを呼び出し
		// OFN_NOCHANGEDIR をフラグに入れれば、自前でカレントディレクトリを復元する必要はありません
		if (GetOpenFileName(&ofn)) {
			return std::string(fullPath) ;
		}

		return "";
	}


	//------------------------------------------------------------------------------------------------------------------------------
	std::string GetOpenSaveFilePath(const std::string& ext, const std::string& defaultName) {
		char fullPath[MAX_PATH] = { 0 };

		// --- デフォルトファイル名をバッファにコピー ---
		// もし引数があれば、それを初期値としてセットする
		if (!defaultName.empty()) {
			// バッファオーバーフローを防ぐため、安全にコピー
			strncpy_s(fullPath, defaultName.c_str(), _TRUNCATE);
		}

		OPENFILENAME ofn = {};
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = GetMainWindowHandle();

		// フィルタ設定（前回同様）
		std::string filterDesc = ext + " File (*." + ext + ")";
		std::string filterPattern = "*." + ext;
		std::vector<char> filter;
		filter.insert(filter.end(), filterDesc.begin(), filterDesc.end());
		filter.push_back('\0');
		filter.insert(filter.end(), filterPattern.begin(), filterPattern.end());
		filter.push_back('\0');
		filter.push_back('\0');

		ofn.lpstrFilter = filter.data();
		ofn.lpstrFile = fullPath;      // 初期値が入ったバッファを渡す
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrDefExt = ext.c_str();
		ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
		ofn.lpstrTitle = "Export";

		if (GetSaveFileName(&ofn)) {
			return std::string(fullPath);
		}

		return "";
	}
	//------------------------------------------------------------------------------------------------------------------------------
	static ID3D11VertexShader* default_vs = nullptr;
	static ID3D11PixelShader* default_ps = nullptr;
	static ID3D11HullShader* default_hs = nullptr;
	static ID3D11GeometryShader* default_gs = nullptr;
	static ID3D11DomainShader* default_ds = nullptr;
	static UINT32 default_sample_mask = 0;
	static ID3D11BlendState* default_blend_state = nullptr;
	static ID3D11RasterizerState* default_rasterize_state = nullptr;
	static ID3D11SamplerState* default_sampler_state = nullptr;
	static ID3D11ShaderResourceView* default_pixel_shader_resouce_view = nullptr;
	static ID3D11DepthStencilState* default_depth_stencil_state_ = nullptr;
	static UINT default_depth_stencil_ref_ = 0;

	void DirectXRenderBegin() {
		ID3D11DeviceContext* pImmediateContext = (ID3D11DeviceContext*)DxLib::GetUseDirect3D11DeviceContext();

		pImmediateContext->OMGetDepthStencilState(&default_depth_stencil_state_, &default_depth_stencil_ref_);

		// DX ライブラリのシェーダバックアップ
		pImmediateContext->VSGetShader(&default_vs, NULL, 0);
		pImmediateContext->PSGetShader(&default_ps, NULL, 0);
		pImmediateContext->HSGetShader(&default_hs, NULL, 0);
		pImmediateContext->GSGetShader(&default_gs, NULL, 0);
		pImmediateContext->DSGetShader(&default_ds, NULL, 0);

		// ブレンドステートバックアップ
		pImmediateContext->OMGetBlendState(&default_blend_state, 0, &default_sample_mask);

		// ラスタライザステートバックアップ
		pImmediateContext->RSGetState(&default_rasterize_state);

		// サンプラステートバックアップ
		pImmediateContext->PSGetSamplers(0, 1, &default_sampler_state);

		// ピクセルシェーダリソースビュー
		pImmediateContext->PSGetShaderResources(0, 1, &default_pixel_shader_resouce_view);
	}

	//------------------------------------------------------------------------------------------------------------------------------
	void DirectXRenderEnd() {
		ID3D11DeviceContext* pImmediateContext = (ID3D11DeviceContext*)DxLib::GetUseDirect3D11DeviceContext();

		pImmediateContext->OMSetDepthStencilState(default_depth_stencil_state_, default_depth_stencil_ref_);

		// DX ライブラリへシェーダバックアップを復帰
		pImmediateContext->VSSetShader(default_vs, NULL, 0);
		pImmediateContext->PSSetShader(default_ps, NULL, 0);
		pImmediateContext->HSSetShader(default_hs, NULL, 0);
		pImmediateContext->GSSetShader(default_gs, NULL, 0);
		pImmediateContext->DSSetShader(default_ds, NULL, 0);

		pImmediateContext->OMSetBlendState(default_blend_state, 0, default_sample_mask);
		pImmediateContext->RSSetState(default_rasterize_state);
		pImmediateContext->PSSetSamplers(0, 1, &default_sampler_state);
		pImmediateContext->PSSetShaderResources(0, 1, &default_pixel_shader_resouce_view);

		DxLib::RefreshDxLibDirect3DSetting();
	}

	//------------------------------------------------------------------------------------------------------------------------------
	void DirectXInitialize() {

		HRESULT hr = E_FAIL;
		ID3D11Device* pd3dDevice = (ID3D11Device*)DxLib::GetUseDirect3D11Device();
		ID3D11DeviceContext* pImmediateContext = (ID3D11DeviceContext*)DxLib::GetUseDirect3D11DeviceContext();

		//-------------------------------------------------------------------------------------------------
		// 深度ステンシルステート作成 ( ステンシル設定なし )
		CreateDepthStencil(eDepthStenclil::DEPTH_W_ON_T_ON, D3D11_DEPTH_WRITE_MASK_ALL, TRUE);
		CreateDepthStencil(eDepthStenclil::DEPTH_W_ON_T_OFF, D3D11_DEPTH_WRITE_MASK_ALL, FALSE);
		CreateDepthStencil(eDepthStenclil::DEPTH_W_OFF_T_ON, D3D11_DEPTH_WRITE_MASK_ZERO, TRUE);
		CreateDepthStencil(eDepthStenclil::DEPTH_W_OFF_T_OFF, D3D11_DEPTH_WRITE_MASK_ZERO, FALSE);

		//-------------------------------------------------------------------------------------------------
		// ブレンドステート作成 
		CreateBlendState(eBlendState::NORMAL);
		CreateBlendState(eBlendState::ALPHA);
		CreateBlendState(eBlendState::ADD);
		CreateBlendState(eBlendState::SUB);
		CreateBlendState(eBlendState::MUL);

		//-------------------------------------------------------------------------------------------------
		// サンプラテート作成 
		CreateSamplerState(eSamplerState::ANISOTROPIC);
		CreateSamplerState(eSamplerState::BILINEAR);
		CreateSamplerState(eSamplerState::NEAREST);

		//-------------------------------------------------------------------------------------------------
		// ラスタライザステート作成
		CreateRasterizerState(eRasterizerState::CULL_NONE);
		CreateRasterizerState(eRasterizerState::CULL_FRONT);
		CreateRasterizerState(eRasterizerState::CULL_BACK);
		CreateRasterizerState(eRasterizerState::WIREFRAME);
	}

	//------------------------------------------------------------------------------------------------------------------------------
	void DirectXRelease() {

		for (int i = 0; i < static_cast<int>(eDepthStenclil::MAX); ++i) {
			g_state_depth_stencil[i].Reset();
		}
		for (int i = 0; i < static_cast<int>(eBlendState::MAX); ++i) {
			g_blend_states[i].Reset();
		}
		for (int i = 0; i < static_cast<int>(eSamplerState::MAX); ++i) {
			g_sampler_states[i].Reset();
		}
		for (int i = 0; i < static_cast<int>(eRasterizerState::MAX); ++i) {
			g_rasterizer_states[i].Reset();
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------
	void ImGuiInitialize(ImGuiColorEditFlags flags) {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= flags;

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		ImGui_ImplWin32_Init(DxLib::GetMainWindowHandle());
		ImGui_ImplDX11_Init((ID3D11Device*)DxLib::GetUseDirect3D11Device(), (ID3D11DeviceContext*)DxLib::GetUseDirect3D11DeviceContext());
	}

	//------------------------------------------------------------------------------------------------------------------------------
	void ImGuiRelease() {
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	//------------------------------------------------------------------------------------------------------------------------------
	void ImGuiNewFrame() {
		DxLib::RenderVertex();
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	//------------------------------------------------------------------------------------------------------------------------------
	void ImGuiRender() {
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		// Update and Render additional Platform Windows
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------
	void ImGuiSetStyleThemeMocha() {

		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* colors = style.Colors;

		// Catppuccin Mocha Palette
		// --------------------------------------------------------
		const ImVec4 base = ImVec4(0.117f, 0.117f, 0.172f, 1.0f); // #1e1e2e
		const ImVec4 mantle = ImVec4(0.109f, 0.109f, 0.156f, 1.0f); // #181825
		const ImVec4 surface0 = ImVec4(0.200f, 0.207f, 0.286f, 1.0f); // #313244
		const ImVec4 surface1 = ImVec4(0.247f, 0.254f, 0.337f, 1.0f); // #3f4056
		const ImVec4 surface2 = ImVec4(0.290f, 0.301f, 0.388f, 1.0f); // #4a4d63
		const ImVec4 overlay0 = ImVec4(0.396f, 0.403f, 0.486f, 1.0f); // #65677c
		const ImVec4 overlay2 = ImVec4(0.576f, 0.584f, 0.654f, 1.0f); // #9399b2
		const ImVec4 text = ImVec4(0.803f, 0.815f, 0.878f, 1.0f); // #cdd6f4
		const ImVec4 subtext0 = ImVec4(0.639f, 0.658f, 0.764f, 1.0f); // #a3a8c3
		const ImVec4 mauve = ImVec4(0.796f, 0.698f, 0.972f, 1.0f); // #cba6f7
		const ImVec4 peach = ImVec4(0.980f, 0.709f, 0.572f, 1.0f); // #fab387
		const ImVec4 yellow = ImVec4(0.980f, 0.913f, 0.596f, 1.0f); // #f9e2af
		const ImVec4 green = ImVec4(0.650f, 0.890f, 0.631f, 1.0f); // #a6e3a1
		const ImVec4 teal = ImVec4(0.580f, 0.886f, 0.819f, 1.0f); // #94e2d5
		const ImVec4 sapphire = ImVec4(0.458f, 0.784f, 0.878f, 1.0f); // #74c7ec
		const ImVec4 blue = ImVec4(0.533f, 0.698f, 0.976f, 1.0f); // #89b4fa
		const ImVec4 lavender = ImVec4(0.709f, 0.764f, 0.980f, 1.0f); // #b4befe

		// Main window and backgrounds
		colors[ImGuiCol_WindowBg] = base;
		colors[ImGuiCol_ChildBg] = base;
		colors[ImGuiCol_PopupBg] = surface0;
		colors[ImGuiCol_Border] = surface1;
		colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		colors[ImGuiCol_FrameBg] = surface0;
		colors[ImGuiCol_FrameBgHovered] = surface1;
		colors[ImGuiCol_FrameBgActive] = surface2;
		colors[ImGuiCol_TitleBg] = mantle;
		colors[ImGuiCol_TitleBgActive] = surface0;
		colors[ImGuiCol_TitleBgCollapsed] = mantle;
		colors[ImGuiCol_MenuBarBg] = mantle;
		colors[ImGuiCol_ScrollbarBg] = surface0;
		colors[ImGuiCol_ScrollbarGrab] = surface2;
		colors[ImGuiCol_ScrollbarGrabHovered] = overlay0;
		colors[ImGuiCol_ScrollbarGrabActive] = overlay2;
		colors[ImGuiCol_CheckMark] = green;
		colors[ImGuiCol_SliderGrab] = sapphire;
		colors[ImGuiCol_SliderGrabActive] = blue;
		colors[ImGuiCol_Button] = surface0;
		colors[ImGuiCol_ButtonHovered] = surface1;
		colors[ImGuiCol_ButtonActive] = surface2;
		colors[ImGuiCol_Header] = surface0;
		colors[ImGuiCol_HeaderHovered] = surface1;
		colors[ImGuiCol_HeaderActive] = surface2;
		colors[ImGuiCol_Separator] = surface1;
		colors[ImGuiCol_SeparatorHovered] = mauve;
		colors[ImGuiCol_SeparatorActive] = mauve;
		colors[ImGuiCol_ResizeGrip] = surface2;
		colors[ImGuiCol_ResizeGripHovered] = mauve;
		colors[ImGuiCol_ResizeGripActive] = mauve;
		colors[ImGuiCol_Tab] = surface0;
		colors[ImGuiCol_TabHovered] = surface2;
		colors[ImGuiCol_TabActive] = surface1;
		colors[ImGuiCol_TabUnfocused] = surface0;
		colors[ImGuiCol_TabUnfocusedActive] = surface1;
		colors[ImGuiCol_DockingPreview] = sapphire;
		colors[ImGuiCol_DockingEmptyBg] = base;
		colors[ImGuiCol_PlotLines] = blue;
		colors[ImGuiCol_PlotLinesHovered] = peach;
		colors[ImGuiCol_PlotHistogram] = teal;
		colors[ImGuiCol_PlotHistogramHovered] = green;
		colors[ImGuiCol_TableHeaderBg] = surface0;
		colors[ImGuiCol_TableBorderStrong] = surface1;
		colors[ImGuiCol_TableBorderLight] = surface0;
		colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.06f);
		colors[ImGuiCol_TextSelectedBg] = surface2;
		colors[ImGuiCol_DragDropTarget] = yellow;
		colors[ImGuiCol_NavHighlight] = lavender;
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.35f);
		colors[ImGuiCol_Text] = text;
		colors[ImGuiCol_TextDisabled] = subtext0;

		// Rounded corners
		style.WindowRounding = 6.0f;
		style.ChildRounding = 6.0f;
		style.FrameRounding = 4.0f;
		style.PopupRounding = 4.0f;
		style.ScrollbarRounding = 9.0f;
		style.GrabRounding = 4.0f;
		style.TabRounding = 4.0f;

		// Padding and spacing
		style.WindowPadding = ImVec2(8.0f, 8.0f);
		style.FramePadding = ImVec2(5.0f, 3.0f);
		style.ItemSpacing = ImVec2(8.0f, 4.0f);
		style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
		style.IndentSpacing = 21.0f;
		style.ScrollbarSize = 14.0f;
		style.GrabMinSize = 10.0f;

		// Borders
		style.WindowBorderSize = 1.0f;
		style.ChildBorderSize = 1.0f;
		style.PopupBorderSize = 1.0f;
		style.FrameBorderSize = 0.0f;
		style.TabBorderSize = 0.0f;

	}

	//------------------------------------------------------------------------------------------------------------------------------
	void ImGuiSetStyleThemeLaivy() {
		auto& style{ ImGui::GetStyle() };
		// Borders
		style.WindowBorderSize = 3.0f;

		// Rounding
		style.FrameRounding = 3.0f;
		style.PopupRounding = 3.0f;
		style.ScrollbarRounding = 3.0f;
		style.GrabRounding = 3.0f;

		// Docking
		style.DockingSeparatorSize = 3.0f;

		{
			constexpr auto ToRGBA = [](uint32_t argb) constexpr
				{
					ImVec4 color{};
					color.x = ((argb >> 16) & 0xFF) / 255.0f;
					color.y = ((argb >> 8) & 0xFF) / 255.0f;
					color.z = (argb & 0xFF) / 255.0f;
					color.w = ((argb >> 24) & 0xFF) / 255.0f;
					return color;
				};

			constexpr auto Lerp = [](const ImVec4& a, const ImVec4& b, float t) constexpr
				{
					return ImVec4{
						std::lerp(a.x, b.y, t),
						std::lerp(a.y, b.y, t),
						std::lerp(a.z, b.z, t),
						std::lerp(a.w, b.w, t)
					};
				};

			auto colors{ style.Colors };
			colors[ImGuiCol_Text] = ToRGBA(0xFFABB2BF);
			colors[ImGuiCol_TextDisabled] = ToRGBA(0xFF565656);
			colors[ImGuiCol_WindowBg] = ToRGBA(0xFF282C34);
			colors[ImGuiCol_ChildBg] = ToRGBA(0xFF21252B);
			colors[ImGuiCol_PopupBg] = ToRGBA(0xFF2E323A);
			colors[ImGuiCol_Border] = ToRGBA(0xFF2E323A);
			colors[ImGuiCol_BorderShadow] = ToRGBA(0x00000000);
			colors[ImGuiCol_FrameBg] = colors[ImGuiCol_ChildBg];
			colors[ImGuiCol_FrameBgHovered] = ToRGBA(0xFF484C52);
			colors[ImGuiCol_FrameBgActive] = ToRGBA(0xFF54575D);
			colors[ImGuiCol_TitleBg] = colors[ImGuiCol_WindowBg];
			colors[ImGuiCol_TitleBgActive] = colors[ImGuiCol_FrameBgActive];
			colors[ImGuiCol_TitleBgCollapsed] = ToRGBA(0x8221252B);
			colors[ImGuiCol_MenuBarBg] = colors[ImGuiCol_ChildBg];
			colors[ImGuiCol_ScrollbarBg] = colors[ImGuiCol_PopupBg];
			colors[ImGuiCol_ScrollbarGrab] = ToRGBA(0xFF3E4249);
			colors[ImGuiCol_ScrollbarGrabHovered] = ToRGBA(0xFF484C52);
			colors[ImGuiCol_ScrollbarGrabActive] = ToRGBA(0xFF54575D);
			colors[ImGuiCol_CheckMark] = colors[ImGuiCol_Text];
			colors[ImGuiCol_SliderGrab] = ToRGBA(0xFF353941);
			colors[ImGuiCol_SliderGrabActive] = ToRGBA(0xFF7A7A7A);
			colors[ImGuiCol_Button] = colors[ImGuiCol_SliderGrab];
			colors[ImGuiCol_ButtonHovered] = colors[ImGuiCol_FrameBgActive];
			colors[ImGuiCol_ButtonActive] = colors[ImGuiCol_ScrollbarGrabActive];
			colors[ImGuiCol_Header] = colors[ImGuiCol_ChildBg];
			colors[ImGuiCol_HeaderHovered] = ToRGBA(0xFF353941);
			colors[ImGuiCol_HeaderActive] = colors[ImGuiCol_FrameBgActive];
			colors[ImGuiCol_Separator] = colors[ImGuiCol_FrameBgActive];
			colors[ImGuiCol_SeparatorHovered] = ToRGBA(0xFF3E4452);
			colors[ImGuiCol_SeparatorActive] = colors[ImGuiCol_SeparatorHovered];
			colors[ImGuiCol_ResizeGrip] = colors[ImGuiCol_Separator];
			colors[ImGuiCol_ResizeGripHovered] = colors[ImGuiCol_SeparatorHovered];
			colors[ImGuiCol_ResizeGripActive] = colors[ImGuiCol_SeparatorActive];
			colors[ImGuiCol_InputTextCursor] = ToRGBA(0xFF528BFF);
			colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
			colors[ImGuiCol_Tab] = colors[ImGuiCol_FrameBgActive];
			colors[ImGuiCol_TabSelected] = colors[ImGuiCol_HeaderHovered];
			colors[ImGuiCol_TabSelectedOverline] = colors[ImGuiCol_HeaderActive];
			colors[ImGuiCol_TabDimmed] = Lerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
			colors[ImGuiCol_TabDimmedSelected] = Lerp(colors[ImGuiCol_TabSelected], colors[ImGuiCol_TitleBg], 0.40f);
			colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4{ 0.50f, 0.50f, 0.50f, 0.00f };
			colors[ImGuiCol_DockingPreview] = colors[ImGuiCol_ChildBg];
			colors[ImGuiCol_DockingEmptyBg] = colors[ImGuiCol_WindowBg];
			colors[ImGuiCol_PlotLines] = ImVec4{ 0.61f, 0.61f, 0.61f, 1.00f };
			colors[ImGuiCol_PlotLinesHovered] = ImVec4{ 1.00f, 0.43f, 0.35f, 1.00f };
			colors[ImGuiCol_PlotHistogram] = ImVec4{ 0.90f, 0.70f, 0.00f, 1.00f };
			colors[ImGuiCol_PlotHistogramHovered] = ImVec4{ 1.00f, 0.60f, 0.00f, 1.00f };
			colors[ImGuiCol_TableHeaderBg] = colors[ImGuiCol_ChildBg];
			colors[ImGuiCol_TableBorderStrong] = colors[ImGuiCol_SliderGrab];
			colors[ImGuiCol_TableBorderLight] = colors[ImGuiCol_FrameBgActive];
			colors[ImGuiCol_TableRowBg] = ImVec4{ 0.00f, 0.00f, 0.00f, 0.00f };
			colors[ImGuiCol_TableRowBgAlt] = ImVec4{ 1.00f, 1.00f, 1.00f, 0.06f };
			colors[ImGuiCol_TextLink] = ToRGBA(0xFF3F94CE);
			colors[ImGuiCol_TextSelectedBg] = ToRGBA(0xFF243140);
			colors[ImGuiCol_TreeLines] = colors[ImGuiCol_Text];
			colors[ImGuiCol_DragDropTarget] = colors[ImGuiCol_Text];
			colors[ImGuiCol_NavCursor] = colors[ImGuiCol_TextLink];
			colors[ImGuiCol_NavWindowingHighlight] = colors[ImGuiCol_Text];
			colors[ImGuiCol_NavWindowingDimBg] = ImVec4{ 0.80f, 0.80f, 0.80f, 0.20f };
			colors[ImGuiCol_ModalWindowDimBg] = ToRGBA(0xC821252B);
		};

	}

	//------------------------------------------------------------------------------------------------------------------------------
	void ImGuiSetStyleThemeV3() {

		int hue07 = 0;
		int alt07 = 0;
		int nav07 = 0;
		int lit01 = 0;
		int compact01 = 0;
		int border01 = 1;
		int shape0123 = 1;

		bool rounded = shape0123 == 2;

		// V3 style from ImThemes
		ImGuiStyle& style = ImGui::GetStyle();

		const float _8 = compact01 ? 4.0f : 8.0f;
		const float _4 = compact01 ? 2.0f : 4.0f;
		const float _2 = compact01 ? 0.5f : 1.0f;

		style.Alpha = 1.0f;
		style.DisabledAlpha = 0.3f;

		style.WindowPadding = ImVec2(4, _8);
		style.FramePadding = ImVec2(4, _4);
		style.ItemSpacing = ImVec2(_8, _2 + _2);
		style.ItemInnerSpacing = ImVec2(4, 4);
		style.IndentSpacing = 16;
		style.ScrollbarSize = compact01 ? 12.0f : 18.0f;
		style.GrabMinSize = compact01 ? 16.0f : 20.0f;

		style.WindowBorderSize = (float)border01;
		style.ChildBorderSize = (float)border01;
		style.PopupBorderSize = (float)border01;
		style.FrameBorderSize = 0;

		style.WindowRounding = 4;
		style.ChildRounding = 6;
		style.FrameRounding = shape0123 == 0 ? 0.0f : shape0123 == 1 ? 4.0f : 12.0f;
		style.PopupRounding = 4;
		style.ScrollbarRounding = (float)( (int)rounded * 8 + 4 ) ;
		style.GrabRounding = style.FrameRounding;

		style.TabBorderSize = 0;
		style.TabBarBorderSize = 2;
		style.TabBarOverlineSize = 2;
		style.TabCloseButtonMinWidthSelected = -1; // -1:always visible, 0:visible when hovered, >0:visible when hovered if minimum width
		style.TabCloseButtonMinWidthUnselected = -1;
		style.TabRounding = rounded;

		style.CellPadding = ImVec2(8.0f, 4.0f);

		style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
		style.WindowMenuButtonPosition = ImGuiDir_Right;

		style.ColorButtonPosition = ImGuiDir_Right;
		style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
		style.SelectableTextAlign = ImVec2(0.5f, 0.5f);
		style.SeparatorTextAlign.x = 1.00f;
		style.SeparatorTextBorderSize = 1;
		style.SeparatorTextPadding = ImVec2(0, 0);

		style.WindowMinSize = ImVec2(32.0f, 16.0f);
		style.ColumnsMinSpacing = 6.0f;

		// diamond sliders
		style.CircleTessellationMaxError = shape0123 == 3 ? 4.00f : 0.30f;

		auto lit = [&](ImVec4 hi) {
			float h, s, v; ImGui::ColorConvertRGBtoHSV(hi.x, hi.y, hi.z, h, s, v);
			ImVec4 lit = ImColor::HSV(h, s * 0.80f, v * 1.00f, hi.w).Value;
			return lit;
			};
		auto dim = [&](ImVec4 hi) {
			float h, s, v; ImGui::ColorConvertRGBtoHSV(hi.x, hi.y, hi.z, h, s, v);
			ImVec4 dim = ImColor::HSV(h, s, lit01 ? v * 0.65f : v * 0.65f, hi.w).Value;
			if (hi.z > hi.x && hi.z > hi.y) return ImVec4(dim.x, dim.y, hi.z, dim.w);
			return dim;
			};

		const ImVec4 cyan = ImVec4(000 / 255.f, 192 / 255.f, 255 / 255.f, 1.00f);
		const ImVec4 red = ImVec4(230 / 255.f, 000 / 255.f, 000 / 255.f, 1.00f);
		const ImVec4 yellow = ImVec4(240 / 255.f, 210 / 255.f, 000 / 255.f, 1.00f);
		const ImVec4 orange = ImVec4(255 / 255.f, 144 / 255.f, 000 / 255.f, 1.00f);
		const ImVec4 lime = ImVec4(192 / 255.f, 255 / 255.f, 000 / 255.f, 1.00f);
		const ImVec4 aqua = ImVec4(000 / 255.f, 255 / 255.f, 192 / 255.f, 1.00f);
		const ImVec4 magenta = ImVec4(255 / 255.f, 000 / 255.f, 88 / 255.f, 1.00f);
		const ImVec4 purple = ImVec4(192 / 255.f, 000 / 255.f, 255 / 255.f, 1.00f);

		ImVec4 alt = cyan;
		/**/ if (alt07 == 0 || alt07 == 'C') alt = cyan;
		else if (alt07 == 1 || alt07 == 'R') alt = red;
		else if (alt07 == 2 || alt07 == 'Y') alt = yellow;
		else if (alt07 == 3 || alt07 == 'O') alt = orange;
		else if (alt07 == 4 || alt07 == 'L') alt = lime;
		else if (alt07 == 5 || alt07 == 'A') alt = aqua;
		else if (alt07 == 6 || alt07 == 'M') alt = magenta;
		else if (alt07 == 7 || alt07 == 'P') alt = purple;
		if (lit01) alt = dim(alt);

		ImVec4 hi = cyan, lo = dim(cyan);
		/**/ if (hue07 == 0 || hue07 == 'C') lo = dim(hi = cyan);
		else if (hue07 == 1 || hue07 == 'R') lo = dim(hi = red);
		else if (hue07 == 2 || hue07 == 'Y') lo = dim(hi = yellow);
		else if (hue07 == 3 || hue07 == 'O') lo = dim(hi = orange);
		else if (hue07 == 4 || hue07 == 'L') lo = dim(hi = lime);
		else if (hue07 == 5 || hue07 == 'A') lo = dim(hi = aqua);
		else if (hue07 == 6 || hue07 == 'M') lo = dim(hi = magenta);
		else if (hue07 == 7 || hue07 == 'P') lo = dim(hi = purple);
		//    if( lit01 ) { ImVec4 tmp = hi; hi = lo; lo = lit(tmp); }

		ImVec4 nav = orange;
		/**/ if (nav07 == 0 || nav07 == 'C') nav = cyan;
		else if (nav07 == 1 || nav07 == 'R') nav = red;
		else if (nav07 == 2 || nav07 == 'Y') nav = yellow;
		else if (nav07 == 3 || nav07 == 'O') nav = orange;
		else if (nav07 == 4 || nav07 == 'L') nav = lime;
		else if (nav07 == 5 || nav07 == 'A') nav = aqua;
		else if (nav07 == 6 || nav07 == 'M') nav = magenta;
		else if (nav07 == 7 || nav07 == 'P') nav = purple;
		if (lit01) nav = dim(nav);

		const ImVec4
			link = ImVec4(0.26f, 0.59f, 0.98f, 1.00f),
			grey0 = ImVec4(0.04f, 0.05f, 0.07f, 1.00f),
			grey1 = ImVec4(0.08f, 0.09f, 0.11f, 1.00f),
			grey2 = ImVec4(0.10f, 0.11f, 0.13f, 1.00f),
			grey3 = ImVec4(0.12f, 0.13f, 0.15f, 1.00f),
			grey4 = ImVec4(0.16f, 0.17f, 0.19f, 1.00f),
			grey5 = ImVec4(0.18f, 0.19f, 0.21f, 1.00f);

#define Luma(v,a) ImVec4((v)/100.f,(v)/100.f,(v)/100.f,(a)/100.f)

		style.Colors[ImGuiCol_Text] = Luma(100, 100);
		style.Colors[ImGuiCol_TextDisabled] = Luma(39, 100);
		style.Colors[ImGuiCol_WindowBg] = grey1;
		style.Colors[ImGuiCol_ChildBg] = ImVec4(0.09f, 0.10f, 0.12f, 1.00f);
		style.Colors[ImGuiCol_PopupBg] = grey1;
		style.Colors[ImGuiCol_Border] = grey4;
		style.Colors[ImGuiCol_BorderShadow] = grey1;
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.11f, 0.13f, 0.15f, 1.00f);
		style.Colors[ImGuiCol_FrameBgHovered] = grey4;
		style.Colors[ImGuiCol_FrameBgActive] = grey4;
		style.Colors[ImGuiCol_TitleBg] = grey0;
		style.Colors[ImGuiCol_TitleBgActive] = grey0;
		style.Colors[ImGuiCol_TitleBgCollapsed] = grey1;
		style.Colors[ImGuiCol_MenuBarBg] = grey2;
		style.Colors[ImGuiCol_ScrollbarBg] = grey0;
		style.Colors[ImGuiCol_ScrollbarGrab] = grey3;
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = lo;
		style.Colors[ImGuiCol_ScrollbarGrabActive] = hi;
		style.Colors[ImGuiCol_CheckMark] = alt;
		style.Colors[ImGuiCol_SliderGrab] = lo;
		style.Colors[ImGuiCol_SliderGrabActive] = hi;
		style.Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.11f, 0.14f, 1.00f);
		style.Colors[ImGuiCol_ButtonHovered] = lo;
		style.Colors[ImGuiCol_ButtonActive] = grey5;
		style.Colors[ImGuiCol_Header] = grey3;
		style.Colors[ImGuiCol_HeaderHovered] = lo;
		style.Colors[ImGuiCol_HeaderActive] = hi;
		style.Colors[ImGuiCol_Separator] = ImVec4(0.13f, 0.15f, 0.19f, 1.00f);
		style.Colors[ImGuiCol_SeparatorHovered] = lo;
		style.Colors[ImGuiCol_SeparatorActive] = hi;
		style.Colors[ImGuiCol_ResizeGrip] = Luma(15, 100);
		style.Colors[ImGuiCol_ResizeGripHovered] = lo;
		style.Colors[ImGuiCol_ResizeGripActive] = hi;
		style.Colors[ImGuiCol_InputTextCursor] = Luma(100, 100);
		style.Colors[ImGuiCol_TabHovered] = grey3;
		style.Colors[ImGuiCol_Tab] = grey1;
		style.Colors[ImGuiCol_TabSelected] = grey3;
		style.Colors[ImGuiCol_TabSelectedOverline] = hi;
		style.Colors[ImGuiCol_TabDimmed] = grey1;
		style.Colors[ImGuiCol_TabDimmedSelected] = grey1;
		style.Colors[ImGuiCol_TabDimmedSelectedOverline] = lo;
		style.Colors[ImGuiCol_DockingPreview] = grey1;
		style.Colors[ImGuiCol_DockingEmptyBg] = Luma(20, 100);
		style.Colors[ImGuiCol_PlotLines] = grey5;
		style.Colors[ImGuiCol_PlotLinesHovered] = lo;
		style.Colors[ImGuiCol_PlotHistogram] = grey5;
		style.Colors[ImGuiCol_PlotHistogramHovered] = lo;
		style.Colors[ImGuiCol_TableHeaderBg] = grey0;
		style.Colors[ImGuiCol_TableBorderStrong] = grey0;
		style.Colors[ImGuiCol_TableBorderLight] = grey0;
		style.Colors[ImGuiCol_TableRowBg] = grey3;
		style.Colors[ImGuiCol_TableRowBgAlt] = grey2;
		style.Colors[ImGuiCol_TextLink] = link;
		style.Colors[ImGuiCol_TextSelectedBg] = Luma(39, 100);
		style.Colors[ImGuiCol_TreeLines] = Luma(39, 100);
		style.Colors[ImGuiCol_DragDropTarget] = nav;
		style.Colors[ImGuiCol_NavCursor] = nav;
		style.Colors[ImGuiCol_NavWindowingHighlight] = lo;
		style.Colors[ImGuiCol_NavWindowingDimBg] = Luma(0, 63);
		style.Colors[ImGuiCol_ModalWindowDimBg] = Luma(0, 63);

		if (lit01) {
			for (int i = 0; i < ImGuiCol_COUNT; i++) {
				float H, S, V;
				ImVec4& col = style.Colors[i];
				ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);
				if (S < 0.5f) V = 1.0f - V, S *= 0.15f;
				ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
			}
		}
#undef Luma

	}


	//------------------------------------------------------------------------------------------------------------------------------
	void ImGuiSetStyleThemeSimv0l() {
		ImGuiStyle* style = &ImGui::GetStyle();
		ImVec4* colors = style->Colors;

		// Base colors for a pleasant and modern dark theme with dark accents
		colors[ImGuiCol_Text] = ImVec4(0.92f, 0.93f, 0.94f, 1.00f);                  // Light grey text for readability
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.52f, 0.54f, 1.00f);          // Subtle grey for disabled text
		colors[ImGuiCol_WindowBg] = ImVec4(0.14f, 0.14f, 0.16f, 1.00f);              // Dark background with a hint of blue
		colors[ImGuiCol_ChildBg] = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);               // Slightly lighter for child elements
		colors[ImGuiCol_PopupBg] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);               // Popup background
		colors[ImGuiCol_Border] = ImVec4(0.28f, 0.29f, 0.30f, 0.60f);                // Soft border color
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);          // No border shadow
		colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.22f, 0.24f, 1.00f);               // Frame background
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.24f, 0.26f, 1.00f);        // Frame hover effect
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.24f, 0.26f, 0.28f, 1.00f);         // Active frame background
		colors[ImGuiCol_TitleBg] = ImVec4(0.14f, 0.14f, 0.16f, 1.00f);               // Title background
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);         // Active title background
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.14f, 0.14f, 0.16f, 1.00f);      // Collapsed title background
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);             // Menu bar background
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);           // Scrollbar background
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.24f, 0.26f, 0.28f, 1.00f);         // Dark accent for scrollbar grab
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.28f, 0.30f, 0.32f, 1.00f);  // Scrollbar grab hover
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.32f, 0.34f, 0.36f, 1.00f);   // Scrollbar grab active
		colors[ImGuiCol_CheckMark] = ImVec4(0.46f, 0.56f, 0.66f, 1.00f);             // Dark blue checkmark
		colors[ImGuiCol_SliderGrab] = ImVec4(0.36f, 0.46f, 0.56f, 1.00f);            // Dark blue slider grab
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.40f, 0.50f, 0.60f, 1.00f);      // Active slider grab

		//colors[ImGuiCol_Button] = ImVec4(0.24f, 0.34f, 0.44f, 1.00f);                // Dark blue button
		//colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.38f, 0.48f, 1.00f);         // Button hover effect
		//colors[ImGuiCol_ButtonActive] = ImVec4(0.32f, 0.42f, 0.52f, 1.00f);          // Active button

		colors[ImGuiCol_Button] = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);                // Dark blue button
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.92f, 0.18f, 0.29f, 0.86f);         // Button hover effect
		colors[ImGuiCol_ButtonActive] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);          // Active button

		colors[ImGuiCol_Header] = ImVec4(0.24f, 0.34f, 0.44f, 1.00f);                // Header color similar to button
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.28f, 0.38f, 0.48f, 1.00f);         // Header hover effect
		colors[ImGuiCol_HeaderActive] = ImVec4(0.32f, 0.42f, 0.52f, 1.00f);          // Active header
		colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.29f, 0.30f, 1.00f);             // Separator color
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.46f, 0.56f, 0.66f, 1.00f);      // Hover effect for separator
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.46f, 0.56f, 0.66f, 1.00f);       // Active separator
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.36f, 0.46f, 0.56f, 1.00f);            // Resize grip
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.40f, 0.50f, 0.60f, 1.00f);     // Hover effect for resize grip
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.44f, 0.54f, 0.64f, 1.00f);      // Active resize grip
		colors[ImGuiCol_Tab] = ImVec4(0.20f, 0.22f, 0.24f, 1.00f);                   // Inactive tab
		colors[ImGuiCol_TabHovered] = ImVec4(0.28f, 0.38f, 0.48f, 1.00f);            // Hover effect for tab
		colors[ImGuiCol_TabActive] = ImVec4(0.24f, 0.34f, 0.44f, 1.00f);             // Active tab color
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.20f, 0.22f, 0.24f, 1.00f);          // Unfocused tab
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.24f, 0.34f, 0.44f, 1.00f);    // Active but unfocused tab
		colors[ImGuiCol_PlotLines] = ImVec4(0.46f, 0.56f, 0.66f, 1.00f);             // Plot lines
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.46f, 0.56f, 0.66f, 1.00f);      // Hover effect for plot lines
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.36f, 0.46f, 0.56f, 1.00f);         // Histogram color
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.40f, 0.50f, 0.60f, 1.00f);  // Hover effect for histogram
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.20f, 0.22f, 0.24f, 1.00f);         // Table header background
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.28f, 0.29f, 0.30f, 1.00f);     // Strong border for tables
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.24f, 0.25f, 0.26f, 1.00f);      // Light border for tables
		colors[ImGuiCol_TableRowBg] = ImVec4(0.20f, 0.22f, 0.24f, 1.00f);            // Table row background
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.22f, 0.24f, 0.26f, 1.00f);         // Alternate row background
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.24f, 0.34f, 0.44f, 0.35f);        // Selected text background
		colors[ImGuiCol_DragDropTarget] = ImVec4(0.46f, 0.56f, 0.66f, 0.90f);        // Drag and drop target
		colors[ImGuiCol_NavHighlight] = ImVec4(0.46f, 0.56f, 0.66f, 1.00f);          // Navigation highlight
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f); // Windowing highlight
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);     // Dim background for windowing
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);      // Dim background for modal windows

		// Style adjustments
		style->WindowPadding = ImVec2(8.00f, 8.00f);
		style->FramePadding = ImVec2(5.00f, 2.00f);
		style->CellPadding = ImVec2(6.00f, 6.00f);
		//style->ItemSpacing = ImVec2(6.00f, 6.00f);
		style->ItemInnerSpacing = ImVec2(6.00f, 6.00f);
		style->TouchExtraPadding = ImVec2(0.00f, 0.00f);
		style->IndentSpacing = 25;
		style->ScrollbarSize = 11;
		style->GrabMinSize = 10;
		style->WindowBorderSize = 1;
		style->ChildBorderSize = 1;
		style->PopupBorderSize = 1;
		style->FrameBorderSize = 1;
		style->TabBorderSize = 1;
		style->WindowRounding = 7;
		style->ChildRounding = 4;
		style->FrameRounding = 3;
		style->PopupRounding = 4;
		style->ScrollbarRounding = 9;
		style->GrabRounding = 3;
		style->LogSliderDeadzone = 4;
		style->TabRounding = 4;
	}


#endif
}
