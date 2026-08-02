// gmSplitProjectile.h
#pragma once
#include "gmProjectileBase.h"
#include "../effect/gmSpriteAnimInstance.h"
#include <dxe.h>
#include <memory>

namespace gm {

    class gmSpriteAnimRegistry;

    // ------------------------------------------------------------
    // 割り砲弾: gmProjectileと同じ放物線軌道(gmProjectileBase)で飛ぶが、
    // 見た目は「風を切るスプライト(ビルボード)」と「黒い3D球(鉄球)」の
    // 組み合わせで表現する。命中した流氷は溶けるのではなく分裂する。
    // ------------------------------------------------------------
    class gmSplitProjectile : public gmProjectileBase {
    public:
        // arg1... 識別ID
        // arg2... 発射位置(ワールド座標)
        // arg3... 着弾目標位置(ワールド座標)
        // arg4... スプライトアニメーションのメタデータ/テクスチャを引くためのレジストリ(風を切るトレイル用)
        gmSplitProjectile(
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
        gmSpriteAnimInstance trailVisual_;      // 風を切るビルボードトレイル
        Shared<dxe::Mesh>    ballMesh_;         // 鉄球(黒い3D球)
        tnl::Vector3         trailFollowPos_;   // トレイルの実際の表示位置(目標地点を遅延追従した結果。毎フレーム更新し、フレームをまたいで保持する)

        // ---- 調整用パラメータ ----
        // 火炎砲弾(gmProjectile)と同じ軌道感になるよう、軌道関連の既定値は揃えてある。
        // TODO:
        // 将来の武器強化要素で、コンストラクタ引数から上書きする想定。
        static constexpr float DEFAULT_ARC_HEIGHT_RATIO = 0.2f;    // 山の高さ ÷ 水平距離 の比率
        static constexpr float DEFAULT_HORIZONTAL_SPEED = 120.0f;  // 水平方向の速さ(world単位/秒)

        // ---- 見た目1: 鉄球(黒い3D球) ----
        static constexpr float BALL_RADIUS = 6.0f;      // 鉄球の見た目の半径(当たり判定よりだいぶ小さくして違和感を減らす)
        static constexpr int   BALL_MESH_DIV = 8;       // 球メッシュの分割数(低ポリで十分)

        // ---- 見た目2: 風を切るビルボードトレイル ----
        static constexpr float       TRAIL_VISUAL_SIZE = 60.0f;         // トレイルのビルボードの大きさ(world単位)
        static constexpr float       COLLIDER_RADIUS = 15.0f;           // 弾の当たり判定の半径
        static constexpr const char* VFX_CLIP_NAME = "tktk_Bless_2";    // 尾の見た目に使うクリップ名(元々シアン〜青系のため、乗算ティントで白っぽい青白さに寄せやすい)

        // 素材の色調(乗算)。tktk_Bless_2は元々シアン〜青系のため、
        // 明るく・少し白寄りに補正しつつ、くっきりしすぎないよう透明度も下げて空気感を出す
        // (オレンジ系の素材だと乗算では青くならないので注意)
        static constexpr uint8_t TRAIL_TINT_R = 235;
        static constexpr uint8_t TRAIL_TINT_G = 245;
        static constexpr uint8_t TRAIL_TINT_B = 255;
        static constexpr uint8_t TRAIL_TINT_A = 170; // 透明度(0〜255)

        // トレイルを進行方向軸まわりに何枚配置するか(3枚=60度刻み、4枚=45度刻み)。
        // 板が真横からほぼ丸見えになる問題(1枚だと視線と進行方向が近づいたときに
        // 薄い板が丸見えになる)を、複数枚を扇状に配置することで軽減する
        static constexpr int TRAIL_MULTI_CROSS_PLANE_COUNT = 3;

        // 板が真横に近づくほどアルファを下げるエッジフェードを使うかどうか。
        // 効果の有無を比較しやすいよう、ここで簡単にON/OFFできるようにしてある
        static constexpr bool TRAIL_EDGE_FADE_ENABLED = true;

        // トレイルの描画位置を、鉄球の位置そのものではなく、進行方向の後方へ
        // ずらした位置にするための量(world単位)。
        // 画像加工でエフェクトの吹き出し原点をコマ中央へ寄せてもなお、鉄球の位置と
        // エフェクトの見た目上の起点にわずかなズレが残ったため、その帳尻合わせに使う
        static constexpr float TRAIL_POSITION_OFFSET = 8.0f;

        // トレイルが鉄球に寸分違わず追従すると動きが硬く見えるため、
        // 「鉄球より少し後ろの目標地点」を毎フレーム指数関数的に追いかける遅延追従にする
        // (弾の進行方向が変化するとその分だけ自然に遅れて揺れる。値が大きいほど追従が速く=遅れが小さくなる)
        static constexpr float TRAIL_FOLLOW_SPEED = 10.0f; // 追従の速さ(1/秒。目安: 5で遅れ大きめ、20でほぼ即追従)

        // 画像素材の「上」方向が、実際には画像の見た目と逆(頭側)を
        // 向いてしまう場合にtrueにすると、進行方向ベクトルを反転して使う
        static constexpr bool REVERSE_TAIL_DIRECTION = false;
    };
}
