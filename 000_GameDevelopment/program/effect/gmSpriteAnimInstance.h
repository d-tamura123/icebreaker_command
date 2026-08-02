// gmSpriteAnimInstance.h
#pragma once
#include <dxe.h>
#include "gmSpriteAnimClip.h"

namespace gm {

    // ビルボード板ポリの配置方式
    enum class gmBillboardMode {
        FaceCamera,             // 板1枚。進行方向軸は固定し、もう片方の軸だけカメラに応じて組み立てる
        CrossCard,              // 板2枚をワールドX/Z軸に交差配置(カメラ追従はしない)。火球等に立体感を出す
        DirectionalMultiCross,  // 進行方向軸を共有した板をN枚、その軸まわりに均等配置する。
                                // (尾を引くエフェクトが真横から見えてしまう問題を軽減する)
    };

    // position_が板ポリのどこに対応するか
    enum class gmSpriteAnchor {
        Bottom, // 板ポリの下端中央がposition_(既定。地面/水面に立つ演出向け)
        Center, // 板ポリの中心がposition_(飛翔体等、宙に浮く演出向け)
    };

    // 素材画像の中で「進行方向」に意味を持つ軸がどちらか
    enum class gmSpriteAxis {
        Horizontal, // 画像の横方向(u軸)が進行方向。例: 頭が左・尾が右の横向き素材(既定)
        Vertical,   // 画像の縦方向(v軸)が進行方向。例: 頭が上・尾が下の縦向き素材
    };

    // ------------------------------------------------------------
    // 1つのエフェクト再生インスタンス。
    // ビルボード板ポリにUVアニメーションを適用して描画する。判定処理には
    // 一切関与しない、見た目だけのクラス。板の配置方式はgmBillboardModeで選べる
    // (常に1枚だけカメラへ向ける方式、ワールド軸固定の2枚交差、進行方向軸まわりの
    //  N枚交差、の3種類)。
    //
    // 再生は「イントロ → (持続系なら)ループ → アウトロ」の3段階(Phase)で管理する。
    // ------------------------------------------------------------
    class gmSpriteAnimInstance {
    public:
        // arg1... 再生するクリップのメタデータ
        // arg2... 対応するテクスチャ
        // arg3... 発生位置(ワールド座標、板ポリの下端中央がここに来る)
        // arg4... ビルボードの一辺の大きさ(world単位、正方形)
        // arg5... true: 持続系再生(loopStart/loopEndの区間を繰り返す。requestStop()まで終了しない)
        //         false: ワンショット再生(frameCount分を1回再生したら自動終了)
        // arg6... 描画方式(既定は1枚板がカメラを向く方式)
        void start(const gmSpriteAnimClip& clip, const Shared<dxe::Texture>& texture,
            const tnl::Vector3& worldPos, float size, bool sustain,
            gmBillboardMode mode = gmBillboardMode::FaceCamera);

        void update(float deltaTime);
        void render(const Shared<dxe::Camera>& camera);

        // 持続系エフェクトを終了させる(ループを抜け、アウトロ区間を1回再生してから終了する)
        // ワンショット再生中に呼んでも何もしない。
        void requestStop();

        // 再生が完全に終わっていればtrue(gmVFXManager側が後片付けの判定に使う)
        bool isFinished() const { return finished_; }

        // 発生源(船など)に追従させたい場合に、毎フレーム位置だけ更新するための関数
        void setPosition(const tnl::Vector3& pos) { position_ = pos; }

        // FaceCamera/DirectionalMultiCrossモードで使う「進行方向」を外部から指定する。
        // 素材画像の向き(頭が左・尾が右など)に意味がある場合、毎フレーム
        // 進行方向ベクトルを渡すことで、その方向へ板の軸(pointingAxis_で選んだ方)が向く。
        // 未設定ならワールドX軸({1,0,0})が使われる。
        void setForwardDirection(const tnl::Vector3& forwardDir) { forwardDir_ = forwardDir; }

        // position_が板ポリのどこに対応するかを指定する(既定はBottom)。
        // 飛翔体等、宙に浮く見た目にしたい場合はCenterを指定する。
        void setAnchor(gmSpriteAnchor anchor) { anchor_ = anchor; }

