// gmSplitProjectile.cpp
#include "gmSplitProjectile.h"
#include "../effect/gmSpriteAnimRegistry.h"
#include "../collision/gmCollisionCategory.h"
#include "../gmGameConfig.h"
#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>
#include <cmath>

namespace gm {

    // ------------------------------------------------------------
    // コンストラクタ。軌道計算は基底クラス(gmProjectileBase)に任せ、
    // ここでは見た目(風トレイルのビルボード + 黒い鉄球のメッシュ)だけを構築する。
    // ------------------------------------------------------------
    gmSplitProjectile::gmSplitProjectile(
        const std::string& id,
        const tnl::Vector3& startPos,
        const tnl::Vector3& targetPos,
        const std::shared_ptr<gmSpriteAnimRegistry>& spriteRegistry,
        float horizontalSpeed,
        float arcHeightRatio
    )
        : gmProjectileBase(id, startPos, targetPos, horizontalSpeed, arcHeightRatio, COLLIDER_RADIUS)
    {
        // ---- 見た目1: 風を切るビルボードトレイル ----
        // (仕組みはgmProjectileの火炎トレイルと同じ。クリップと色調だけが異なる)
        if (spriteRegistry) {
            const gmSpriteAnimClip* clip = spriteRegistry->getClip(VFX_CLIP_NAME);
            if (clip) {
                Shared<dxe::Texture> tex = spriteRegistry->getTexture(VFX_CLIP_NAME, VFX_EFFECT_GRAPHICS_DIR);

                // gmProjectileと同じ理由(飛行時間に応じてfpsを都度計算し直し、
                // 着弾前にアニメーションだけ先に終わらないようにする)
                gmSpriteAnimClip vfxClip = *clip;
                const int frameCount = std::max(1, vfxClip.frameCount);
                vfxClip.fps = static_cast<float>(frameCount) / flightTime_;

                // 進行方向の後方へオフセットした位置から再生を開始する(updateVisual()と同じ考え方)
                const tnl::Vector3 initialForwardDir = tnl::Vector3::Normalize(velocity_);
                const tnl::Vector3 initialTrailPos = position_ - initialForwardDir * TRAIL_POSITION_OFFSET;
                trailFollowPos_ = initialTrailPos; // 遅延追従の初期値(いきなり離れた位置から追いかけ始めるとワープして見えるため)

                trailVisual_.start(vfxClip, tex, initialTrailPos, TRAIL_VISUAL_SIZE, /*sustain=*/false, gmBillboardMode::DirectionalMultiCross);
                trailVisual_.setAnchor(gmSpriteAnchor::Center);
                trailVisual_.setMultiCrossPlaneCount(TRAIL_MULTI_CROSS_PLANE_COUNT);
                trailVisual_.setEdgeFadeEnabled(TRAIL_EDGE_FADE_ENABLED);

                // tktk_Bless_2は画像加工で「頭が上・尾が下」の縦向きに補正済みのため、
                // forwardDir_をu軸(横)ではなくv軸(縦)に合わせる
                trailVisual_.setPointingAxis(gmSpriteAxis::Vertical);

                // 縦向き画像は「頭(画像の上=v0)」がそのまま進行方向を向けばよいため、
                // 横向き素材(Fire_10)のような符号反転は不要
                tnl::Vector3 initialTrailFacingDir = initialForwardDir;
                if (REVERSE_TAIL_DIRECTION) {
                    initialTrailFacingDir = -initialTrailFacingDir;
                }
                trailVisual_.setForwardDirection(initialTrailFacingDir);

                // tktk_Bless_2は元がシアン〜青系のため、白っぽく明るめに補正しつつ、
                // くっきりしすぎないよう透明度も下げて空気感を出す
                trailVisual_.setTintColor(TRAIL_TINT_R, TRAIL_TINT_G, TRAIL_TINT_B, TRAIL_TINT_A);
            }
        }

        // ---- 見た目2: 黒い鉄球(3D球メッシュ) ----
        // テクスチャは全インスタンス共通のため、初回だけ読み込んで使い回す
        static Shared<dxe::Texture> s_ballTexture = dxe::Texture::CreateFromFile(GRAPHICS_FILE_PATH__SPLIT_PROJECTILE_BALL);

        ballMesh_ = dxe::Mesh::CreateSphereMV(BALL_RADIUS, BALL_MESH_DIV, BALL_MESH_DIV);
        if (ballMesh_ && s_ballTexture) {
            ballMesh_->setTexture(s_ballTexture);
        }
    }

