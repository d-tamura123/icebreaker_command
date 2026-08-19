#pragma once
#include "gmShip.h"
#include <functional>
#include <memory>

namespace gm {

    // 前方宣言
    class gmInputManager;

    class gmPlayerShip : public gmShip {
    public:
        gmPlayerShip(const std::string& id, const tnl::Vector3& pos)
            : gmShip(id, pos) {
        }

        void update(float deltaTime) override;

        // 入力管理への参照を設定
        void setInputManager(const std::shared_ptr<gmInputManager>& inputManager) {
            inputManager_ = inputManager;
        }

        // ------------------------------------------------------------
        // 撃沈開始時(HPが0になり、Destroyed状態に入った瞬間)に呼ばれるコールバックを設定する。
        // 沈む姿を見せるためのカメラ演出切り替えなど、gmGameScene側の責務をここから呼び出す想定。
        // setOnDestroyedCompleteCallback()(演出"完了"時)とは呼ばれるタイミングが異なるので注意。
        //
        // このコールバックは gmGameScene::onEnter() で一度だけ設定する想定。
        // ------------------------------------------------------------
        void setOnDeathCallback(std::function<void()> callback) {
            onDeathCallback_ = callback;
        }

        // ------------------------------------------------------------
        // プレイヤー撃沈演出の完了時に呼ばれるコールバックを設定する。
        // 
        // フェード遷移(gmFadeTransitionEffect)、マップ再配置(gmMapManager)、
        // カメラリセットなどは gmGameScene 側の責務であり、gmPlayerShip が
        // gmGameContext 等へ直接依存しないよう、外部からコールバックとして受け取る。
        //
        // このコールバックは gmGameScene::onEnter() で一度だけ設定する想定。
        // ------------------------------------------------------------
        void setOnDestroyedCompleteCallback(std::function<void()> callback) {
            onDestroyedCompleteCallback_ = callback;
        }

        // Destroyed状態を解除して通常状態に戻す(gmShip::resetToNormalState()の公開ラッパー)。
        // 位置・向きの再配置自体は呼び出し元(setPosition/setYaw)の責務。
        void resetAfterRespawn() {
            resetToNormalState();
            rudderIndex_ = 2;             // RUDDER_LEVELSの中央(0.0)
            rudderHeldLastFrame_ = false;
        }

        // ------------------------------------------------------------
        // リカバリ(5キー)発動。合計amountのHPを、durationSec秒かけてじわじわ回復する
        // (即時全回復ではない)。既に発動中の場合は、その時点の残り分を破棄して上書きする。
        //
        // 発射可否・クールダウン管理はgmWeaponSelectionState側の責務。
        // このメソッドは実際の回復量の適用だけを行う(呼ぶ側でクールダウン判定は済んでいる前提)。
        // ------------------------------------------------------------
        void startRecovery(float amount, float durationSec) {
            if (durationSec <= 0.0f) {
                heal(amount);
                return;
            }
            recoveryRatePerSec_ = amount / durationSec;
            recoveryTimeRemaining_ = durationSec;
        }

    protected:

        // 撃沈開始時(HP0到達の瞬間)、設定済みのコールバックを呼ぶだけ。
        void onDeath() override;

        // 撃沈演出完了時、設定済みのコールバックを呼ぶだけ(実際のフェード・再配置はgmGameScene側)。
        void onDestroyedComplete() override;


    private:
        void handleInput();
        void updateRecovery(float deltaTime); // リカバリのじわじわ回復の毎フレーム処理

        // マップ外へ出ないよう、X/Z座標を独立にクランプする(gmShip::update()による移動の後に呼ぶ)。
        // 軸ごとに独立してクランプするため、斜め方向に境界へ突っ込んだ場合も、
        // 境界に沿ってすり抜けるように移動できる(いわゆる壁ズレが自然に実現される)。
        void clampToMapBounds();

        std::weak_ptr<gmInputManager> inputManager_;

        std::function<void()> onDeathCallback_;
        std::function<void()> onDestroyedCompleteCallback_;

        // A/D(押しっぱなし方式)がちょうど離された瞬間を検知するためのフラグ。
        // A/D=離すと中央へ戻る/Q/E=押すたびに切り替わり離しても保持される、という
        // 性質の異なる2つの入力方式が同じdynamics_.targetRudderを操作するため、
        // 「A/Dを離した瞬間だけ明示的に中央へ戻す」判定にこれが必要になる
        // (handleInput()参照)。
        bool rudderHeldLastFrame_ = false;

        // Q/Eキー(トグル式)での現在の舵角段階。RUDDER_LEVELSへの添字。
        // 2(=RUDDER_LEVELSの中央=0.0)を初期値とする。
        int rudderIndex_ = 2;

        // リカバリ進行中の、秒あたりの回復量と残り秒数。0以下の間は何もしない。
        float recoveryRatePerSec_ = 0.0f;
        float recoveryTimeRemaining_ = 0.0f;
    };

}
