// gmProjectile.h
#pragma once
#include "gmProjectileBase.h"
#include "../effect/gmSpriteAnimInstance.h"
#include <memory>

namespace gm {

    class gmSpriteAnimRegistry;

    // ------------------------------------------------------------
    // 通常弾(火炎砲弾): クリックした地点まで放物線を描いて飛ぶ、流氷を溶かす攻撃手段。
    //
    // 見た目は、進行方向を「上」軸として固定した軸固定ビルボードの
    // スプライトアニメーション(火炎)で表現する(gmMeshBaseは継承しない)。
    // 尾を引く画像素材を使う想定のため、重力で刻々と変わる速度ベクトルを
    // 毎フレーム反映し、常に進行方向へ尾がなびいて見えるようにする。
    //
    // 軌道計算(飛行時間・山なりの高さ等)はgmProjectileBaseで共通化されている。
    // ------------------------------------------------------------
    class gmProjectile : public gmProjectileBase {
    public:
        // arg1... 識別ID
        // arg2... 発射位置(ワールド座標)
        // arg3... 着弾目標位置(ワールド座標)
        // arg4... スプライトアニメーションのメタデータ/テクスチャを引くためのレジストリ
        gmProjectile(
            const std::string& id,
            const tnl::Vector3& startPos,
            const tnl::Vector3& targetPos,
            const std::shared_ptr<gmSpriteAnimRegistry>& spriteRegistry,
            float horizontalSpeed = DEFAULT_HORIZONTAL_SPEED,
            float arcHeightRatio = DEFAULT_ARC_HEIGHT_RATIO
        );

        void onCollisionEnter(gmObjectBase* other) override;

    protected:
        void updateVisual(const tnl::Vector3& position, const tnl::Vector3& forwardDir, float scaledDeltaTime) override;
        void renderVisual(const Shared<dxe::Camera>& camera) override;

    private:
        gmSpriteAnimInstance visual_;

        // ---- 調整用パラメータ ----
        // 偏差射撃(未来位置予測)のテクニックを要求するゲーム性のため、
        // 意図的に遅め・山なりがやや大きめの値にしてある。
        // TODO:
        // 将来の武器強化要素で、コンストラクタ引数から上書きする想定。
        static constexpr float DEFAULT_ARC_HEIGHT_RATIO = 0.2f;             // 山の高さ ÷ 水平距離 の比率。大きいほど山が高くなる
        static constexpr float DEFAULT_HORIZONTAL_SPEED = 120.0f;           // 水平方向の速さ(world単位/秒)。近距離〜基準距離まではこれで飛行時間が比例して伸びる
        static constexpr float VISUAL_SIZE = 60.0f;                         // 弾のビルボードの大きさ(world単位)
        static constexpr float COLLIDER_RADIUS = 15.0f;                     // 弾の当たり判定の半径
        static constexpr const char* VFX_CLIP_NAME = "tktk_Fire_10";        // 弾の見た目に使うクリップ名

        // 画像素材の「上」方向が、実際には画像の見た目と逆(頭側)を
        // 向いてしまう場合にtrueにすると、進行方向ベクトルを反転して使う
        static constexpr bool REVERSE_TAIL_DIRECTION = false;
    };
}
