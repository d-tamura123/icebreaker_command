// gmFlameThrowerAttack.cpp
#include "gmFlameThrowerAttack.h"
#include "../effect/gmSpriteAnimRegistry.h"
#include "../collision/gmCollisionCategory.h"
#include "../gmGameConfig.h"
#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>
#include <cmath>

namespace gm {

    // ------------------------------------------------------------
    // コンストラクタ。
    //   1. 中心方向(centerDir)を基準に、扇の中のライン方向を本数ぶん計算する
    //   2. ライン1本ごとに、当たり判定用のカプセルコライダーを1本追加する
    //   3. ライン1本ごとに、起点から先端まで1本の細長い板ポリを配置する
    // ------------------------------------------------------------
    gmFlameThrowerAttack::gmFlameThrowerAttack(
        const std::string& id,
        const tnl::Vector3& originPos,
        const tnl::Vector3& centerDir,
        const std::shared_ptr<gmSpriteAnimRegistry>& spriteRegistry,
        float fanAngleDeg,
        int lineCount,
        float range,
        float duration
    )
        : gmObjectBase(id, originPos)
        , duration_(duration)
    {
        setCollisionCategory(gmCollisionCategory::Projectile);

        lineCount = std::max(1, lineCount);
        lineVisuals_.resize(static_cast<size_t>(lineCount));

        // 中心方向をXZ平面へ正規化(Y成分は無視する。攻撃は常に水平方向へ広がる想定)
        tnl::Vector3 flatCenterDir(centerDir.x, 0.0f, centerDir.z);
        const float centerLenSq = tnl::Vector3::Dot(flatCenterDir, flatCenterDir);
        flatCenterDir = (centerLenSq > 1e-8f) ? tnl::Vector3::Normalize(flatCenterDir) : tnl::Vector3(0.0f, 0.0f, 1.0f);

        // 中心方向を角度(atan2)に変換しておき、扇の中のライン角度をこの角度からのオフセットで求める。
        // gmShip::getForward()と同じ規約(角度0のとき+Z方向、角度が増えると+X方向へ回る)に合わせてある。
        const float centerAngle = atan2f(flatCenterDir.x, flatCenterDir.z);
        const float fanAngleRad = tnl::ToRadian(fanAngleDeg);
        const float halfFan = fanAngleRad * 0.5f;

        Shared<dxe::Texture> tex;
        const gmSpriteAnimClip* clip = nullptr;
        if (spriteRegistry) {
            clip = spriteRegistry->getClip(VFX_CLIP_NAME);
            if (clip) {
                tex = spriteRegistry->getTexture(VFX_CLIP_NAME, VFX_EFFECT_GRAPHICS_DIR);
            }
        }

        for (int i = 0; i < lineCount; ++i) {
            // ---- ラインごとの方向を求める ----
            // lineCount==1ならそのまま中心方向、2本以上なら-halfFan〜+halfFanへ均等割り
            const float t = (lineCount == 1) ? 0.5f : static_cast<float>(i) / static_cast<float>(lineCount - 1);
            const float lineAngle = centerAngle - halfFan + t * fanAngleRad;
            const tnl::Vector3 lineDir(sinf(lineAngle), 0.0f, cosf(lineAngle));

            // ---- 当たり判定: ライン1本 = カプセル1本 ----
            // カプセルの軸は仕様上ローカルY軸固定のため、localRotationで
            // 「ローカルY軸 → lineDir」となる回転を組み立てる。
            // Y軸とlineDir(XZ平面上のベクトル)は常に直交するため、
            // 「両者に直交する軸まわりに90度回す」だけで狙った向きになる
            // (gmShip側の「Y軸→Z軸(前方)」に90度回転で対応させているのと同じ考え方)。
            {
                tnl::Vector3 rotAxis = tnl::Vector3::Cross(tnl::Vector3(0.0f, 1.0f, 0.0f), lineDir);
                const float rotAxisLenSq = tnl::Vector3::Dot(rotAxis, rotAxis);
                rotAxis = (rotAxisLenSq > 1e-8f) ? tnl::Vector3::Normalize(rotAxis) : tnl::Vector3(1.0f, 0.0f, 0.0f);

                const float rotAngle = CAPSULE_AXIS_FLIP ? -tnl::PI * 0.5f : tnl::PI * 0.5f;

                gmCollider collider;
                collider.type = ColliderShapeType::Capsule;
                collider.radius = LINE_COLLIDER_RADIUS;
                collider.capsuleHeight = range;
                // 起点(originPos = 自分のposition_)から先端へ、ちょうどrangeぶん伸びるように、
                // カプセル中心をlineDir方向へ半分だけオフセットする
                collider.localOffset = lineDir * (range * 0.5f);
                collider.localRotation = tnl::Quaternion::RotationAxis(rotAxis, rotAngle);
                addCollider(collider);
            }

            // ---- 見た目: 起点から先端まで、1本の細長い板ポリで表現する ----
            if (clip) {
                const tnl::Vector3 linePos = originPos + lineDir * (range * 0.5f); // ラインの中点(Center基準)

                gmSpriteAnimInstance& lineVisual = lineVisuals_[static_cast<size_t>(i)];
                lineVisual.start(*clip, tex, linePos, LINE_VISUAL_WIDTH, /*sustain=*/true, gmBillboardMode::FaceCamera);
                lineVisual.setAnchor(gmSpriteAnchor::Center);
                lineVisual.setForwardLength(range); // 進行方向(=ラインの長さ)だけsize_(太さ)と切り離して伸ばす
                lineVisual.setUInset(VISUAL_HEAD_UV_INSET, VISUAL_TAIL_UV_INSET); // 素材の透明な余白ぶんを詰める

                // tktk_Fire_10は「頭(玉)が左・尾が右」の横向き素材を想定。
                // 火元(玉)を起点側、尾を先端側に見立てたいので、その向きへforwardDir_を合わせる
                tnl::Vector3 facingDir = -lineDir;
                if (REVERSE_TAIL_DIRECTION) {
                    facingDir = -facingDir;
                }
                lineVisual.setForwardDirection(facingDir);
            }
        }
    }

