#include "gmIceberg.h"
#include "../map/gmMapManager.h"
#include "../wallet/gmWallet.h"
#include "gmWaterPlane.h"
#include "../gmGameConfig.h"
#include "../util/gmMeshBoundsUtil.h"
#include "gmSplitProjectile.h"
#include "gmFlameThrowerAttack.h"

#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>
#include <cmath>
#include <dxe.h>

namespace gm {
    
    // ヘルパー関数
    namespace {
        // FPSに依存しない指数減衰のブレンド係数を計算する
        // ratePerSecond が大きいほど目標値に素早く近づく(時定数の逆数に相当)
        float FrameRateIndependentBlend(float ratePerSecond, float deltaTime)
        {
            return 1.0f - std::expf(-ratePerSecond * deltaTime);
        }
    }

    gmIceberg::gmIceberg(const std::string& id, const tnl::Vector3& pos, const Shared<dxe::Mesh>& mesh)
        : gmMeshBase(id, pos)
    {
        mesh_ = mesh;

        // 個体ごとに自転の向き・速さをばらつかせる(演出用)
        spinSpeed_ = tnl::GetRandomDistribution<float>(-0.02f, 0.02f);

        // 球コライダーを自動設定する
        // Note: mesh_->getBoundingSphereRadius()はDxLibのメッシュインデックス0だけを
        // 見て計算されるため、複数パーツで構成されたメッシュだと不正確になりうる。
        //
        // 全サブメッシュを合算するComputeMeshBounds()から求めるが、
        // 対角線の半分だと最も保守的(大きめ)になりすぎるため、
        // 最大軸の半分を採用したうえでICEBERG_COLLIDER_RADIUS_SCALEで微調整する。
        if (mesh_) {
            tnl::Vector3 boxCenter, boxSize;
            if (ComputeMeshBounds(mesh_->getDxMvHdl(), boxCenter, boxSize)) {
                const float maxExtent = std::max({ boxSize.x, boxSize.y, boxSize.z });

                baseColliderRadius_ = (maxExtent * 0.5f) * ICEBERG_COLLIDER_RADIUS_SCALE;
                baseColliderOffset_ = boxCenter;

                gmCollider collider;
                collider.type = ColliderShapeType::Sphere;
                collider.radius = baseColliderRadius_;
                collider.localOffset = baseColliderOffset_;
                addCollider(collider);
            }
        }
        setCollisionCategory(gmCollisionCategory::Iceberg);
    }

    void gmIceberg::setMap(const std::shared_ptr<gmMapManager>& map)
    {
        map_ = map;
    }

    void gmIceberg::setWater(const std::shared_ptr<gmWaterPlane>& water)
    {
        water_ = water;
    }

    void gmIceberg::setWallet(const std::shared_ptr<gmWallet>& wallet)
    {
        wallet_ = wallet;
    }

    void gmIceberg::update(float deltaTime)
    {
        snapshotPosition();     // 移動キャンセルにつかう、移動前の位置情報を退避
        updateDrift(deltaTime);
        updateWave(deltaTime);
        updateSpin(deltaTime);

        // 火炎放射(持続攻撃)のダメージは、onCollisionEnter側では即時適用せず、
        // ここで自分のdeltaTimeを使ってフレームレートに依存しない形で適用する
        if (flameHitThisFrame_) {
            applyMeltDamage(MELT_DAMAGE_PER_SEC_FLAME * getFlameDamageMultiplier() * deltaTime);
            flameHitThisFrame_ = false;
        }

        // 見た目のスケール・コライダーを、health_の目標値へ向けて徐々に近づける
        // (アニメーション表現の演出のため)
        updateMeltVisual(deltaTime);

        // コリジョンを無視するタイマー
        if (collisionGraceTimer_ > 0.0f) {
            collisionGraceTimer_ -= deltaTime;
        }

        checkOutOfBounds();
    }

