#pragma once

#include <dxe.h>
#include "gmVirtualWave.h"

namespace gm {

	class gmWaterPlane {
	public:
		gmWaterPlane(const std::string& path);
		
		// 更新（カメラ位置に応じてタイル展開中心を更新）
		void update(const Shared<dxe::Camera>& camera);

		// dxe::WaterPlaneの描画
		void render(const Shared<dxe::Camera>& camera);
	
		// gmVirtualWaveに渡すパラメータの同期
		void syncParams();

		// 波の高さを返す
		float sampleHeight(const tnl::Vector3& pos, float time) const;
		float getTimeScale() const;

		const Shared<dxe::WaterPlane>& getWaterMesh() { return water_; }
		
		void addLandingMesh(const Shared<dxe::Mesh>& mesh, dxe::WaterPlane::fLandingMeshUse f_use, const float projection_volume = 5.0f) const;

	private:
		Shared<dxe::WaterPlane> water_;
		gmVirtualWave virtualWave_;
	};




}