        // FaceCamera/DirectionalMultiCrossモードで、forwardDir_をどちらの軸に
        // 合わせるかを指定する(既定はHorizontal)。
        // 素材画像が縦向き(頭が上・尾が下等)の場合はVerticalを指定すること。
        void setPointingAxis(gmSpriteAxis axis) { pointingAxis_ = axis; }

        // DirectionalMultiCrossモードで配置する板の枚数を指定する(既定3枚)。
        // 進行方向軸まわりに180度/枚数の間隔で均等配置される
        // (板は表裏両方描画するため、実質「枚数×2方向」から見え方が安定する)。
        void setMultiCrossPlaneCount(int count) { multiCrossPlaneCount_ = (count < 1) ? 1 : count; }

        // 板がカメラに対して真横に近づくほどアルファを下げてフェードさせるかどうか。
        // (FaceCamera/DirectionalMultiCrossの両モードに効く。既定は有効)
        void setEdgeFadeEnabled(bool enabled) { edgeFadeEnabled_ = enabled; }

        // 素材の色調を変えたい場合に使う(乗算ブレンド)。既定は白(255,255,255,255)=無着色。
        // 例: 緑系の素材を青白くしたい場合、setTintColor(200, 220, 255)のように指定する。
        void setTintColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) { tintColor_ = GetColorU8(r, g, b, a); }

    private:
        enum class Phase { Intro, Loop, Outro, Done };

        int getCurrentFrameIndex() const;
        void getFrameUV(int frameIndex, float& u0, float& v0, float& u1, float& v1) const;

        // 1枚分の板ポリを描画する共通処理(FaceCamera/CrossCard/DirectionalMultiCrossどれからも呼ぶ)
        // arg1... 板の「右」方向ベクトル(既に正規化済みのものを渡す)
        // arg2... 板の「上」方向ベクトル(既に正規化済みのものを渡す)
        // arg5... 追加のアルファ倍率(0.0〜1.0)。エッジフェード用途で使う(既定1.0=倍率なし)
        void drawQuad(const tnl::Vector3& rightDir, const tnl::Vector3& upDir, float u0, float v0, float u1, float v1, float alphaScale = 1.0f) const;

        // 進行方向軸とカメラ方向のなす角のsin値がこの値を下回る範囲で、
        // もう片方の軸をなめらかにフォールバック軸へブレンドする(render()内で使用)
        static constexpr float EDGE_BLEND_START = 0.5f; // 0.5 ≒ 30度

        // ---- 再生するクリップとその見た目 ----
        gmSpriteAnimClip     clip_;
        Shared<dxe::Texture> texture_;
        tnl::Vector3         position_{ 0.0f, 0.0f, 0.0f };
        float                size_ = 100.0f;

        // ---- 板ポリの配置・向きに関する設定 ----
        gmBillboardMode mode_ = gmBillboardMode::FaceCamera;
        tnl::Vector3    forwardDir_{ 1.0f, 0.0f, 0.0f };            // FaceCamera/DirectionalMultiCrossモードで使う「進行方向」(既定はワールドX軸)
        gmSpriteAnchor  anchor_ = gmSpriteAnchor::Bottom;
        gmSpriteAxis    pointingAxis_ = gmSpriteAxis::Horizontal;   // forwardDir_をu軸/v軸のどちらに合わせるか
        int             multiCrossPlaneCount_ = 3;                  // DirectionalMultiCrossモードで配置する板の枚数
        bool            edgeFadeEnabled_ = true;                    // 真横に近づくほどアルファを下げるか

        // ---- 再生の進行状況(Phase管理) ----
        bool  sustain_ = false;
        bool  stopping_ = false;
        bool  finished_ = false;
        Phase phase_ = Phase::Intro;
        float phaseTime_ = 0.0f;        // 現在のPhaseに入ってからの経過時間

        COLOR_U8 tintColor_{ 255, 255, 255, 255 }; // 描画時に乗算する色調(既定は無着色)
    };
}
