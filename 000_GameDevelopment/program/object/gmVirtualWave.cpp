#include "gmVirtualWave.h"
namespace gm {

	void gmVirtualWave::setParams(const WaveParams& params) {
		params_ = params;
	}

	float gmVirtualWave::sampleHeight(const tnl::Vector3& pos, float time) const {
		
		float amplitude = params_.amplitude;
		float frequency = params_.frequency;
	
		tnl::Vector3 dirs[4] = {
			{ 1,  0,  0},
			{-1,  0,  0},
			{ 0,  0,  1},
			{ 0,  0, -1},
		};

		float height = 0.0f;
		for (auto& d : dirs) {
			float phase = (d.x * pos.x + d.z * pos.z) * frequency + time;
			height += amplitude * sinf(phase);
		}

		return height;
	}
}