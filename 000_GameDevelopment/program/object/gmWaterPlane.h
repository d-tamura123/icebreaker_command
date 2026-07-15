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

	private:
		Shared<dxe::WaterPlane> water_;
		gmVirtualWave virtualWave_;
	};




}