// gmSpriteAnimInstance.cpp
#include "gmSpriteAnimInstance.h"
#include "../util/gmRenderUtil.h"
#include <DxLib.h>
#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>
#include <cmath>

namespace gm {

    // ------------------------------------------------------------
    // 再生を開始する。渡されたクリップ・テクスチャ・配置設定を覚えておき、
    // 再生状態(Phase)をイントロの先頭にリセットするだけ。
    // ------------------------------------------------------------
    void gmSpriteAnimInstance::start(const gmSpriteAnimClip& clip, const Shared<dxe::Texture>& texture,
        const tnl::Vector3& worldPos, float size, bool sustain, gmBillboardMode mode)
    {
        clip_ = clip;
        texture_ = texture;
        position_ = worldPos;
        size_ = size;
        mode_ = mode;

        sustain_ = sustain;
        stopping_ = false;
        finished_ = false;
        phase_ = Phase::Intro;
        phaseTime_ = 0.0f;
    }

    // ------------------------------------------------------------
    // 持続系エフェクトを終了させる。ループ区間を抜けてアウトロへ移るための
    // フラグを立てるだけで、実際の切り替えはupdate()側で行う。
    // ------------------------------------------------------------
    void gmSpriteAnimInstance::requestStop()
    {
        if (sustain_ && phase_ != Phase::Outro && phase_ != Phase::Done) {
            stopping_ = true;
        }
    }

    // ------------------------------------------------------------
    // 再生状態(Phase)の更新。
    // フレーム番号の計算そのものは getCurrentFrameIndex() 側で行い、
    // ここでは「今どのPhaseにいるべきか」の切り替えだけを判定する。
    // ------------------------------------------------------------
    void gmSpriteAnimInstance::update(float deltaTime)
    {
        if (finished_) {
            return;
        }

        phaseTime_ += deltaTime;
        const float frameDuration = 1.0f / std::max(clip_.fps, 0.01f); // 1コマあたりの表示時間(秒) = 1 ÷ fps

        // ループ区間が有効かどうか(loopStart/loopEndが両方とも妥当な値かどうか)
        const bool hasValidLoop = (clip_.loopStart >= 0)
            && (clip_.loopEnd > clip_.loopStart)
            && (clip_.loopEnd <= clip_.frameCount);

        switch (phase_) {
        case Phase::Intro: {
            // 持続系かつループ設定が有効なら「loopStartまで」がイントロ、
            // それ以外(ワンショット、またはループ未設定)は「frameCount全部」がイントロ扱い。
            const int introEndFrame = (sustain_ && hasValidLoop) ? clip_.loopStart : clip_.frameCount;
            const float introDuration = introEndFrame * frameDuration;

            if (phaseTime_ >= introDuration) {
                phaseTime_ = 0.0f;
                if (sustain_ && hasValidLoop) {
                    phase_ = Phase::Loop;
                }
                else {
                    // ワンショット、またはループ設定が無い持続系はここで終了する
                    phase_ = Phase::Done;
                    finished_ = true;
                }
            }
            break;
        }

        case Phase::Loop: {
            if (stopping_) {
                phase_ = Phase::Outro;
                phaseTime_ = 0.0f;
            }
            // ループ中はフレーム計算側(getCurrentFrameIndex)で
            // ループ区間の長さを使って時間を折り返すので、ここでは何もしない。
            break;
        }

        case Phase::Outro: {
            const int outroFrames = std::max(0, clip_.frameCount - clip_.loopEnd);
            const float outroDuration = outroFrames * frameDuration;
            if (phaseTime_ >= outroDuration) {
                phase_ = Phase::Done;
                finished_ = true;
            }
            break;
        }

        case Phase::Done:
            finished_ = true;
            break;
        }
    }

