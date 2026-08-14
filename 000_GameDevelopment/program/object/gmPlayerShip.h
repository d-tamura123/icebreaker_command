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

    protected:
        // 撃沈演出完了時、設定済みのコールバックを呼ぶだけ(実際のフェード・再配置はgmGameScene側)。
        void onDestroyedComplete() override;

    private:
        void handleInput();

        std::weak_ptr<gmInputManager> inputManager_;

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
    };

}
