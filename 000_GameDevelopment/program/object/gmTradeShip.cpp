// gmTradeShip.cpp
#include "gmTradeShip.h"
#include "../gmGameConfig.h"
#include "../collision/gmCollisionCategory.h"
#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>
#include <cmath>

namespace gm {

    namespace {
        // gmShip::getForward()の定義(forward = {sin(yaw), 0, cos(yaw)})に対応する、
        // 方向ベクトル(XZ平面)からyawを逆算するヘルパー
        float DirectionToYaw(const tnl::Vector3& dir)
        {
            return atan2f(dir.x, dir.z);
        }
    }

    // ------------------------------------------------------------
    // arg2(centerline)を保持し、累積距離テーブルを構築する。
    // 初期位置はcenterline.front()(=S)、初期向きは2点目を向く。
    // ------------------------------------------------------------
    gmTradeShip::gmTradeShip(const std::string& id, const std::vector<tnl::Vector3>& centerline)
        : gmShip(id, centerline.empty() ? tnl::Vector3(0.0f, 0.0f, 0.0f) : centerline.front())
        , centerline_(centerline)
    {
        cumulativeDistance_.resize(centerline_.size());
        if (!centerline_.empty()) {
            cumulativeDistance_[0] = 0.0f;
            for (size_t i = 1; i < centerline_.size(); ++i) {
                cumulativeDistance_[i] = cumulativeDistance_[i - 1] + (centerline_[i] - centerline_[i - 1]).length();
            }
        }

        if (centerline_.size() >= 2) {
            const tnl::Vector3 dir = centerline_[1] - centerline_[0];
            setYaw(DirectionToYaw(dir));
        }

        dynamics_.targetSpeed = TRADE_SHIP_CRUISE_SPEED;
    }

    void gmTradeShip::update(float deltaTime)
    {
        // 進捗計測・停滞タイムアウトは、バック中かどうかによらず常に行う
        // (updateSteeringAI()の中だけに置くと、バック中はこの判定自体が止まってしまい、
        // 最終防衛ラインとして機能しなくなるため)。
        updateProgressTracking(deltaTime);

        if (collisionBackoff_) {
            updateCollisionBackoff(deltaTime);
        }
        else {
            updateSteeringAI(deltaTime);
        }

        gmShip::update(deltaTime);
    }

    // ------------------------------------------------------------
    // 航路追従AI本体(バック中でない時だけ呼ばれる)。
    //   1. 終点到達判定(到達していればkill()してデスポーン)
    //   2. 先読み点への角度差からtargetRudderを求める(seek+path followingを1本の式に単純化)
    // 進捗計測(現在位置に最も近い中心線上の点の更新・停滞タイムアウト+ワープ)は、
    // updateProgressTracking()側でバック中かどうかによらず常に行われている。
    // ------------------------------------------------------------
    void gmTradeShip::updateSteeringAI(float deltaTime)
    {
        if (centerline_.size() < 2) {
            kill(); // 不正なデータ(呼び出し側のチェック漏れ)への保険
            return;
        }

        // ---- 終点(G)到達判定 ----
        // 進捗計測(advanceNearestIndex等)はupdateProgressTracking()側で毎フレーム行われている前提
        const float remaining = cumulativeDistance_.back() - cumulativeDistance_[nearestIndex_];
        if (remaining <= TRADE_SHIP_ARRIVAL_THRESHOLD) {
            kill();
            return;
        }

        // ---- 操舵: 先読み点への角度差→targetRudder ----
        const tnl::Vector3 lookahead = getLookaheadPoint();
        tnl::Vector3 toLookahead = lookahead - position_;
        toLookahead.y = 0.0f;

        if (toLookahead.length() > 0.001f) {
            const tnl::Vector3 forward = tnl::Vector3::Normalize(getForward());
            const tnl::Vector3 toLookaheadN = tnl::Vector3::Normalize(toLookahead);

            // 符号付き角度差(-π〜+π)をatan2(外積のY成分, 内積)で求める。
            // asin単体と違い、90度を超える大きな角度差でも符号・大きさとも正しく扱える。
            const float cross = forward.x * toLookaheadN.z - forward.z * toLookaheadN.x;
            const float dot = std::clamp(tnl::Vector3::Dot(forward, toLookaheadN), -1.0f, 1.0f);
            const float angleDiff = atan2f(cross, dot);

            dynamics_.targetRudder = std::clamp(-angleDiff * TRADE_SHIP_STEERING_GAIN, -1.0f, 1.0f);
        }

        dynamics_.targetSpeed = TRADE_SHIP_CRUISE_SPEED;

        // ---- デバッグ専用: 強制的に操舵を破綻させる(Oキー、debugSetForcedBadSteering()参照) ----
        // 通常の操舵計算の結果を、ここで最後に上書きするだけ。上のロジックは一切変更していない。
        // 値は動作確認したい状況(島側に旋回させたい/離れる側に旋回させたい)に応じて
        // 1.0f・-1.0fを直接書き換えて使う想定(現状: 島側へ向かう-1.0fに設定中)。
        if (debugForcedBadSteering_) {
            dynamics_.targetRudder = -1.0f; // 舵を一方向に固定し続け、意図的に航路から逸れさせる
        }
    }