    // ------------------------------------------------------------
    // 現在のPhase・経過時間から、実際に表示すべきコマ番号を求める
    // ------------------------------------------------------------
    int gmSpriteAnimInstance::getCurrentFrameIndex() const
    {
        const float frameDuration = 1.0f / std::max(clip_.fps, 0.01f); // 1コマあたりの表示時間(秒)

        switch (phase_) {
        case Phase::Intro: {
            // 経過時間 ÷ 1コマの表示時間 = 何コマ分の時間が過ぎたか
            const int frameIndex = static_cast<int>(phaseTime_ / frameDuration);
            return std::clamp(frameIndex, 0, clip_.frameCount - 1);
        }

        case Phase::Loop: {
            const int loopFrameCount = std::max(1, clip_.loopEnd - clip_.loopStart);
            const float loopDuration = loopFrameCount * frameDuration;
            // ループ区間内での経過時間を、ループの長さで割った「あまり」に変換する
            // (fmod = 割り算の余りを求める関数。これにより時間が何周しても
            //  ループ区間の中に折り返され続ける)
            const float timeWithinLoop = std::fmod(phaseTime_, loopDuration);
            const int frameIndex = clip_.loopStart + static_cast<int>(timeWithinLoop / frameDuration);
            return std::clamp(frameIndex, 0, clip_.frameCount - 1);
        }

        case Phase::Outro: {
            const int frameIndex = clip_.loopEnd + static_cast<int>(phaseTime_ / frameDuration);
            return std::clamp(frameIndex, 0, clip_.frameCount - 1);
        }

        default:
            return std::max(0, clip_.frameCount - 1);
        }
    }

    // ------------------------------------------------------------
    // コマ番号から、テクスチャ上のUV矩形(0.0〜1.0)を求める
    // ------------------------------------------------------------
    void gmSpriteAnimInstance::getFrameUV(int frameIndex, float& u0, float& v0, float& u1, float& v1) const
    {
        const int col = frameIndex % clip_.cols;
        const int row = frameIndex / clip_.cols;

        const float cellUvWidth = 1.0f / static_cast<float>(clip_.cols);
        const float cellUvHeight = 1.0f / static_cast<float>(clip_.rows);

        u0 = col * cellUvWidth;
        v0 = row * cellUvHeight;
        u1 = u0 + cellUvWidth;
        v1 = v0 + cellUvHeight;
    }

    // ------------------------------------------------------------
    // 板ポリ1枚分の描画。position_を基準(anchor_の設定に応じて下端中央 or 中心)として、
    // rightDir方向にrightExtent分、upDir方向にupExtent分だけ広がる矩形を描く
    // (rightExtent == upExtentなら従来通りの正方形。setForwardLength()で
    //  進行方向側だけ値を変えると細長い板ポリになる)。
    // ------------------------------------------------------------
    void gmSpriteAnimInstance::drawQuad(const tnl::Vector3& rightDir, const tnl::Vector3& upDir, float rightExtent, float upExtent, float u0, float v0, float u1, float v1, float alphaScale) const
    {
        const float halfSize = rightExtent * 0.5f;

        // anchor_ に応じて、position_からの上端・下端オフセットを切り替える
        //   Bottom: 下端(0)〜上端(upExtent)         … position_は板ポリの下端
        //   Center: 下端(-upExtent/2)〜上端(upExtent/2) … position_は板ポリの中心
        const float halfUp = upExtent * 0.5f;
        const float topOffset = (anchor_ == gmSpriteAnchor::Center) ? halfUp : upExtent;
        const float bottomOffset = (anchor_ == gmSpriteAnchor::Center) ? -halfUp : 0.0f;

        // 頂点色(tintColor_)に、追加のアルファ倍率(エッジフェード用)を掛け合わせる
        COLOR_U8 vertexColor = tintColor_;
        vertexColor.a = static_cast<uint8_t>(std::clamp(vertexColor.a * alphaScale, 0.0f, 255.0f));

        VERTEX3D vtx[4]{};
        auto setVtx = [&](int i, const tnl::Vector3& offset, float u, float v) {
            tnl::Vector3 p = position_ + offset;
            vtx[i].pos = VGet(p.x, p.y, p.z);
            vtx[i].dif = vertexColor;
            vtx[i].spc = GetColorU8(0, 0, 0, 0);
            vtx[i].norm = VGet(0.0f, 1.0f, 0.0f);
            vtx[i].u = u;
            vtx[i].v = v;
            };

        setVtx(0, -rightDir * halfSize + upDir * topOffset, u0, v0);    // 左上
        setVtx(1, rightDir * halfSize + upDir * topOffset, u1, v0);     // 右上
        setVtx(2, -rightDir * halfSize + upDir * bottomOffset, u0, v1); // 左下
        setVtx(3, rightDir * halfSize + upDir * bottomOffset, u1, v1);  // 右下

        WORD idx[6] = { 0, 1, 2, 1, 3, 2 };

        DrawPolygonIndexed3D(vtx, 4, idx, 2, texture_->getDxLibGraphHandle(), TRUE);
    }

