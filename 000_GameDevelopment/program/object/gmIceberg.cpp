#include "gmIceberg.h"
#include "../map/gmMapManager.h"
#include "gmWaterPlane.h"
#include "../gmGameConfig.h"
#include "../util/gmMeshBoundsUtil.h"
#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>
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
        spinSpeed_ = tnl::GetRandomDistribution<float>(-0.15f, 0.15f);

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

                gmCollider collider;
                collider.type = ColliderShapeType::Sphere;
                collider.radius = (maxExtent * 0.5f) * ICEBERG_COLLIDER_RADIUS_SCALE;
                collider.localOffset = boxCenter;
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

    void gmIceberg::update(float deltaTime)
    {
        snapshotPosition();     // 移動キャンセルにつかう、移動前の位置情報を退避
        updateDrift(deltaTime);
        updateWave(deltaTime);
        updateSpin(deltaTime);
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

        position_ += velocity_ * deltaTime;
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
            // TODO: 砲弾/火炎で溶かす攻撃の処理(未実装)。
            // ダメージ・破壊・kill()呼び出し等をここに追加する。
            break;

        default:
            // 船・島・他の氷山との衝突は、移動前の位置に丸ごと戻して移動抑止する
            revertToLastSafePosition();
            break;
        }
    }
}