    // ------------------------------------------------------------
    // 島衝突による一時バック挙動。
    // targetSpeedを後退段階に、targetRudderは衝突時に決めた向きへ軽くかけるだけに徹する
    // ------------------------------------------------------------
    void gmTradeShip::updateCollisionBackoff(float deltaTime)
    {
        dynamics_.targetSpeed = TRADE_SHIP_COLLISION_BACKOFF_SPEED;
        dynamics_.targetRudder = collisionRudderBias_ * 0.3f; // 強く切りすぎず、軽く対称性を崩す程度

        collisionBackoffTimer_ -= deltaTime;
        if (collisionBackoffTimer_ <= 0.0f) {
            collisionBackoff_ = false;
            dynamics_.targetSpeed = TRADE_SHIP_CRUISE_SPEED;
        }
    }

    // ------------------------------------------------------------
    // 進捗(累積距離)のハイウォーターマーク更新・停滞タイムアウト判定・ワープ。
    // update()から、collisionBackoff_の状態によらず毎フレーム必ず呼ばれる
    // (バック中もここは動き続けるので、「バックし続けて一向に解決しない」ケースでも
    // 最終防衛ラインとして機能する)。
    // ------------------------------------------------------------
    void gmTradeShip::updateProgressTracking(float deltaTime)
    {
        if (centerline_.size() < 2) {
            return; // 不正なデータはupdateSteeringAI()側でkill()される
        }

        advanceNearestIndex();

        const float currentDistance = cumulativeDistance_[nearestIndex_];
        if (currentDistance > highWaterMarkDistance_ + 0.01f) {
            highWaterMarkDistance_ = currentDistance;
            stuckTimer_ = 0.0f;
        }
        else {
            stuckTimer_ += deltaTime;
            if (stuckTimer_ >= TRADE_SHIP_STUCK_TIMEOUT) {
                warpToRecoveryPoint();
                stuckTimer_ = 0.0f;
            }
        }
    }

    // ------------------------------------------------------------
    // 停滞タイムアウト成立時の共通処理: 最寄りの経路上地点(ハイウォーターマークより
    // 少し先)へワープし、向きも補正する。バック中に成立した場合は、バックの状態も解除する
    // (ワープ後は正常な航路上の位置に戻っているため、バックを続ける理由が無いため)。
    // ------------------------------------------------------------
    void gmTradeShip::warpToRecoveryPoint()
    {
        const float warpTargetDistance = highWaterMarkDistance_ + TRADE_SHIP_WARP_ADVANCE_DISTANCE;

        size_t warpIndex = nearestIndex_;
        while (warpIndex + 1 < cumulativeDistance_.size() && cumulativeDistance_[warpIndex] < warpTargetDistance) {
            ++warpIndex;
        }

        position_ = centerline_[warpIndex];
        nearestIndex_ = warpIndex;
        highWaterMarkDistance_ = cumulativeDistance_[warpIndex];

        const size_t faceIndex = std::min(warpIndex + 1, centerline_.size() - 1);
        const tnl::Vector3 dir = centerline_[faceIndex] - centerline_[warpIndex];
        if (dir.length() > 0.001f) {
            setYaw(DirectionToYaw(dir));
        }

        // バック中にワープした場合、正常な航路上の位置へ移動済みなのでバックを終了する
        collisionBackoff_ = false;
        dynamics_.targetSpeed = TRADE_SHIP_CRUISE_SPEED;
    }

    // ------------------------------------------------------------
    // nearestIndex_から前方だけを見て、position_に最も近い点を探し直す。
    // 中心線は密にサンプリング済み(数百〜千点程度)なので、後戻りは考慮せず、
    // 探索範囲を区切ることで毎フレームの負荷を抑える。
    // ------------------------------------------------------------
    void gmTradeShip::advanceNearestIndex()
    {
        float bestDist = (centerline_[nearestIndex_] - position_).length();

        constexpr size_t SEARCH_WINDOW = 40; // 1フレームあたりに先読みする点数の上限
        const size_t searchLimit = std::min(nearestIndex_ + SEARCH_WINDOW, centerline_.size() - 1);

        for (size_t i = nearestIndex_ + 1; i <= searchLimit; ++i) {
            const float d = (centerline_[i] - position_).length();
            if (d < bestDist) {
                bestDist = d;
                nearestIndex_ = i;
            }
        }
    }

