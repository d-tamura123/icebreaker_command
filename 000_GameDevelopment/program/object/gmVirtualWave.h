#pragma once

#include <dxe.h>

namespace gm {
	class gmVirtualWave {
	public:
		struct WaveParams {
			float amplitude = 1.0f;		// 振幅
			float frequency = 1.0f;		// 周波数
			float timeScale = 1.0f;		// 時間の進み
		};

		gmVirtualWave() = default;

		// 初期値パラメータ設定
		void setParams(const WaveParams& params);

		// 波の高さを返す(純粋ロジック)
		float sampleHeight(const tnl::Vector3& pos, float time) const;

	private:
		WaveParams params_;
	};
}