    // ------------------------------------------------------------
    // 毎フレーム更新。
    //   ・全ビルボードのアニメーションを進める(位置は固定なので更新不要)
    //   ・duration_が経過したら、全ビルボードへ一斉にrequestStop()を送る
    //   ・全ビルボードの再生が完全に終わったら、自分自身をkill()する
    //     (kill()されるとgmFlameThrowerManager::update()がリストから取り除き、
    //      衝突システム側もweak_ptr経由で自動的に外れる)
    // ------------------------------------------------------------
    void gmFlameThrowerAttack::update(float deltaTime)
    {
        elapsedTime_ += deltaTime;

        for (auto& seg : lineVisuals_) {
            seg.update(deltaTime);
        }

        if (!stopRequested_ && elapsedTime_ >= duration_) {
            requestStop();
        }

        if (stopRequested_) {
            const bool allFinished = std::all_of(lineVisuals_.begin(), lineVisuals_.end(),
                [](const gmSpriteAnimInstance& seg) { return seg.isFinished(); });

            if (allFinished) {
                kill();
            }
        }
    }

    // ------------------------------------------------------------
    // 描画。全ビルボードをそのまま描画するだけ。
    // ------------------------------------------------------------
    void gmFlameThrowerAttack::render(const Shared<dxe::Camera>& camera)
    {
        for (auto& seg : lineVisuals_) {
            seg.render(camera);
        }
    }

    // ------------------------------------------------------------
    // 外部から途中終了させたい場合の入口。中身はupdate()内のロジックと同じ
    // タイミングで全ビルボードへ伝搬させるため、フラグを立てるだけにしてある。
    // ------------------------------------------------------------
    void gmFlameThrowerAttack::requestStop()
    {
        if (stopRequested_) return;

        stopRequested_ = true;
        for (auto& seg : lineVisuals_) {
            seg.requestStop();
        }
    }

    // ------------------------------------------------------------
    // 衝突時の処理。
    // 弾(gmProjectile等)と違い、自分自身は命中しても消えない
    // (durationが尽きるまで扇全体が持続する持続系攻撃のため)。
    // ------------------------------------------------------------
    void gmFlameThrowerAttack::onCollisionEnter(gmObjectBase* other)
    {
        if (!other) return;

        if (other->getCollisionCategory() == gmCollisionCategory::Iceberg) {
            // TODO: 流氷へのダメージ処理(他の攻撃と同様、次のステップで実装)。
            // 持続攻撃のため、同じ氷山に対して複数フレームぶん連続でここへ
            // 入ってくる点に注意(1回だけ効かせたいなら、氷山側で
            // 直前ヒットからの経過時間などのクールダウンを持たせる想定)。
        }
    }
}
