// gmProjectileBase.h
#pragma once
#include "gmObjectBase.h"
#include <memory>

namespace gm {

    // ------------------------------------------------------------
    // 「クリックした地点まで放物線を描いて飛ぶ」弾道兵器の共通基底クラス。
    //
    // ここに集約するのはあくまで軌道計算(飛行時間・山なりの高さ・重力/初速の
    // 逆算・毎フレームの位置更新)とタイミング管理だけで、見た目の構築・更新・描画、
    // および着弾時の効果(溶かす/割る等)は派生クラス側の責務とする。
    //
    // 見た目や着弾効果が全く異なる火炎放射(船から扇状の範囲に継続して炎を出す、
    // 発射地点→着弾地点という概念自体が無い攻撃)は、この軌道計算ロジックを
    // 一切使わないため、あえてこのクラスは継承しない。
    // ------------------------------------------------------------
    class gmProjectileBase : public gmObjectBase {
    public:
        void update(float deltaTime) override final;
        void render(const Shared<dxe::Camera>& camera) override final;

        // 現在の速度ベクトル(初速のまま、以後不変)。
        // 用途: 命中した相手側が「どちらから撃たれたか」を知りたい場合(氷山の分裂方向など)
        const tnl::Vector3& getVelocity() const { return velocity_; }

    protected:
        // arg1... 識別ID
        // arg2... 発射位置(ワールド座標)
        // arg3... 着弾目標位置(ワールド座標)
        // arg4... 水平方向の速さ(world単位/秒)。近距離〜基準距離まではこれで飛行時間が比例して伸びる
        // arg5... 山の高さ ÷ 水平距離 の比率。大きいほど山なりが大きくなる
        // arg6... 当たり判定(球)の半径
        gmProjectileBase(
            const std::string& id,
            const tnl::Vector3& startPos,
            const tnl::Vector3& targetPos,
            float horizontalSpeed,
            float arcHeightRatio,
            float colliderRadius
        );

        // 派生クラスが、着弾するまでの間(毎フレーム)自分の見た目を更新するためのフック。
        // arg1... 現在位置
        // arg2... 現在の瞬間速度方向(正規化済み、実際の進行方向そのまま)。
        //         素材の向き(頭が左/右等)に合わせた符号反転は派生クラス側で行うこと。
        // arg3... スローモーション倍率適用後のdeltaTime(見た目のアニメーション側にもそのまま渡すとよい)
        virtual void updateVisual(const tnl::Vector3& position, const tnl::Vector3& forwardDir, float scaledDeltaTime) = 0;

        // 派生クラスが自分の見た目を描画するためのフック
        virtual void renderVisual(const Shared<dxe::Camera>& camera) = 0;

        // ---- 発射条件(コンストラクタで確定し、以後は変化しない) ----
        tnl::Vector3 startPos_;   // 発射位置
        tnl::Vector3 targetPos_;  // 着弾目標位置
        tnl::Vector3 velocity_;   // 初速度ベクトル。放物線の式から逆算した固定値で、以後は変化しない

        // ---- 飛行タイミング ----
        float elapsedTime_ = 0.0f;  // 発射からの経過時間(秒)。update()が毎フレーム加算していく
        float flightTime_ = 0.0f;   // 着弾までの飛行時間(秒)。コンストラクタで一度だけ決定する

        // ---- 軌道の形を決めるパラメータ ----
        float horizontalSpeed_;    // 水平方向の速さ(world単位/秒)
        float arcHeightRatio_;     // 山の高さ ÷ 水平距離 の比率
        float gravity_ = 0.0f;     // "見かけの重力"。コンストラクタで山の高さ・飛行時間から逆算する(update()の落下計算で使う実行時専用の値であり、物理的に正しい重力加速度そのものではない)

        // ---- 軌道計算の調整用パラメータ(弾道兵器全般で共通の考え方) ----
        static constexpr float MIN_FLIGHT_TIME              = 0.3f;  // 極端に近い的でも最低これだけの飛行時間を確保する(秒)
        static constexpr float ARC_TIME_LINEAR_PHASE_SEC    = 1.2f;  // この秒数ぶんの距離までは「距離÷水平速度」の比例。それより遠いとゆるやかな伸びに切り替わる

        // 見た目確認用のスローモーション倍率(共通)。
        // 1.0で通常速度、小さいほど飛行・アニメーションとも全体がゆっくりになる。
        static constexpr float DEBUG_TIME_SCALE = 1.0f;
    };
}
