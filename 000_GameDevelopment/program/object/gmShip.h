#pragma once
#include "gmMeshBase.h"
#include "gmWaterPlane.h"
#include "../gmGameConfig.h"
#include <dxe.h>
#include <unordered_map>

namespace gm {

    // 前方宣言
    class gmIceberg;

    // gmShip.h
    class gmShip : public gm::gmMeshBase {
    public:
        // 運動状態を保持する変数群
        struct ShipDynamics final {
            float speed         = 0.0f;         // 現在速度
            float targetSpeed   = 0.0f;         // 目標速度（W/Sで段階変更）
            float rudder        = 0.0f;         // 現在舵角（-1.0〜1.0）
            float targetRudder  = 0.0f;         // 目標舵角（A/Dで段階変更）
            float yaw           = 0.0f;         // 船の向き
        };

        static constexpr float SPEED_LEVELS[7] = {
           -1.0f, -0.5f, 0.0f, 0.25f, 0.5f, 0.75f, 1.0f
        };

        // Q/Eキー(トグル式の舵角操作)用の5段階。SPEED_LEVELSと同じ考え方で、
        // 押すたびに1段ずつ移動する(gmPlayerShip::handleInput()参照)。
        static constexpr float RUDDER_LEVELS[5] = {
           -1.0f, -0.5f, 0.0f, 0.5f, 1.0f
        };

        // 舵角が0から最大値(±1.0)に到達するまでの時間(秒)。
        static constexpr float RUDDER_RAMP_TIME = 2.8f;

        tnl::Vector3 getForward() const {
            // Y軸回転（船の向き）から forward ベクトルを作る
            float yaw = dynamics_.yaw;
            return {
                sinf(yaw),
                0.0f,
                cosf(yaw)
            };
        }

    public:
        gmShip(const std::string& id, const tnl::Vector3& pos)
            : gmMeshBase(id, pos) {
        }

        void update(float deltaTime) override;                 // 入力・物理・状態更新
        void render(const Shared<dxe::Camera>& camera) override;

        void setWater(const std::shared_ptr<gmWaterPlane>& water);
        std::shared_ptr<gmWaterPlane> getWater() const;

        void setYaw(float yaw);
        float getYaw() const;

        // コライダー自動生成
        void setupDefaultCollider();

        // コライダーを手動指定で設定する(自動計算が信頼できない場合の代替経路)
        // arg1... カプセルの半径
        // arg2... カプセルの胴体長(半球キャップ分を除いた長さ)
        // arg3... 船の中心からの前後方向オフセット(ローカルZ、正で船首側)
        void setupManualCollider(float radius, float length, float forwardOffset = 0.0f);

        // 衝突検出イベント
        // 移動前の位置に丸ごと戻す(移動抑止)
        void onCollisionEnter(gmObjectBase* other) override;

        // ------------------------------------------------------------
        // ダメージ・HP管理。外部(流氷の接触判定等)からダメージを与える入り口。
        // HPが0になった瞬間にDestroyed状態へ遷移し、以後は通常の操舵・移動を行わず
        // updateDestroyed()による撃沈演出だけが進む。
        // arg1... ダメージ量
        // arg2... ダメージを与えた相手(将来のログ・演出用。未使用でも可)
        // arg3... 大ダメージ(初回接触相当)かどうか。onDamaged()へそのまま伝播する
        // ------------------------------------------------------------
        void applyDamage(float amount, gmObjectBase* source, bool isBigHit);

        float getHp() const { return hp_; }
        float getMaxHp() const { return maxHp_; }
        bool isDestroyed() const { return state_ == ShipState::Destroyed; }

        int getSpeedIndex() const { return speedIndex_; }
        const ShipDynamics& getDynamics() const { return dynamics_; }

    private:
        void updateEngine(float deltaTime);                 // 速度段階・慣性
        void updateRudder(float deltaTime);                 // 舵角
        void updateMovement(float deltaTime);               // 前進・旋回
        void updateWave(float deltaTime);                   // 波同期・傾き
        void updateIcebergContactDamage(float deltaTime);   // 継続分(DoT)適用・猶予タイマーの経過処理
        void updateDestroyed(float deltaTime);              // Destroyed状態の処理

