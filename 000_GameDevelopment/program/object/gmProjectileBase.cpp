// gmProjectileBase.cpp
#include "gmProjectileBase.h"
#include "../collision/gmCollider.h"
#include "../collision/gmCollisionCategory.h"
#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>
#include <cmath>

namespace gm {

    // ------------------------------------------------------------
    // コンストラクタ。
    // 「発射位置から着弾目標位置まで、放物線を描いて指定の飛行時間で必ず着弾する」
    // 軌道になるよう、初速度ベクトル(velocity_)と見かけの重力(gravity_)を逆算する。
    //
    // 処理は大きく4段階に分かれる。
    //   手順1: 発射位置から着弾位置までの「水平距離」を求める
    //   手順2: 水平距離から「飛行時間(滞空時間)」を求める
    //   手順3: 水平距離から「山の高さ(頂点の高さ)」を求める
    //   手順4: 飛行時間と山の高さから、初速度と見かけの重力を逆算する
    // 最後に、当たり判定用のコライダーを設定する。
    // ------------------------------------------------------------
    gmProjectileBase::gmProjectileBase(
        const std::string& id,
        const tnl::Vector3& startPos,
        const tnl::Vector3& targetPos,
        float horizontalSpeed,
        float arcHeightRatio,
        float colliderRadius
    )
        : gmObjectBase(id, startPos)
        , startPos_(startPos)
        , targetPos_(targetPos)
        , horizontalSpeed_(horizontalSpeed)
        , arcHeightRatio_(arcHeightRatio)
    {
        // ------------------------------------------------------------
        // 手順1: 発射位置から着弾位置までの「水平距離」を求める。
        //
        // まず、X軸・Z軸それぞれについて「発射位置から着弾位置までどれだけ離れているか」
        // (horizontalOffsetX, horizontalOffsetZ)を求める。
        // この2つの値は、直角三角形でいう「底辺」と「高さ」にあたる。
        //
        // 三平方の定理(ピタゴラスの定理): 直角三角形の斜辺の長さ = √(底辺² + 高さ²)
        // これをX-Z平面(水平面)に当てはめると、
        //   水平距離 = √(horizontalOffsetX² + horizontalOffsetZ²)
        // という式で、発射位置から着弾位置までの水平方向の直線距離が求まる。
        // ------------------------------------------------------------
        const float horizontalOffsetX = targetPos.x - startPos.x;
        const float horizontalOffsetZ = targetPos.z - startPos.z;
        const float horizontalDist = std::sqrt(horizontalOffsetX * horizontalOffsetX + horizontalOffsetZ * horizontalOffsetZ);

        // ------------------------------------------------------------
        // 手順2: 水平距離から「飛行時間(滞空時間)」を求める。
        //
        // 基準距離(referenceDist = 水平速度 × ARC_TIME_LINEAR_PHASE_SEC)までは、
        //   飛行時間 = 距離 ÷ 水平速度
        // という単純な比例関係で求める(速さが一定なら、距離が2倍になれば時間も2倍になる)。
        //
        // 基準距離より遠い場合は、そのまま比例させ続けると遠距離ほど飛行時間が
        // 伸びすぎてしまう(=待たされる印象が強くなる)ため、√距離に比例する
        // ゆるやかな関数に切り替える。平方根のグラフは、値が大きくなるほど
        // 傾き(伸び方の速さ)が緩やかになるという性質を持つため、これを利用して
        // 「遠距離でも完全に頭打ちにはしないが、伸び方自体は抑える」形にしている。
        //
        // 切り替え地点(基準距離)で不自然な折れ目ができないよう、
        // 「値」だけでなく「傾き(dT/d距離、距離が少し伸びたときに飛行時間がどれだけ
        // 伸びるか)」も一致させてなめらかに接続する。
        //   y = √x の傾き(微分)は dy/dx = 1/(2√x) という数学の公式があるので、
        //   これを使って基準距離地点の傾きが線形部分の傾き(1/水平速度)と
        //   一致するように、伸び方の強さを表す係数(extendCoeff)を逆算している。
        //
        // ※ 物理法則(重力)通りに計算すると、遠距離ほど山なりが過剰に高く・
        //    長い滞空時間になってしまい見た目が不自然だったため、あえてこのような
        //    独自のカーブ(ゲームとして気持ちよく見える形)を設計している。
        // ------------------------------------------------------------
        const float referenceDist = horizontalSpeed_ * ARC_TIME_LINEAR_PHASE_SEC;
        if (horizontalDist <= referenceDist) {
            flightTime_ = std::max(MIN_FLIGHT_TIME, horizontalDist / horizontalSpeed_);
        }
        else {
            const float sqrtRef = std::sqrt(referenceDist);
            // referenceDist地点で傾き(dT/d距離)が線形部分と一致するように逆算した係数
            const float extendCoeff = 2.0f * sqrtRef / horizontalSpeed_;
            flightTime_ = ARC_TIME_LINEAR_PHASE_SEC + extendCoeff * (std::sqrt(horizontalDist) - sqrtRef);
        }

        // ------------------------------------------------------------
        // 手順3・4: 山の高さ(頂点の高さ)を求め、そこから初速度・見かけの重力を逆算する。
        //
        // 手順3: 山の高さH(発射位置基準の頂点の高さ)を、水平距離に比例するだけの
        //         シンプルな式で決める。
        //           H = arcHeightRatio_ × 水平距離
        //         この比率(arcHeightRatio_)がそのまま「山なりの強さ」のつまみになる。
        //
        // 手順4: 「山の高さH」と「手順2で求めた飛行時間T」の2つから、
        //         初速度と見かけの重力を逆算する。
        //           見かけの重力    g_eff = 8H / T²
        //           鉛直方向の初速度 vy0   = (heightOffset + 4H) / T
        //         (heightOffsetは着弾位置と発射位置の高低差。詳しくは下記)
        //
        //         この式は、「発射位置から着弾位置まで高さを直線的に補間した軌道に、
        //         山の高さHぶんの盛り上がりを足しただけの軌道」と数学的に同じ形になる
        //         (時刻tにおける経過割合をs = t/Tとすると、
        //          高さ(t) = 直線補間(heightOffset, s) + 4H・s・(1-s) という式になり、
        //          これを展開して物理の位置の式 y(t) = vy0・t - (1/2)g・t² と
        //          係数を見比べることで、上のg_eff・vy0の式が導ける)。
        //         高低差が無い(heightOffset=0)場合は、物理の対称な放物線の
        //         公式(山の高さ = g・T² / 8)と完全に一致する。
        //
        //         式の中の8.0f・4.0fは、山の高さHと飛行時間Tから放物線を
        //         組み立てるための固定係数であり、山なりの強さそのものを
        //         調整したい場合は、この数値ではなくarcHeightRatio_の方を変えること。
        // ------------------------------------------------------------
        const float heightOffset = targetPos.y - startPos.y;        // 着弾位置と発射位置の高低差(着弾位置の方が高ければ正の値)
        const float arcHeight = arcHeightRatio_ * horizontalDist;   // 山の高さH(発射位置基準の頂点の高さ)

        gravity_ = 8.0f * arcHeight / (flightTime_ * flightTime_);  // 見かけの重力(この一投の間、update()側でもこの値を使い続ける)
        velocity_.x = horizontalOffsetX / flightTime_;
        velocity_.z = horizontalOffsetZ / flightTime_;
        velocity_.y = (heightOffset + 4.0f * arcHeight) / flightTime_;

        // ------------------------------------------------------------
        // 当たり判定(球形コライダー)を設定する。
        // ------------------------------------------------------------
        gmCollider collider;
        collider.type = ColliderShapeType::Sphere;
        collider.radius = colliderRadius;
        addCollider(collider);
        setCollisionCategory(gmCollisionCategory::Projectile);
    }

