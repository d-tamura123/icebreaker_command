#include "gmShip.h"
#include "../util/gmMeshBoundsUtil.h"
#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>

namespace gm {

	void gmShip::update(float deltaTime) {

		snapshotPosition();			// 移動キャンセルにつかう、移動前の位置情報を退避
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

	// ------------------------------------------------------------
	// メッシュのバウンディング情報からカプセルコライダーを自動生成する
	// ------------------------------------------------------------
	void gmShip::setupDefaultCollider() {
		if (!mesh_) return;

		gmCollider collider;
		collider.type = ColliderShapeType::Capsule;

		// メッシュのバウンディングボックス(全長)から、前後方向(ローカルZ)を
		// カプセルの軸の長さ、幅・高さの短い方を半径として採用する
		// Note: mesh_->getBoundingBoxSize()はDxLibのメッシュインデックス0だけを見るため、
		// 複数パーツで構成されたモデルだと不正確になる。全サブメッシュを合算する
		// ComputeMeshBounds()を使い、確実な値を求める。
		tnl::Vector3 boxCenter, boxSize;
		if (!ComputeMeshBounds(mesh_->getDxMvHdl(), boxCenter, boxSize)) {
			return;
		}
		
		float bodyLength = boxSize.z * SHIP_COLLIDER_LENGTH_SCALE; // 船首・船尾のカバー不足を補うための倍率
		float bodyRadius = std::min(boxSize.x, boxSize.y) * 0.5f;

		collider.radius = bodyRadius;
		// 半球の分(半径×2)を引いた残りが胴体の長さになる
		collider.capsuleHeight = std::max(0.0f, bodyLength - bodyRadius * 2.0f);
		// モデルのローカル原点がバウンディングボックス中心からズレている場合に備え、
		// 中心オフセットもコライダーに反映しておく
		collider.localOffset = boxCenter;

		// gmColliderのカプセルは軸がローカルY軸固定の実装のため、
		// X軸周りに90度回転させて軸をローカルZ(船の前後方向)へ向け直す
		collider.localRotation = tnl::Quaternion::RotationAxis(tnl::Vector3(1, 0, 0), tnl::PI * 0.5f);

		addCollider(collider);
		setCollisionCategory(gmCollisionCategory::Ship);
	}


	// ------------------------------------------------------------
	// コライダーを手動指定で設定する
	// (自動計算が信頼できない場合の代替経路。数値は目視で決め打ちする)
	// ------------------------------------------------------------
	void gmShip::setupManualCollider(float radius, float length, float forwardOffset) {
		gmCollider collider;
		collider.type = ColliderShapeType::Capsule;
		collider.radius = radius;
		collider.capsuleHeight = length;

		// forwardOffsetはローカルZ方向(船首側が正)のオフセットとして扱う
		collider.localOffset = tnl::Vector3(0.0f, 0.0f, forwardOffset);

		// setupDefaultCollider()と同じく、カプセルの軸をローカルZ(前後方向)へ向け直す
		collider.localRotation = tnl::Quaternion::RotationAxis(tnl::Vector3(1, 0, 0), tnl::PI * 0.5f);

		addCollider(collider);
		setCollisionCategory(gmCollisionCategory::Ship);
	}


	// ------------------------------------------------------------
	// 衝突検出イベント
	// 「移動前の位置に丸ごと戻す」ことで移動抑止を実現する
	// ------------------------------------------------------------
	void gmShip::onCollisionEnter(gmObjectBase* other) {
		revertToLastSafePosition();
	}
}