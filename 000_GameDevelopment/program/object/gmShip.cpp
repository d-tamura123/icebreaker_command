#include "gmShip.h"

namespace gm {

	void gmShip::update(float deltaTime) {
		updateEngine(deltaTime);
		updateRudder(deltaTime);
		updateMovement(deltaTime);
		updateWave(deltaTime);
	}

	void gmShip::render(const Shared<dxe::Camera>& camera) /*override*/ {

		if (!mesh_) return;
		
		gmMeshBase::render(camera);
	}
	
	void gmShip::setWater(const std::shared_ptr<gmWaterPlane>& water) {
		water_ = water;
	}

	std::shared_ptr<gmWaterPlane> gmShip::getWater() const {
		return water_.lock();
	}

	// -----------------------------
	// 速度段階・慣性
	// -----------------------------
	void gmShip::updateEngine(float deltaTime) {

		// targetSpeed は派生クラス（プレイヤー or NPC）が設定する
		// ここでは慣性追従だけ行う
		dynamics_.speed += (dynamics_.targetSpeed - dynamics_.speed) * 0.02f;
	}

	// -----------------------------
	// 舵角
	// -----------------------------
	void gmShip::updateRudder(float deltaTime)
	{
		// targetRudder も派生クラスが設定する
		dynamics_.rudder += (dynamics_.targetRudder - dynamics_.rudder) * 0.1f;
	}

	// -----------------------------
	// 前進・旋回
	// -----------------------------
	void gmShip::updateMovement(float deltaTime)
	{
		// 船の向き更新
		dynamics_.yaw += dynamics_.rudder * fabsf(dynamics_.speed) * 0.01f;

		// 前進方向
		tnl::Vector3 dir = {
			sinf(dynamics_.yaw),
			0,
			cosf(dynamics_.yaw)
		};

		// 位置更新
		position_ += dir * dynamics_.speed * 2.0f;

		// Euler 角として rotation_ に反映
		rotation_.y = dynamics_.yaw;
	}

	// -----------------------------
	// 波同期・傾き
	// -----------------------------
	void gmShip::updateWave(float deltaTime)
	{
		auto water = water_.lock();
		if (!water) return;

		waveTime_ += deltaTime * water->getTimeScale();

		// 波の高さ
		position_.y = water->sampleHeight(position_, waveTime_);

		// 傾き計算
		float d = 15.0f;
		if (wavePos_.x == 0 && wavePos_.y == 0 && wavePos_.z == 0) {
			wavePos_ = position_;
		}
		wavePos_ += (position_ - wavePos_) * 0.05f;

		float hL = water->sampleHeight(wavePos_ + tnl::Vector3(-d,  0,  0), waveTime_);
		float hR = water->sampleHeight(wavePos_ + tnl::Vector3( d,  0,  0), waveTime_);
		float hF = water->sampleHeight(wavePos_ + tnl::Vector3( 0,  0,  d), waveTime_);
		float hB = water->sampleHeight(wavePos_ + tnl::Vector3( 0,  0, -d), waveTime_);

		float s_roll = (hR - hL) * 0.01f;
		float s_pitch = (hF - hB) * 0.01f;

		// Euler 角として rotation_ に反映
		rotation_.x = s_pitch;
		rotation_.z = s_roll;
	}

	// Yawのセッター
	void gmShip::setYaw(float yaw) {
		dynamics_.yaw = yaw;
		rotation_.y = yaw;
	}

	// Yawのゲッター
	float gmShip::getYaw() const {
		return dynamics_.yaw;
	}
}