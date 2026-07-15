#include "gmMeshBase.h"

namespace gm {

	void gmMeshBase::create(const std::string filePath, float scale) {
		mesh_ = dxe::Mesh::CreateFromFileMV(filePath.c_str(), scale);
	}

	/*virtual*/ void gmMeshBase::render(const Shared<dxe::Camera>& camera) {
		if (!mesh_) return;

		mesh_->setPosition(position_);
		mesh_->setScaling(scale_);
		
		// Euler -> Quaternion •ÏŠ·
		tnl::Quaternion quaternionMesh =
			tnl::Quaternion::RotationAxis({ 1, 0, 0 }, rotation_.x) *
			tnl::Quaternion::RotationAxis({ 0, 1, 0 }, rotation_.y) *
			tnl::Quaternion::RotationAxis({ 0, 0, 1 }, rotation_.z);

		mesh_->setRotation(quaternionMesh);
		
		mesh_->render(camera);
	}

}