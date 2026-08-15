#include "gmShip.h"
#include "../util/gmMeshBoundsUtil.h"
#include "../collision/gmCollisionCategory.h"
#include "gmIceberg.h"
#include <dxe.h>
#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>
#include <cmath>

namespace gm {

	void gmShip::update(float deltaTime) {
		
		if (state_ == ShipState::Destroyed) {
			updateDestroyed(deltaTime);
			return;
		}

		snapshotPosition();			// 移動キャンセルにつかう、移動前の位置情報を退避
		collidedThisFrame_ = false;	// このフレームの衝突有無を再判定する(onCollisionEnter()が立てる)

		updateEngine(deltaTime);
		updateRudder(deltaTime);
		updateMovement(deltaTime);
		updateWave(deltaTime);
		updateIcebergContactDamage(deltaTime);
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
		// targetRudder は派生クラス(プレイヤー入力 or NPC操舵AI)が設定する。
		//
		// 1秒あたりに変化できる舵角の量を上限で制限する「線形レート制限」。
		// これにより、舵を切り始めてから最大舵角に達するまで、常にRUDDER_RAMP_TIME秒かかる
		// 中立へ戻す時・左右を切り替える時も同じレートで変化する。
		//
		// NPC船もこの関数を共有するため、航路追従AIの操舵も自動的に同じ挙動になる。
		//
		const float rudderChangeRate = 1.0f / RUDDER_RAMP_TIME; // 1秒あたりに変化できる舵角の上限
		const float maxDelta = rudderChangeRate * deltaTime;

		const float diff = dynamics_.targetRudder - dynamics_.rudder;

		if (fabsf(diff) <= maxDelta) {
			dynamics_.rudder = dynamics_.targetRudder; // 目標に十分近ければピッタリ合わせる(オーバーシュート防止)
		}
		else {
			dynamics_.rudder += (diff > 0.0f) ? maxDelta : -maxDelta;
		}
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

		if (state_ == ShipState::Destroyed) return;		// 撃沈演出中(Destroyed状態)は衝突応答そのものを無視する。

		revertToLastSafePosition();

		// HUD表示用の「このフレームは衝突で進めなかった」という情報だけを別途フラグで残す
		collidedThisFrame_ = true;

		applyIcebergContactDamage(other);
	}


	// ------------------------------------------------------------
	// ダメージ・HP管理
	// ------------------------------------------------------------
	void gmShip::applyDamage(float amount, gmObjectBase* source, bool isBigHit)
	{
		if (state_ == ShipState::Destroyed) return; // 既に撃沈演出中なら何もしない

		hp_ = std::max(0.0f, hp_ - amount);
		onDamaged(amount, isBigHit);

		if (hp_ <= 0.0f) {
			state_ = ShipState::Destroyed;
			destroyedElapsed_ = 0.0f;
			destroyedCompleteFired_ = false;
			destroyedStartTiltZ_ = rotation_.z;		// 死亡直前の波による傾きを起点として引き継ぐ (gmWaterPlaneの再計算のため)
			destroyedStartY_ = position_.y;			// 死亡直前の波によるY座標を起点として引き継ぐ (gmWaterPlaneの再計算のため)
			onDeath();
		}
	}

	// ------------------------------------------------------------
	// HP回復
	// ------------------------------------------------------------
	void gmShip::heal(float amount)
	{
		if (state_ == ShipState::Destroyed) return;
		hp_ = std::min(maxHp_, hp_ + amount);
	}

	// ------------------------------------------------------------
	// 流氷との接触ダメージ判定。
	// 氷山インスタンスごとの猶予タイマー(icebergContacts_)を見て、
	// 猶予切れ(または初回接触)なら大ダメージ、そうでなければ「接触した」フラグだけ立てる
	// (継続ダメージの実際の適用はupdateIcebergContactDamage()側、deltaTimeを使える場所で行う)。
	// ------------------------------------------------------------
	void gmShip::applyIcebergContactDamage(gmObjectBase* other)
	{
		if (state_ == ShipState::Destroyed) return;
		if (!other || other->getCollisionCategory() != gmCollisionCategory::Iceberg) return;

		gmIceberg* iceberg = dynamic_cast<gmIceberg*>(other);
		if (!iceberg) return;

		IcebergContact& contact = icebergContacts_[iceberg];
		contact.touchedThisFrame = true;

		if (contact.graceTimer <= 0.0f) {
			applyDamage(SHIP_ICEBERG_BIG_HIT_DAMAGE, iceberg, true);
			contact.justBigHit = true;
		}

		// 接触し続けている間は猶予を毎フレーム最大値へリセットし続ける。
		// (減衰はupdateIcebergContactDamage()側で、離れているフレームのみ行う。
		//  これにより「離れてから猶予秒数が経過して初めて、次の大ダメージが有効になる」
		//  仕様になる。逆に触れ続けている間は猶予が尽きないため、大ダメージは初回のみ)
		contact.graceTimer = SHIP_ICEBERG_CONTACT_GRACE_SEC;
	}