    // ------------------------------------------------------------
    // 毎フレームの更新。経過時間から弾の現在位置・進行方向を計算し、
    // 見た目の更新(派生クラスのupdateVisual)へ橋渡しする。
    // 着弾したら位置を目標地点に確定させて消滅させる。
    // ------------------------------------------------------------
    void gmProjectileBase::update(float deltaTime)
    {
        // 見た目のスローモーション倍率を、飛行・アニメーション再生の両方に適用する
        // (速度や重力そのものを変えると軌道の"形"が変わってしまうため、時間の進み方を遅くする)
        const float scaledDeltaTime = deltaTime * DEBUG_TIME_SCALE;

        elapsedTime_ += scaledDeltaTime;

        if (elapsedTime_ >= flightTime_) {
            // 着弾。位置を目標地点に確定させて消滅する。
            // 実際の命中判定(氷山へのダメージ・分裂等)は、コライダーによる衝突検出
            // (onCollisionEnter、派生クラス側で実装)に任せる想定のため、ここでは行わない。
            position_ = targetPos_;
            kill();
            return;
        }

        // ------------------------------------------------------------
        // 現在位置を、経過時間tから閉じた式(=積み上げ計算をしない直接計算)で求める。
        // 速度を毎フレーム積分して足し込む方式だと、フレームレートによって
        // 誤差が蓄積してしまうため、常に「発射時刻からの経過時間」を基準に
        // 位置を計算し直すことで、フレームレートに依存しない正確な軌道にしている。
        //
        // 以下は物理の「等加速度運動」の位置の式そのもの。
        //   水平方向(X, Z): 重力の影響を受けないので、速さ一定の等速直線運動
        //       x(t) = x0 + vx・t
        //   鉛直方向(Y)  : 重力によって加速度gで落とされ続ける等加速度運動
        //       y(t) = y0 + vy0・t − (1/2)・g・t²
        // ------------------------------------------------------------
        const float t = elapsedTime_;
        position_.x = startPos_.x + velocity_.x * t;
        position_.z = startPos_.z + velocity_.z * t;
        position_.y = startPos_.y + velocity_.y * t - 0.5f * gravity_ * t * t;

        // ------------------------------------------------------------
        // 現在(時刻t)の瞬間速度ベクトルを求める。
        // 鉛直方向の速度は、重力によって時間とともに変化する:
        //   vy(t) = vy0 − g・t
        // (最初は上向きの速度vy0を持つが、重力によって毎秒gずつ下向きに削られていく)
        // これにより、放物線の頂点付近では速度がほぼ水平に、落下中は下向きに、
        // と見た目の進行方向が自然に追従する。
        // ------------------------------------------------------------
        const tnl::Vector3 currentVelocity{
            velocity_.x,
            velocity_.y - gravity_ * t,
            velocity_.z
        };
        const tnl::Vector3 forwardDir = tnl::Vector3::Normalize(currentVelocity);

        updateVisual(position_, forwardDir, scaledDeltaTime);
    }

    // ------------------------------------------------------------
    // 描画。見た目の描画は派生クラス(renderVisual)へ委譲するだけ。
    // ------------------------------------------------------------
    void gmProjectileBase::render(const Shared<dxe::Camera>& camera)
    {
        renderVisual(camera);
    }
}
