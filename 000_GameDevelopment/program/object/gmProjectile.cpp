// gmProjectile.cpp
#include "gmProjectile.h"
#include "../effect/gmSpriteAnimRegistry.h"
#include "../collision/gmCollisionCategory.h"
#include "../gmGameConfig.h"
#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>

namespace gm {

    // ------------------------------------------------------------
    // コンストラクタ。軌道計算は基底クラス(gmProjectileBase)に任せ、
    // ここでは見た目(火炎のビルボードスプライトアニメーション)だけを構築する。
    // ------------------------------------------------------------
    gmProjectile::gmProjectile(
        const std::string& id,
        const tnl::Vector3& startPos,
        const tnl::Vector3& targetPos,
        const std::shared_ptr<gmSpriteAnimRegistry>& spriteRegistry,
        float horizontalSpeed,
        float arcHeightRatio
    )
        : gmProjectileBase(id, startPos, targetPos, horizontalSpeed, arcHeightRatio, COLLIDER_RADIUS)
    {
        if (!spriteRegistry) return;

        const gmSpriteAnimClip* clip = spriteRegistry->getClip(VFX_CLIP_NAME);
        if (!clip) return;

        Shared<dxe::Texture> tex = spriteRegistry->getTexture(VFX_CLIP_NAME, VFX_EFFECT_GRAPHICS_DIR);

        // ------------------------------------------------------------
        // アニメーションの再生速度(fps)を、飛行時間ちょうどに合わせて計算し直す。
        //
        // 素材本来のfps(既定値)のままワンショット再生すると、アニメーションの
        // 総再生時間(コマ数 ÷ fps、素材本来だと約1.3秒)が固定になってしまう。
        // 一方、弾の飛行時間(flightTime_、基底クラスで距離に応じて計算済み)は
        // 着弾距離に応じて伸び縮みするため、近距離では気づかないが、遠距離だと
        // 飛行時間の方が長くなり、着弾前にアニメーションだけ先に「再生完了」して
        // 消えてしまっていた。
        //
        // → 総コマ数(frameCount)は変えず、
        //     再生速度fps = 総コマ数 ÷ 飛行時間
        //   という式でfpsの方を都度計算し直すことで、距離によらず
        //   「着弾の瞬間にアニメーションもちょうど終わる」ようにする。
        // ------------------------------------------------------------
        gmSpriteAnimClip vfxClip = *clip;
        const int frameCount = std::max(1, vfxClip.frameCount);
        vfxClip.fps = static_cast<float>(frameCount) / flightTime_;

        visual_.start(vfxClip, tex, position_, VISUAL_SIZE, /*sustain=*/false, gmBillboardMode::FaceCamera);

        // 画像素材に合わせて、
        // 弾は宙を飛ぶ飛翔体なので、板ポリの中心をposition_に合わせる
        visual_.setAnchor(gmSpriteAnchor::Center);

        // 初期姿勢も発射時点の速度方向に合わせておく
        // (素材は「頭が左・尾が右」の横向き画像のため、頭側が進行方向を向くよう符号を反転する)
        visual_.setForwardDirection(-tnl::Vector3::Normalize(velocity_));
    }

    // ------------------------------------------------------------
    // 毎フレームの見た目更新。ビルボードの位置・向きを弾の現在位置・
    // 進行方向へ追従させ、アニメーションを進める。
    // ------------------------------------------------------------
    void gmProjectile::updateVisual(const tnl::Vector3& position, const tnl::Vector3& forwardDir, float scaledDeltaTime)
    {
        // 素材は「頭が左・尾が右」の横向き画像のため、頭側が進行方向を向くよう符号を反転する
        // さらに反転用スイッチを指定に応じて適用する
        tnl::Vector3 spriteFacingDir = -forwardDir;
        if (REVERSE_TAIL_DIRECTION) {
            spriteFacingDir = -spriteFacingDir;
        }

        visual_.setPosition(position);
        visual_.setForwardDirection(spriteFacingDir);
        visual_.update(scaledDeltaTime);
    }

    // ------------------------------------------------------------
    // 描画。ビルボードスプライトをそのまま描画するだけ。
    // ------------------------------------------------------------
    void gmProjectile::renderVisual(const Shared<dxe::Camera>& camera)
    {
        visual_.render(camera);
    }

    // ------------------------------------------------------------
    // 衝突時の処理。流氷に当たったら消滅する。
    // 
    // 溶解(耐久値を減らす)処理はgmIceberg::onCollisionEnter側に実装されているため、
    // ここでは自分を消すだけでよい。
    // ------------------------------------------------------------
    void gmProjectile::onCollisionEnter(gmObjectBase* other)
    {
        if (!other) return;

        if (other->getCollisionCategory() == gmCollisionCategory::Iceberg) {
            kill();
        }
    }
}