	// ------------------------------------------------------------
	// 流氷接触ダメージの継続分(DoT)適用・猶予タイマーの経過処理。
	// 大ダメージを与えたばかりのフレームはDoTを重ねない(justBigHitで判定)。
	// 接触も途絶え、猶予も切れたエントリはここで間引く。
	// ------------------------------------------------------------
	void gmShip::updateIcebergContactDamage(float deltaTime)
	{
		for (auto it = icebergContacts_.begin(); it != icebergContacts_.end(); ) {
			IcebergContact& contact = it->second;

			if (contact.touchedThisFrame && !contact.justBigHit) {
				applyDamage(SHIP_ICEBERG_DOT_DAMAGE_PER_SEC * deltaTime, it->first, false);
			}

			// 猶予の減衰は「離れているフレーム」だけ行う。接触中はapplyIcebergContactDamage()側で
			// 毎フレーム最大値へリセットされ続けるため、ここで一緒に減らしてしまうと
			// 接触し続けていても猶予が尽きて大ダメージが再発してしまう。
			if (!contact.touchedThisFrame) {
				contact.graceTimer = std::max(0.0f, contact.graceTimer - deltaTime);
			}

			const bool stillRelevant = contact.touchedThisFrame || contact.graceTimer > 0.0f;

			contact.touchedThisFrame = false;
			contact.justBigHit = false;

			if (!stillRelevant) {
				it = icebergContacts_.erase(it); // 接触も猶予も切れた氷山は追跡不要になるため間引く
			}
			else {
				++it;
			}

			// Note:
			// applyDamage()がonDeath()経由でstate_をDestroyedへ変えている場合があるが、
			// このループ自体はicebergContacts_の整理を続けるだけなので、そのまま最後まで回して問題ない。
		}
	}

	// ------------------------------------------------------------
	// Destroyed状態の更新。
	//   死亡時点の傾き・Y座標を起点に、経過時間の割合(t)をイーズイン(t^EASE_POWER)した上で、
	//   傾き・沈み込みとも「起点→目標値」を直接補間する(始めはゆっくり、徐々に加速していく)。
	// 
	//   両者は全体の尺(SHIP_DESTROYED_DURATION)の中で別々の時間区間を使う
	//   (TILT_PORTION/SINK_START_PORTION参照)。同時に同じ速さで進めると、水中がほぼ
	//   不透明で見えないせいで「傾いている最中に見えなくなる」形になり、傾き演出が
	//   ほとんど視認できなかったため、傾きを先に見せ、沈み込みは終盤に回す時間差を付けている。
	// 
	//   SHIP_DESTROYED_DURATION秒でonDestroyedComplete()を1回呼ぶ。
	// ------------------------------------------------------------
	void gmShip::updateDestroyed(float deltaTime)
	{
		destroyedElapsed_ += deltaTime;

		const float t = std::clamp(destroyedElapsed_ / SHIP_DESTROYED_DURATION, 0.0f, 1.0f);
		
		// ---- 傾き: 全体の0〜TILT_PORTIONの区間で完了させる ----
		const float tiltT = std::clamp(t / SHIP_DESTROYED_TILT_PORTION, 0.0f, 1.0f);
		const float easedTiltT = powf(tiltT, SHIP_DESTROYED_EASE_POWER);
		rotation_.z = destroyedStartTiltZ_ + (SHIP_DESTROYED_TILT_TARGET_RAD - destroyedStartTiltZ_) * easedTiltT;

		// ---- 沈み込み: 全体のSINK_START_PORTION〜1.0の区間で進める ----
		const float sinkT = std::clamp(
			(t - SHIP_DESTROYED_SINK_START_PORTION) / (1.0f - SHIP_DESTROYED_SINK_START_PORTION),
			0.0f, 1.0f);
		const float easedSinkT = powf(sinkT, SHIP_DESTROYED_EASE_POWER);
		position_.y = destroyedStartY_ - SHIP_DESTROYED_SUBMERGE_DEPTH * easedSinkT;

		// ---- 完了判定 ----
		if (!destroyedCompleteFired_ && t >= 1.0f) {
			destroyedCompleteFired_ = true;
			onDestroyedComplete();
		}
	}

	// ------------------------------------------------------------
	// Destroyed状態を解除し、通常状態に戻す。
	// ------------------------------------------------------------
	void gmShip::resetToNormalState()
	{
		state_					= ShipState::Normal;
		hp_						= maxHp_;
		rotation_.x				= 0.0f;
		rotation_.z				= 0.0f;			// 傾きだけ戻す(yaw/rotation_.yは呼び出し元がsetYaw()で別途設定する想定)
		destroyedElapsed_		= 0.0f;
		destroyedCompleteFired_	= false;
		icebergContacts_.clear();				// 再配置後に古い猶予情報を引き継がないようにする

		// 操作状態(速度・舵角)のリセット。死亡前の速度・舵角を引き継いだまま復活すると、
		// 再配置直後に勝手に動き出す・曲がり始めるという不自然な挙動になるため。
		dynamics_.speed			= 0.0f;
		dynamics_.targetSpeed	= 0.0f;
		dynamics_.rudder		= 0.0f;
		dynamics_.targetRudder	= 0.0f;
		speedIndex_				= 2;			// 停止(SPEED_LEVELSの中央)

		// updateWave()の傾き計算用にじわじわ追従するwavePos_を{0,0,0}に戻す。
		// これを忘れると、死亡地点付近の古いwavePos_が再配置後の新しい位置とかけ離れたまま
		// 残ってしまい、updateWave()冒頭の「wavePos_が{0,0,0}なら現在位置で初期化する」判定を
		// 頼りに再初期化させる必要がある(戻さないと、無関係な地点の波サンプルをもとに
		// 傾きが計算され、じわじわ追いつくまでの数フレーム、傾きがカクカクする不具合になる)。
		wavePos_ = tnl::Vector3(0.0f, 0.0f, 0.0f);
	}
}