    void gmIceberg::render(const Shared<dxe::Camera>& camera)
    {
        // カメラからRENDER_DISTANCEを超えて離れている場合は描画を省く
        tnl::Vector3 camPos = camera->getPosition();

        const float dx = position_.x - camPos.x;
        const float dz = position_.z - camPos.z;

        if ((dx * dx + dz * dz) > RENDER_DISTANCE_SQ) {
            return;
        }

        gmMeshBase::render(camera);
    }

    // ------------------------------------------------------------
    // 海流+慣性による移動
    // ------------------------------------------------------------
    void gmIceberg::updateDrift(float deltaTime)
    {
        auto map = map_.lock();
        if (!map) return;

        // ワールド座標→グリッド座標(浮動小数点)
        // worldZ = -y * CELL_SIZE の規則に従い、Z→Yへは符号反転して変換する
        const float gridX = position_.x / CELL_SIZE;
        const float gridY = -position_.z / CELL_SIZE;

        const float clampedGridX = std::clamp(gridX, 0.0f, static_cast<float>(MAP_CHIP_WIDTH - 1) - 0.001f);
        const float clampedGridY = std::clamp(gridY, 0.0f, static_cast<float>(MAP_CHIP_HEIGHT - 1) - 0.001f);
        
        tnl::Vector2f flow = map->SampleFlowFloat(clampedGridX, clampedGridY);

        tnl::Vector3 targetVelocity{ flow.x * DRIFT_SCALE, 0.0f, flow.y * DRIFT_SCALE };

        // 慣性追従(gmShipのエンジン挙動と同じ考え方。値を小さくして重さを表現する)
        const float blend = FrameRateIndependentBlend(INERTIA_RATE_PER_SEC, deltaTime);
        velocity_ += (targetVelocity - velocity_) * blend;

        // 分裂の押し出し(splitPushVelocity_)は、海流追従(velocity_)とは別軸で
        // 独立に、専用の速いレートで0へ減衰させる
        if (splitPushVelocity_.x != 0.0f || splitPushVelocity_.y != 0.0f || splitPushVelocity_.z != 0.0f) {
            const float pushBlend = FrameRateIndependentBlend(SPLIT_PUSH_DECAY_RATE_PER_SEC, deltaTime);
            splitPushVelocity_ -= splitPushVelocity_ * pushBlend;
        }

        position_ += (velocity_ + splitPushVelocity_) * deltaTime;
    }

    // ------------------------------------------------------------
    // 波の揺らぎ(gmShipと同じ仕組みだが、係数で薄めて重量感を出す)
    // ------------------------------------------------------------
    void gmIceberg::updateWave(float deltaTime)
    {
        auto water = water_.lock();
        if (!water) return;

        waveTime_ += deltaTime * water->getTimeScale();

        // Y座標: サンプル高さへ一部だけ寄せる(全部寄せるとgmShipと同じ軽さになる)
        const float rawHeight = water->sampleHeight(position_, waveTime_);
        position_.y += (rawHeight - position_.y) * WAVE_DAMPING;

        // 傾き: 前後左右のサンプル差分から求め、同じく係数で薄める
        const float d = 15.0f;
        const float hL = water->sampleHeight(position_ + tnl::Vector3(-d, 0.0f, 0.0f), waveTime_);
        const float hR = water->sampleHeight(position_ + tnl::Vector3(d, 0.0f, 0.0f), waveTime_);
        const float hF = water->sampleHeight(position_ + tnl::Vector3(0.0f, 0.0f, d), waveTime_);
        const float hB = water->sampleHeight(position_ + tnl::Vector3(0.0f, 0.0f, -d), waveTime_);

        const float rawRoll = (hR - hL) * 0.01f;
        const float rawPitch = (hF - hB) * 0.01f;

        rotation_.x = rawPitch * TILT_DAMPING;
        rotation_.z = rawRoll * TILT_DAMPING;
    }