    // ------------------------------------------------------------
    // ビルボード板ポリの描画。gmBillboardMode(mode_)に応じて、
    // 以下のいずれかの方式で1〜N枚の板を描画する。
    //   CrossCard             : ワールドX方向・Z方向を向いた2枚を90度交差させて配置する。
    //                           カメラには追従しないが、どの角度から見ても常にどちらかの面が
    //                           ある程度見えるため、1枚板より立体感が出る(火球等に向け)。
    //   FaceCamera            : 進行方向軸を固定して、板1枚をカメラに応じて組み立てる。
    //   DirectionalMultiCross : FaceCameraと同じ考え方で、板をN枚、進行方向軸まわりに
    //                           扇状に配置する(1枚だと真横から見えてしまう問題を軽減する)。
    // ------------------------------------------------------------
    void gmSpriteAnimInstance::render(const Shared<dxe::Camera>& camera)
    {
        if (finished_ || !texture_) {
            return;
        }

        const int frameIndex = getCurrentFrameIndex();
        float u0, v0, u1, v1;
        getFrameUV(frameIndex, u0, v0, u1, v1);

        gm::ApplyCamera3D(camera);

        SetUseLighting(FALSE);
        SetWriteZBuffer3D(FALSE);                   // 半透明合成のため、Zバッファへの書き込みはしない
        SetUseZBuffer3D(TRUE);                      // 奥行きの前後関係は考慮する(他オブジェクトに隠れてよい)
        SetUseBackCulling(FALSE);
        SetDrawBlendMode(DX_BLENDMODE_ADD, 255);    // 炎・光系のエフェクトは加算合成が映える

        // gmBillboardMode::CrossCard の場合
        if (mode_ == gmBillboardMode::CrossCard) {
            drawQuad(tnl::Vector3(1.0f, 0.0f, 0.0f), tnl::Vector3(0.0f, 1.0f, 0.0f), size_, size_, u0, v0, u1, v1); // ワールドX方向を向いた面
            drawQuad(tnl::Vector3(0.0f, 0.0f, 1.0f), tnl::Vector3(0.0f, 1.0f, 0.0f), size_, size_, u0, v0, u1, v1); // ワールドZ方向を向いた面
        }
        // gmBillboardMode::FaceCamera / DirectionalMultiCross の場合
        else {
            // ------------------------------------------------------------
            // 進行方向軸ビルボード:
            // 板の2つの辺ベクトル(quadRight, quadUp)のうち、進行方向を表す方の軸は
            // カメラの向きに関わらず常にforwardDir_そのものを使う。もう片方の軸だけ、
            // その軸とカメラ方向(toCamera)から直交計算して求める
            // (=完全にはカメラへ正対しなくなるが、進行方向側の軸は常に安定する)。
            //
            // ※ 以前は「進行方向をカメラ正対平面へ投影する」方式だったが、
            // 進行方向がカメラの視線に近い角度になるほど投影後のベクトルが短くなり
            // 不安定になる(=前方や遠距離で向きがおかしくなる)問題があったため、この方式に変更した。
            //
            // 手順は次の4段階:
            //   手順1: 進行方向軸(dirAxis)を確定する
            //   手順2: 直交計算が破綻したときの逃げ道(camRightFallback)を用意する
            //   手順3: 「もう片方の軸」をカメラ方向から求める関数を定義する
            //   手順4: 板を(DirectionalMultiCrossなら複数枚)配置して描画する
            // ------------------------------------------------------------

            const tnl::Vector3 toCamera = tnl::Vector3::Normalize(camera->getPosition() - position_);

            // ---- 手順1: 進行方向軸(未指定/ゼロベクトルならワールドX軸へフォールバック) ----
            tnl::Vector3 dirAxis = forwardDir_;
            if (dirAxis.length() < 1e-4f) {
                dirAxis = tnl::Vector3(1.0f, 0.0f, 0.0f);
            }
            else {
                dirAxis = tnl::Vector3::Normalize(dirAxis);
            }

            // ---- 手順2: もう片方の軸がdirAxisとtoCameraの平行で潰れてしまった場合のフォールバック用 ----
            tnl::Vector3 camRightFallback = tnl::Vector3::Cross(tnl::Vector3(0.0f, 1.0f, 0.0f), toCamera);
            if (camRightFallback.length() < 1e-4f) {
                camRightFallback = tnl::Vector3(1.0f, 0.0f, 0.0f);
            }
            else {
                camRightFallback = tnl::Vector3::Normalize(camRightFallback);
            }

            // ---- 手順3: 「もう片方の軸」をカメラ向きから計算する関数 ----
            // pointingAxis_に応じて、dirAxisをu軸(横)/v軸(縦)のどちらに固定するかを切り替え、
            // 「もう片方の軸」をカメラ向きから計算する(=1枚目の板の向き)。
            //
            // ここで使う外積(Cross)は、「2つのベクトルの両方に対して垂直な、
            // 新しいベクトルを作る計算」で、その長さは2つのベクトルのなす角θのsinθに
            // 比例する(2つが平行に近いほど0に近づき、直交するほど1に近づく)。
            // つまりcrossLenは「dirAxisとcamDirがどれだけしっかり交差しているか」の
            // 目安になっており、これが小さい(≒平行に近い)ほど計算が不安定になる。
            //
            // dirAxisとcamDirがほぼ平行(=カメラが進行方向の真正面/真後ろ)に近づくほど、
            // camRightFallbackへなめらかにブレンドする(閾値でパキッと切り替えると、
            // 放物線の頂点付近(進行方向がカメラ視線に近づきやすい)でカクついて見えるため)。
            auto computeOtherAxis = [&](const tnl::Vector3& camDir) -> tnl::Vector3 {
                tnl::Vector3 crossRaw;
                if (pointingAxis_ == gmSpriteAxis::Vertical) {
                    crossRaw = tnl::Vector3::Cross(dirAxis, camDir);
                }
                else {
                    crossRaw = tnl::Vector3::Cross(camDir, dirAxis);
                }

                const float crossLen = crossRaw.length(); // dirAxisとcamDirのなす角のsin。0=平行(危険)、1=直交(安全)
                if (crossLen < 1e-4f) {
                    // 完全に平行で計算不能な場合のみフォールバックそのものを返す
                    return camRightFallback;
                }

                const tnl::Vector3 normalizedAxis = crossRaw * (1.0f / crossLen);

                // crossLenがEDGE_BLEND_START未満の範囲で、フォールバック側へなめらかに寄せる。
                // blendFactor(0〜1)は「本来の軸をどれだけ信用するか」の割合で、
                //   0 = 完全にフォールバックを使う、1 = 完全に本来の軸(normalizedAxis)を使う
                // smoothstep(3t²−2t³という式)を使うことで、0や1の付近で
                // 変化が滑らかに止まり、切り替わりの継ぎ目が目立たなくなる。
                const float blendRatio = std::clamp(crossLen / EDGE_BLEND_START, 0.0f, 1.0f);
                const float blendFactor = blendRatio * blendRatio * (3.0f - 2.0f * blendRatio);
                const tnl::Vector3 blendedAxis = camRightFallback * (1.0f - blendFactor) + normalizedAxis * blendFactor;

                const float blendedAxisLen = blendedAxis.length();
                return (blendedAxisLen > 1e-4f) ? blendedAxis * (1.0f / blendedAxisLen) : camRightFallback;
                };

            const tnl::Vector3 baseOtherAxis = computeOtherAxis(toCamera);

            // ---- 手順4: 板を配置して描画する ----
            // DirectionalMultiCrossなら複数枚、FaceCameraなら1枚だけ描画する。
            // 板はdirAxis(進行方向軸)を共有したまま、もう片方の軸をdirAxisまわりに
            // 180度/枚数の間隔で回転させて配置する(表裏両方描画するため、
            // 実質「枚数×2方向」から見え方が安定する)。
            const int planeCount = (mode_ == gmBillboardMode::DirectionalMultiCross) ? multiCrossPlaneCount_ : 1;
            const float angleStepDeg = 180.0f / static_cast<float>(planeCount);

            for (int planeIndex = 0; planeIndex < planeCount; ++planeIndex) {
                tnl::Vector3 otherAxis = baseOtherAxis;
                if (planeIndex > 0) {
                    const float angleDeg = angleStepDeg * static_cast<float>(planeIndex);
                    const tnl::Quaternion rot = tnl::Quaternion::RotationAxis(dirAxis, tnl::ToRadian(angleDeg));
                    otherAxis = tnl::Vector3::Normalize(tnl::Vector3::TransformCoord(baseOtherAxis, rot));
                }

                tnl::Vector3 quadRight, quadUp;
                float rightExtent, upExtent;

                // forwardLength_が設定されていれば、進行方向軸(dirAxis)側だけその長さを使う
                // (もう片方=太さ側は常にsize_のまま)。未設定(-1)ならこれまで通り正方形。
                const float forwardExtent = (forwardLength_ > 0.0f) ? forwardLength_ : size_;
                const float crossExtent = size_;

                // 進行方向軸に対応するUV(u軸 or v軸)を、setUInset()の指定ぶんだけ内側に詰める
                // (素材の透明な余白を除いて、不透明な部分だけを引き伸ばすため)
                float du0 = u0, du1 = u1, dv0 = v0, dv1 = v1;
                const float insetStart = std::clamp(uInsetStart_, 0.0f, 0.49f);
                const float insetEnd = std::clamp(uInsetEnd_, 0.0f, 0.49f);

                if (pointingAxis_ == gmSpriteAxis::Vertical) {
                    quadUp = dirAxis;
                    quadRight = otherAxis;
                    upExtent = forwardExtent;
                    rightExtent = crossExtent;

                    const float vRange = v1 - v0;
                    dv0 = v0 + vRange * insetStart;
                    dv1 = v1 - vRange * insetEnd;
                }
                else {
                    quadRight = dirAxis;
                    quadUp = otherAxis;
                    rightExtent = forwardExtent;
                    upExtent = crossExtent;

                    const float uRange = u1 - u0;
                    du0 = u0 + uRange * insetStart;
                    du1 = u1 - uRange * insetEnd;
                }

                // 板がカメラに対して真横に近いほどアルファを下げる。
                // quadRight・quadUpは互いに直交する単位ベクトルなので、その外積は
                // そのまま板の法線(面が向いている方向)になる。この法線とtoCameraの
                // 内積は「板がどれだけ正面から見えているか」を−1〜1で表し、
                // 0(=法線とtoCameraが直交=板が真横を向いている)に近いほど
                // 薄く見えるべきなので、その絶対値をそのままアルファ倍率として使う。
                const float faceToCameraFactor = edgeFadeEnabled_
                    ? std::fabs(tnl::Vector3::Dot(tnl::Vector3::Cross(quadRight, quadUp), toCamera))
                    : 1.0f;

                drawQuad(quadRight, quadUp, rightExtent, upExtent, du0, dv0, du1, dv1, faceToCameraFactor);
            }
        }

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        SetUseBackCulling(TRUE);
        SetUseZBuffer3D(TRUE);
        SetWriteZBuffer3D(TRUE);
        SetUseLighting(TRUE);
    }
}
