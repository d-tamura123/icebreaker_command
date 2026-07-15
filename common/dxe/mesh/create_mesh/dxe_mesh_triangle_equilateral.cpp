#pragma warning(disable:4819)
#include "../dxe_mesh.h"
#include "../../../tnl/tnl_math.h"

namespace dxe {

	//----------------------------------------------------------------------------------------
	Shared<Mesh> Mesh::CreateTriangleEquilateral(const float edge_length, const int div_w, const int div_h, const bool is_left_cycle) noexcept
	{		
		float width = edge_length;
		float height = edge_length * sqrtf(3.0f) * 0.5f;
		Shared<Mesh> mesh = CreateTriangleIsosceles({ width, height, 0 }, div_w, div_h, is_left_cycle);
		mesh->desc_ = std::make_shared<MeshDescTriangleEquilateral>(edge_length, div_w, div_h, is_left_cycle);
		mesh->shape_type_ = eShapeType::TRIANGLE_EQUILATERAL;

		float r = tnl::RightTriangleHeightByBase(tnl::ToRadian(60), edge_length / 2.0f);
		mesh->bd_box_size_ = { edge_length, r*2, 1.0f };
		mesh->bd_sphere_radius_ = mesh->bd_box_size_.length() * 0.5f;

		return mesh;
	}

	Shared<Mesh> Mesh::CreateTriangleEquilateralMV(const float edge_lenght, const int div_w, const int div_h, const bool is_left_cycle) noexcept {
		Shared<Mesh> mesh = CreateTriangleEquilateral(edge_lenght, div_w, div_h, is_left_cycle);
		mesh = CreateConvertMV(mesh);
		return mesh;
	}


}

