#include "gmWaterPlane.h"

namespace gm {

	gmWaterPlane::gmWaterPlane(const std::string& path) {
		water_ = std::make_shared<dxe::WaterPlane>(path.c_str());

		// 描画範囲に合わせてサイズを設定
		water_->setSizeWidth(dxe::WaterPlane::eSize::S8192);
		water_->setSizeDepth(dxe::WaterPlane::eSize::S8192);

		syncParams();
	}

	void gmWaterPlane::update(const Shared<dxe::Camera>& cam)
	{
		tnl::Vector3 camPos = cam->getPosition();

		water_->setCubeMapCameraPosition(camPos);

		tnl::Vector3 waterPos(camPos.x, 0.0f, camPos.z);

		water_->setPosition(waterPos);
	}

	void gmWaterPlane::render(const Shared<dxe::Camera>& camera) {
		water_->render(camera);
	}

	// gmVirtualWaveに渡すパラメータの同期
	void gmWaterPlane::syncParams() {
		gmVirtualWave::WaveParams p;
		p.amplitude = water_->getHeightMax() * water_->getBaseAmpFactor();
		p.frequency = water_->getFrequencyFactor() * 0.1f;
		p.timeScale = water_->getTimeScale();
		virtualWave_.setParams(p);
	}

	float gmWaterPlane::sampleHeight(const tnl::Vector3& pos, float time) const {
		return virtualWave_.sampleHeight(pos, time);
	}

	float gmWaterPlane::getTimeScale() const {
		return water_->getTimeScale();
	}
}