    // ------------------------------------------------------------
    // ゆるやかな自転
    // 船と違って「向き」の概念がないため、海流方向には合わせない
    // ------------------------------------------------------------
    void gmIceberg::updateSpin(float deltaTime)
    {
        rotation_.y += spinSpeed_ * deltaTime;
    }

    // ------------------------------------------------------------
    // マップ範囲外まで流されたら消滅させる
    // ------------------------------------------------------------
    void gmIceberg::checkOutOfBounds()
    {
        const float mapWidthWorld = MAP_CHIP_WIDTH * CELL_SIZE;
        const float mapHeightWorld = MAP_CHIP_HEIGHT * CELL_SIZE;

        const bool outOfX =
            position_.x < -OUT_OF_BOUNDS_MARGIN ||
            position_.x > mapWidthWorld + OUT_OF_BOUNDS_MARGIN;

        // worldZ = -y*CELL_SIZE のため、0付近が北端、-mapHeightWorld付近が南端
        const bool outOfZ =
            position_.z > OUT_OF_BOUNDS_MARGIN ||
            position_.z < -mapHeightWorld - OUT_OF_BOUNDS_MARGIN;

        if (outOfX || outOfZ) {
            kill();
        }
    }

    // ------------------------------------------------------------
    // 衝突イベント
    // : 相手のカテゴリによって挙動を分岐する
    // ------------------------------------------------------------
    void gmIceberg::onCollisionEnter(gmObjectBase* other)
    {
        if (!other) return;

        switch (other->getCollisionCategory()) {
        case gmCollisionCategory::Projectile:
            if (auto* splitProjectile = dynamic_cast<gmSplitProjectile*>(other)) {
                // 割る弾: 耐久値ではなく「ティア」を1段階落とす。
                // 小(これ以上分裂できない)は割る弾に対して無反応(何もしない)。
                // それ以外なら分裂の意思表示だけしておき、
                // 実際の生成はgmIcebergManager側(onPostUpdate)に任せる。
                if (tier_ == Tier::Small) {
                    // これ以上は割れない
                    // ゲーム仕様として、なにもしない
                    // 通常弾・火炎放射による溶解は引き続き有効。
                }
                else if (!pendingSplit_) {
                    // 命中方向(弾の飛んできた向き)の水平成分を分裂の押し出し軸にする。
                    // ほぼ真上から等、水平成分がほぼ無い場合はランダム軸にフォールバックする。
                    tnl::Vector3 dir = splitProjectile->getVelocity();
                    dir.y = 0.0f;
                    const float lenSq = tnl::Vector3::Dot(dir, dir);
                    if (lenSq > 1e-6f) {
                        pendingSplitDir_ = tnl::Vector3::Normalize(dir);
                    }
                    else {
                        const float randomAngle = tnl::GetRandomDistribution<float>(0.0f, tnl::PI * 2.0f);
                        pendingSplitDir_ = tnl::Vector3(sinf(randomAngle), 0.0f, cosf(randomAngle));
                    }
                    pendingSplit_ = true;
                }
            }
            else if (dynamic_cast<gmFlameThrowerAttack*>(other)) {
                // 火炎放射: 持続的に何度もここへ入ってくるため、即時適用せずフラグだけ立てる
                // (実際の減衰量はupdate()側で自分のdeltaTimeを使って計算する)
                flameHitThisFrame_ = true;
            }
            else {
                // 通常弾など、それ以外の攻撃は固定量×ティア倍率だけ溶かす
                applyMeltDamage(MELT_DAMAGE_PER_SHOT * getShotDamageMultiplier());
            }
            break;

        default:
            // 船・島・他の氷山との衝突は、移動前の位置に丸ごと戻して移動抑止する。
            // ただし、分裂直後の猶予期間中(collisionGraceTimer_>0)は、
            // 他の氷山どうしの押し戻しだけは無視する(同座標で生まれた兄弟フラグメントが
            // 押し出しで十分に離れるまで、お互いを固定してしまうのを防ぐため)。
            // 船・島との衝突は猶予期間中も引き続き有効(すり抜けさせない)。
            if (collisionGraceTimer_ > 0.0f && other->getCollisionCategory() == gmCollisionCategory::Iceberg) {
                break;
            }
            revertToLastSafePosition();
            break;
        }
    }