    protected:
        // ------------------------------------------------------------
        // イベント風のフック(派生クラスでオーバーライドして使う)。
        // ------------------------------------------------------------
        // ダメージを受けるたびに呼ばれる(初回大ダメージ・継続ダメージ問わず)
        virtual void onDamaged(float amount, bool isBigHit) {}
        
        // HPが0になった瞬間、Destroyed状態へ遷移する直前に1回だけ呼ばれる
        virtual void onDeath() {}
        
        // 撃沈演出(傾き→沈み込み)が完了した瞬間に1回だけ呼ばれる。
        // 交易船はここでkill()してデスポーンし、プレイヤー船はここでフェード+再配置を行う想定。
        virtual void onDestroyedComplete() {}


        // 流氷との接触ダメージ判定
        // (氷山インスタンスごとの猶予タイマー管理込み)。
        // 
        // onCollisionEnter()から呼ぶ想定(gmShip自身と、gmTradeShipの両方から呼ばれる)。
        void applyIcebergContactDamage(gmObjectBase* other);

        // Destroyed状態を解除し、通常状態に戻す
        // (プレイヤーの再配置時に使う想定)。
        // 
        // HP全回復・傾き/経過時間のリセットを行う。位置・向きの再配置自体は
        // 呼び出し元(setPosition/setYaw)の責務とする。
        void resetToNormalState();

        // 状態管理
        enum class ShipState {
            Normal,
            Destroyed,
        };
        ShipState state_ = ShipState::Normal;

        ShipDynamics dynamics_;
        int speedIndex_ = 2; // 停止
        float waveTime_ = 0.0f;
        tnl::Vector3 wavePos_ {0, 0, 0};

        std::weak_ptr<gmWaterPlane> water_;

        // ---- ゲームルールのパラメータ ----
        float hp_ = SHIP_MAX_HP;
        float maxHp_ = SHIP_MAX_HP;

        // ---- コライダー調整用パラメータ ----
        // 実測で船首・船尾がカプセルに覆われていなかったため、
        // バウンディングボックスの前後長にかける倍率で余裕を持たせる。
        // 1.0で「半球キャップ分を差し引いた胴体長」、大きくするほど船首・船尾側に伸びる。
        static constexpr float SHIP_COLLIDER_LENGTH_SCALE = 1.4f;

    private:
        // 流氷ごとの接触状態(大ダメージの猶予・このフレームで接触したか)を監視する。
        struct IcebergContact {
            float graceTimer = 0.0f;         // これが0より大きい間、大ダメージの再発を抑止する
            bool  touchedThisFrame = false;  // このフレームで接触したか(DoT判定用。毎フレーム末尾でリセット)
            bool  justBigHit = false;        // このフレームで大ダメージを与えたばかりか(同フレームでのDoT二重適用を防ぐ)
        };
        // Note: キーはgmIceberg*の生ポインタ。マップのキーとしての同一性比較にしか使わず、
        // 氷山が消滅した後のポインタを実際に参照(デリファレンス)することは無いため実害は無い想定。
        // 接触が途絶えて猶予も切れたエントリはupdateIcebergContactDamage()内で自動的に間引かれるため、
        // 消滅済みの氷山を指すキーが長期間残り続けることも無い。
        std::unordered_map<gmIceberg*, IcebergContact> icebergContacts_;

        float destroyedElapsed_ = 0.0f;         // Destroyed状態に入ってからの経過秒数
        bool  destroyedCompleteFired_ = false;  // onDestroyedComplete()を呼び終えたか(二重呼び出し防止)

        // Destroyed状態に入った瞬間の傾き・Y座標。イーズイン補間の起点として使う
        // (死亡直前の波による多少の傾き・高さのブレを引き継いだ状態から始める)。
        float destroyedStartTiltZ_ = 0.0f;
        float destroyedStartY_ = 0.0f;
    };
}