    // ------------------------------------------------------------
    // nearestIndex_からTRADE_SHIP_LOOKAHEAD_DISTANCEだけ先にある中心線上の点を返す。
    // 末尾を超える場合は終点(G)を返す。
    // ------------------------------------------------------------
    tnl::Vector3 gmTradeShip::getLookaheadPoint() const
    {
        const float targetDistance = cumulativeDistance_[nearestIndex_] + TRADE_SHIP_LOOKAHEAD_DISTANCE;

        size_t idx = nearestIndex_;
        while (idx + 1 < cumulativeDistance_.size() && cumulativeDistance_[idx] < targetDistance) {
            ++idx;
        }
        return centerline_[idx];
    }

    // ------------------------------------------------------------
    // 衝突時、簡易な優先度ルールに基づいて一時バック挙動に入る
    // (既にバック中なら何もしない)。
    // 
    // revertToLastSafePosition()は衝突カテゴリを問わず常に呼ぶ(既存の移動抑止動作)。
    // ------------------------------------------------------------
    void gmTradeShip::onCollisionEnter(gmObjectBase* other)
    {
        // Note:
        // 位置の巻き戻しは、バック開始のきっかけになった最初の接触の時だけ行う。
        // バック中(collisionBackoff_が既にtrue)にまで毎フレーム巻き戻してしまうと、
        // 「後退しようとした移動」自体がその都度キャンセルされ、targetRudderによる
        // 向き変化だけが効いて位置が一切動かない(=島にくっついたまま船首だけ振れる)
        // という状態に陥ることが実機検証で判明したため。
        if (!collisionBackoff_) {
            revertToLastSafePosition();
        }


        if (!other) {
            return;
        }

        // ------------------------------------------------------------
        // 船同士の衝突について。
        // 「どちらか一方が必ず道を譲る」という簡易な優先度ルールでバック要否を決める。
        // 正確な物理シミュレーション(押しのけ力の計算等)は行わず、対称性さえ崩れれば
        // デッドロックしない、という最低限の保証だけを目的にしている。
        //   ・島: 常にバック(不動の障害物)
        //   ・流氷: 常にバック(交易船側が道を譲る。環境側の障害物として扱う)
        //   ・交易船同士: インスタンスのアドレス比較で優先度を決め、劣後側だけがバックする
        //     (this < other なら自分が道を譲る、というだけの取り決め。物理的な意味は無く、
        //     2隻が矛盾なく「どちらか一方だけ」を選べれば良いための機械的なタイブレークルール)
        //   ・その他(プレイヤー船 等): 常にバック(プレイヤー優先)
        // ------------------------------------------------------------
        bool shouldBackoff = false;

        switch (other->getCollisionCategory()) {
        case gmCollisionCategory::Island:
        case gmCollisionCategory::Iceberg:
            shouldBackoff = true;
            break;

        case gmCollisionCategory::Ship:
            if (gmTradeShip* otherTradeShip = dynamic_cast<gmTradeShip*>(other)) {
                shouldBackoff = (this < otherTradeShip);
            }
            else {
                shouldBackoff = true; // 交易船以外の船(プレイヤー船 等)には常に道を譲る
            }
            break;

        default:
            shouldBackoff = true; // 想定外のカテゴリも、安全側に倒してバックしておく
            break;
        }

        if (shouldBackoff && !collisionBackoff_) {
            collisionBackoff_ = true;
            collisionBackoffTimer_ = TRADE_SHIP_COLLISION_BACKOFF_DURATION;

            // 相手から見て自分がどちら側にいるかで、バック中にかける舵の向きを決める
            // (対称性を崩し、バック→また同じ相手へ、の繰り返しを防ぐ)
            const tnl::Vector3 toSelf = position_ - other->getPosition();
            const tnl::Vector3 forward = getForward();
            const float side = forward.x * toSelf.z - forward.z * toSelf.x;
            collisionRudderBias_ = (side >= 0.0f) ? 1.0f : -1.0f;
        }
    }




    // ------------------------------------------------------------
    // デバッグ専用: 実際の衝突無しに、島衝突時と同じバック挙動を即座に発生させる(Pキー)。
    // onCollisionEnter()の島衝突時の処理(collisionBackoff_関連の状態遷移)と全く同じことを、
    // テスト用の固定の舵バイアスで行うだけ。既存のonCollisionEnter()自体は無改変。
    // リリースビルドの成果物に含めないため#ifdef _DEBUGで囲む(宣言側も同様、gmTradeShip.h参照)。
    // ------------------------------------------------------------
#ifdef _DEBUG
    void gmTradeShip::debugTriggerCollisionBackoff()
    {
        collisionBackoff_ = true;
        collisionBackoffTimer_ = TRADE_SHIP_COLLISION_BACKOFF_DURATION;
        collisionRudderBias_ = 1.0f; // テスト用に固定(実際の衝突時は接触方向から自動算出される)
    }
#endif
}
