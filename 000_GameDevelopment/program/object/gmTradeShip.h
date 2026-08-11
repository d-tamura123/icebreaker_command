// gmTradeShip.h
#pragma once
#include <vector>
#include <functional>
#include "gmShip.h"

namespace gm {

    // ------------------------------------------------------------
    // NPC交易船。gmPlayerShipと同じくgmShipを継承し、
    // handleInput()の代わりに航路追従AI(updateSteeringAI())が
    // dynamics_.targetSpeed / targetRudder を書き換える。
    // 「NPCもプレイヤー船と同じ動きの範囲内」という設計方針のため、
    // yaw/rotationを直接いじる処理は一切持たない
    // (gmShip::updateMovement()の共通パイプラインに委ねる)。
    //
    // 【航路追従AIの要点】
    //   ・操舵: 現在位置から中心線上の「少し先の点(先読み点)」への角度差を
    //     targetRudderに変換する。
    // 
    //   ・移動不能の保険として2段階、島衝突時と進捗管理の2段階で対応する。
    // 
    //   ・衝突時: 通常の操舵を一時的に止め、後退(バック)する例外処理
    //     (SPEED_LEVELSの負の段階を使う。NPC専用の特殊な動きは増やさない)。
    // 
    //     島・流氷には常に道を譲り、交易船同士はインスタンス比較でどちらか一方だけが
    //     バックする(簡易な優先度ルール。詳細はonCollisionEnter()参照)。
    // 
    //     流氷との接触はバックとは別に、gmShip側のHP・ダメージ機構(applyIcebergContactDamage())
    //     にも渡り、実際のダメージ判定はそちらが担う。
    // 
    //   ・進捗管理(最終的な担保): 経路上の進捗(累積距離)のハイウォーターマークを追跡し、
    //     一定時間更新が無ければ最寄りの経路上地点へワープ+向き補正する保険。
    //
    //     この計測とワープ判定は update() 直下で常に行う。
    //     バック中に計測を止めると、NPCが延々バックし続けて抜け出せない
    //     実機で確認された不具合が再発するため。
    //
    //   ・終点(G)到達、またはHP0による撃沈でデスポーン(isAlive()がfalseになる)
    // ------------------------------------------------------------
    class gmTradeShip : public gmShip {
    public:
        // arg1... インスタンスID
        // arg2... この船が辿る航路の中心線(ワールド座標、S→Gの順。
        //         gmRouteCenterlineUtil::SampleRouteCenterline()の出力をそのまま渡す想定)。
        //         2点未満は不正なデータとして扱う(呼び出し側で事前にチェックすること)。
        gmTradeShip(const std::string& id, const std::vector<tnl::Vector3>& centerline);

        void update(float deltaTime) override;

        // 衝突検出イベント。
        // 相手に応じてバック要否を決める。
        // 流氷が相手ならgmShip::applyIcebergContactDamage()でダメージ判定も行う。
        void onCollisionEnter(gmObjectBase* other) override;

        // ------------------------------------------------------------
        // 終点(G)へ到達した瞬間(kill()の直前)に呼ばれるコールバックを設定する。
        // 引数は到着時点のHP比率(0.0〜1.0)。資金報酬の計算・付与は
        // gmTradeShipManager側の責務のため、gmTradeShip自身はgmWallet等を
        // 直接知らずに済むよう、汎用的なコールバックとして受け取る形にしている
        // (gmPlayerShip::setOnDestroyedCompleteCallback()と同じ考え方)。
        // ------------------------------------------------------------
        void setOnArrivedCallback(std::function<void(float hpRatio)> callback) {
            onArrivedCallback_ = callback;
        }

#ifdef _DEBUG
        // ------------------------------------------------------------
        // デバッグ専用(O/Pキー、gmGameScene参照)。
        // 異常系(進捗停滞タイムアウト+ワープ/島衝突時のバック)は、
        // 通常プレイではめったに踏まない経路のため、意図的に発生させて
        // 動作確認するための入り口。呼び出し元(gmGameScene)がデバッグモード時のみ
        // 呼ぶ想定で、gmTradeShip自身はキー入力やデバッガの存在を一切知らない
        // (updateSteeringAI()の末尾に1行追記するだけ、既存ロジックは無改変)。
        // ------------------------------------------------------------

        // 強制的に操舵を破綻させるかどうかを切り替える(Oキー想定)。
        // ON中は毎フレーム、通常の航路追従計算の結果を無視してtargetRudderを
        // 一方向に固定し続け、航路から逸れさせる(=進捗停滞タイムアウト→ワープの動作確認用)。
        void debugSetForcedBadSteering(bool enabled) { debugForcedBadSteering_ = enabled; }

        // 島衝突時のバック挙動を、実際の衝突無しに即座に発生させる(Pキー想定)。
        // onCollisionEnter()の島衝突時の処理と同じ状態遷移を、テスト用の固定値で行う。
        void debugTriggerCollisionBackoff();
#endif

    protected:
        // 撃沈演出完了時に呼ばれる(gmShip参照)。
        // 交易船は単純にkill()してデスポーンするだけでよい。
        // (到着していれば得られたはずの資金の機会損失、という扱いでスコアは特に操作しない)
        void onDestroyedComplete() override;

    private:
        void updateSteeringAI(float deltaTime);
        void updateCollisionBackoff(float deltaTime);

        // 進捗(累積距離)のハイウォーターマーク更新・停滞タイムアウト判定・ワープを行う。
        // update()から、collisionBackoff_の状態によらず毎フレーム必ず呼ぶ
        // (updateSteeringAI()の中に置いてしまうと、バック中はこの関数自体が呼ばれず、
        // 最終防衛ラインのはずのタイムアウトが機能しなくなるため。実機検証で判明した不具合の修正)。
        void updateProgressTracking(float deltaTime);

        // 停滞タイムアウト成立時の共通処理: 最寄りの経路上地点(ハイウォーターマークより
        // 少し先)へワープし、向きも補正する。
        void warpToRecoveryPoint();

        // centerline_上で、現在位置に最も近い点のインデックスを探す。
        // 前フレームのnearestIndex_から前方だけを見て進める(全走査を避けるための最適化)。
        void advanceNearestIndex();

        // centerline_上で、nearestIndex_からTRADE_SHIP_LOOKAHEAD_DISTANCEだけ先にある点を返す
        // (末尾を超える場合は終点を返す)
        tnl::Vector3 getLookaheadPoint() const;

        std::vector<tnl::Vector3> centerline_;      // この船が辿る中心線(不変)
        std::vector<float> cumulativeDistance_;     // centerline_の各点までの累積距離(S=0起点)

        size_t nearestIndex_ = 0;                   // 直近フレームで最も近かったcenterline_上の点
        float  highWaterMarkDistance_ = 0.0f;       // 進捗(累積距離)の最高到達点
        float  stuckTimer_ = 0.0f;                  // ハイウォーターマークが更新されていない秒数

        bool   collisionBackoff_ = false;           // 島衝突による一時バック中かどうか
        float  collisionBackoffTimer_ = 0.0f;       // バックの残り時間
        float  collisionRudderBias_ = 1.0f;         // バック中に軽くかける舵の向き(対称性を崩し、同じ島へ再度当たるのを防ぐ)

        bool   debugForcedBadSteering_ = false;     // デバッグ専用: trueの間、操舵を強制的に破綻させる(Oキー)
        
        // ---- コールバック関連 (イベントハンドラ) ----
        std::function<void(float hpRatio)> onArrivedCallback_; // 終点到達時に呼ばれる(setOnArrivedCallback()参照)
    };

}
