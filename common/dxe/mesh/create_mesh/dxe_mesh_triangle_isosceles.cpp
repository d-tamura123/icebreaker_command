#pragma warning(disable:4819)
#include "../dxe_mesh.h"
#include "../../../tnl/tnl_math.h"

namespace dxe {

	//----------------------------------------------------------------------------------------
	Shared<Mesh> Mesh::CreateTriangleIsosceles(const tnl::Vector3& size_wh, const int div_w, const int div_h, const bool is_left_cycle) noexcept
	{
		Shared<Mesh> mesh = Shared<Mesh>(new Mesh());
		mesh->desc_ = std::make_shared<MeshDescTriangleIsosceles>(size_wh, div_w, div_h, is_left_cycle);
		mesh->shape_type_ = eShapeType::TRIANGLE_ISOSCELES;

		mesh->bd_box_size_ = { size_wh.x, size_wh.y * 2, 1.0f };
		mesh->bd_sphere_radius_ = mesh->bd_box_size_.length() * 0.5f;

		// 横並びの頂点数 = ( 横分割数 * 2 ) - ( 横分割数 - 1 )
		// 縦並びの頂点数 = ( 縦分割数 * 2 ) - ( 縦分割数 - 1 )
		// 総頂点数 = 横並びの頂点数 * 縦並びの頂点数
		int vtx_num = ((div_w * 2) - (div_w - 1)) * ((div_h * 2) - (div_h - 1));
		mesh->vtxs_.resize(vtx_num);

		float width = size_wh.x;
		float height = size_wh.y;

		int stack = div_h;
		int srice = div_w;
		//	tnl::Vector3 c = (tnl::Vector3(0, hh, 0) + tnl::Vector3(-ww, -hh, 0) + tnl::Vector3(ww, -hh, 0)) / 3.0f;
		tnl::Vector3 c;

		// 頂点座標・UV・法線の計算
		tnl::Vector3 vv;
		for (int i = 0; i < (stack + 1); ++i) {
			for (int k = 0; k < (srice + 1); ++k) {
				vv.x = (-(width * 0.5f) + (width / srice * k)) * (1.0f / stack * i);
				vv.y = height - (height / stack * i);
				vv.z = 0;

				mesh->vtxs_[(i * (srice + 1)) + k].pos = { vv.x, vv.y, vv.z };

				mesh->vtxs_[(i * (srice + 1)) + k].u = 0.5f + (mesh->vtxs_[(i * (srice + 1)) + k].pos.x / width);
				mesh->vtxs_[(i * (srice + 1)) + k].v = 1.0f - (1.0f / (float)stack * i);

				mesh->vtxs_[(i * (srice + 1)) + k].norm.x = 0;
				mesh->vtxs_[(i * (srice + 1)) + k].norm.y = 0;
				mesh->vtxs_[(i * (srice + 1)) + k].norm.z = (is_left_cycle) ? 1.0f : -1.0f;

				mesh->vtxs_[(i * (div_w + 1)) + k].dif = GetColorU8(255, 255, 255, 255);
			}
		}

		mesh->createPlaneIndex(div_w, div_h, !is_left_cycle);
		mesh->createVBO();
		return mesh;
	}

	Shared<Mesh> Mesh::CreateTriangleIsoscelesMV(const tnl::Vector3& size_wh, const int div_w, const int div_h, const bool is_left_cycle) noexcept {
		Shared<Mesh> mesh = CreateTriangleIsosceles(size_wh, div_w, div_h, is_left_cycle);
		mesh = CreateConvertMV(mesh);
		return mesh;
	}


}