    // ------------------------------------------------------------
    // 分裂の意思表示を取り出す(呼ぶと同時にリセットされる)
    // ------------------------------------------------------------
    tnl::Vector3 gmIceberg::consumePendingSplitDirection()
    {
        pendingSplit_ = false;
        return pendingSplitDir_;
    }

    // ------------------------------------------------------------
    // 耐久値を減らし、見た目(スケール)とコライダーを追従させる。
    // ダメージ量に応じた「溶かす経験値」をウォレットへ加算する。
    // 0まで減ったら(=完全に溶けたら)kill()する。
    // (通常弾・炎放射どちらのダメージもここを通るため、ここ1箇所で完結)
    // ------------------------------------------------------------
    void gmIceberg::applyMeltDamage(float amount)
    {
        if (amount <= 0.0f || health_ <= 0.0f) return;

        // ウォレットへ加算
        if (auto wallet = wallet_.lock()) {
            wallet->addMeltExp(amount * MELT_EXP_PER_DAMAGE_POINT);
        }
        
        // ダメージ算出とkill()
        health_ = std::max(0.0f, health_ - amount);

        if (health_ <= 0.0f) {
            kill();
        }
    }

    // ------------------------------------------------------------
    // displayScale_(見た目に表示中のスケール)を、health_由来の目標値へ
    // 毎フレーム徐々に近づける。
    // 
    // FrameRateIndependentBlendと同じ考え方で、
    // 差分量に比例して速度が変わるため、ダメージが大きいほど最初は
    // 素早く縮み、目標に近づくほど自然に減速する
    // (桁の大きい数字ほど勢いよく回るパタパタ式カウンターのような動き)。
    // ------------------------------------------------------------
    void gmIceberg::updateMeltVisual(float deltaTime)
    {
        const float targetScale = std::max(MIN_VISIBLE_SCALE, health_);

        if (std::fabs(displayScale_ - targetScale) < 1e-4f) {
            displayScale_ = targetScale; // 誤差が十分小さければ、以後の計算を省いてピタッと合わせる
        }
        else {
            const float blend = FrameRateIndependentBlend(MELT_VISUAL_BLEND_RATE_PER_SEC, deltaTime);
            displayScale_ += (targetScale - displayScale_) * blend;
        }

        scale_ = tnl::Vector3(displayScale_, displayScale_, displayScale_);

        if (!colliders_.empty()) {
            colliders_[0].radius = baseColliderRadius_ * displayScale_;
            colliders_[0].localOffset = baseColliderOffset_ * displayScale_;
        }
    }

    // ------------------------------------------------------------
    // ティア別のダメージ倍率。
    // ------------------------------------------------------------
    float gmIceberg::getShotDamageMultiplier() const
    {
        switch (tier_) {
        case Tier::Small:  return SHOT_DAMAGE_MULT_SMALL;
        case Tier::Medium: return SHOT_DAMAGE_MULT_MEDIUM;
        case Tier::Large:  return SHOT_DAMAGE_MULT_LARGE;
        }
        return 1.0f;
    }

    float gmIceberg::getFlameDamageMultiplier() const
    {
        switch (tier_) {
        case Tier::Small:  return FLAME_DAMAGE_MULT_SMALL;
        case Tier::Medium: return FLAME_DAMAGE_MULT_MEDIUM;
        case Tier::Large:  return FLAME_DAMAGE_MULT_LARGE;
        }
        return 1.0f;
    }
}