    // ------------------------------------------------------------
    // 毎フレームの見た目更新。
    //   手順1: 風トレイルの「向き」を進行方向に合わせる
    //   手順2: 風トレイルの「表示位置」を、鉄球より少し後方の目標地点へ
    //          遅延追従(バネのように少し遅れて追いかける)させる
    //   手順3: 鉄球の位置を更新する
    // ------------------------------------------------------------
    void gmSplitProjectile::updateVisual(const tnl::Vector3& position, const tnl::Vector3& forwardDir, float scaledDeltaTime)
    {
        // ---- 手順1: 風トレイルの向き ----
        // tktk_Bless_2は「頭が上・尾が下」の縦向き画像(setPointingAxis(Vertical)済み)。
        // 頭(画像の上)がそのまま進行方向を向けばよいため、符号反転は不要
        // (Fire_10のような横向き素材は逆に反転が必要。REVERSE_TAIL_DIRECTIONで個別に調整可能)
        tnl::Vector3 trailFacingDir = forwardDir;
        if (REVERSE_TAIL_DIRECTION) {
            trailFacingDir = -trailFacingDir;
        }

        // ---- 手順2: 風トレイルの表示位置(遅延追従) ----
        // 画像加工でエフェクトの吹き出し原点をコマ中央へ寄せてもなお、
        // 鉄球の位置とエフェクトの見た目上の起点にズレが残るため、
        // 進行方向の後方(鉄球より少し後ろ)へ目標地点をずらして帳尻を合わせる
        const tnl::Vector3 targetTrailPos = position - forwardDir * TRAIL_POSITION_OFFSET;

        // トレイルが鉄球の真後ろにピタッと追従したままだと硬い印象になるため、
        // 目標地点(targetTrailPos)を毎フレーム指数関数的に追いかける遅延追従にする。
        // (サイン波などで独立に揺らすのではなく、弾の実際の進行方向の変化に
        //  ちゃんと反応して遅れる形にすることで、機械的なループ感を避ける狙い)
        //
        // 「指数関数的に追いかける」とは、1フレームで目標地点までの残り距離の
        // 何割かだけ近づく、という動きを毎フレーム繰り返すこと。
        //   1フレームで近づく割合(followLerp) = 1 − e^(−追従の速さ × 経過時間)
        // という式で、TRAIL_FOLLOW_SPEEDが大きいほど1フレームで近づく割合が
        // 大きくなり(=素早く追いつく)、小さいほど遅れが大きく残る。
        const float followLerp = 1.0f - std::exp(-TRAIL_FOLLOW_SPEED * scaledDeltaTime);
        trailFollowPos_ = trailFollowPos_ + (targetTrailPos - trailFollowPos_) * followLerp;

        trailVisual_.setPosition(trailFollowPos_);
        trailVisual_.setForwardDirection(trailFacingDir);
        trailVisual_.update(scaledDeltaTime);

        // ---- 手順3: 鉄球の位置 ----
        // 黒一色の球体には向きの情報自体に意味が無いため、位置更新のみ行う
        // (以前は進行方向を軸にした自転演出を入れていたが、低ポリ球を高速回転させると
        //  面のハイライトがちらついて不自然に見えるため削除した)
        if (ballMesh_) {
            ballMesh_->setPosition(position);
        }
    }

    // ------------------------------------------------------------
    // 描画。風トレイルのビルボードと鉄球メッシュを、それぞれ描画するだけ。
    // ------------------------------------------------------------
    void gmSplitProjectile::renderVisual(const Shared<dxe::Camera>& camera)
    {
        trailVisual_.render(camera);
        if (ballMesh_) {
            ballMesh_->render(camera);
        }
    }

    // ------------------------------------------------------------
    // 衝突時の処理。流氷に当たったら消滅する(分裂処理は未実装)。
    // ------------------------------------------------------------
    void gmSplitProjectile::onCollisionEnter(gmObjectBase* other)
    {
        if (!other) return;

        if (other->getCollisionCategory() == gmCollisionCategory::Iceberg) {
            // TODO: 流氷の分裂処理(次のステップで実装)
            kill();
        }
    }
}
