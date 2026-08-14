#include "gmPlayerShip.h"
#include "../input/gmInputManager.h"
#include <dxe.h>

namespace gm {

    void gmPlayerShip::update(float deltaTime)
    {
        // 撃沈演出(傾き・沈み込み・フェード)は
        // gmShip::update()側のupdateDestroyed()に一任する。
        // プレイヤー入力は受け付けない。
        
        if (!isDestroyed()) {
            handleInput();  // 破壊中は入力しない
        }

        gmShip::update(deltaTime);  // 共通ロジックは親に任せる
    }

    // ------------------------------------------------------------
    // 撃沈開始時(HPが0になり、Destroyed状態に入った瞬間): 実際のカメラ演出切り替えは
    // gmGameScene側の責務のため、ここでは設定済みのコールバックを呼ぶだけにする(未設定なら何もしない)。
    // ------------------------------------------------------------
    void gmPlayerShip::onDeath()
    {
        if (onDeathCallback_) {
            onDeathCallback_();
        }
    }

    // ------------------------------------------------------------
    // 撃沈演出完了時: 実際のフェード・再配置・カメラリセットはgmGameScene側の責務のため、
    // ここでは設定済みのコールバックを呼ぶだけにする(未設定なら何もしない)。
    // ------------------------------------------------------------
    void gmPlayerShip::onDestroyedComplete()
    {
        if (onDestroyedCompleteCallback_) {
            onDestroyedCompleteCallback_();
        }
    }

    void gmPlayerShip::handleInput()
    {
        auto input = inputManager_.lock();
        if (!input) return; // 入力管理が未設定の間は何もしない(setInputManager()呼び出し前の保険)

        // -----------------------------
        // 速度段階（W/S）
        // -----------------------------
        if (input->consumePress(gmAction::Ship_SpeedUp, gmInputCallerId::PlayerShip_SpeedUp))
            speedIndex_++;

        if (input->consumePress(gmAction::Ship_SpeedDown, gmInputCallerId::PlayerShip_SpeedDown))
            speedIndex_--;

        speedIndex_ = std::clamp(speedIndex_, 0, 5);

        dynamics_.targetSpeed = SPEED_LEVELS[speedIndex_];

        // -----------------------------
        // 舵角（A/D=押しっぱなし方式 / Q/E=段階トグル方式）
        // -----------------------------
        // 「押しっぱなし方式」と「段階トグル方式」の両方を実装する。
        //   A/D: 押している間だけ最大舵角まで旋回し、離すと中央へ戻る(従来通り)
        //   Q/E: W/Sの速度段階(SPEED_LEVELS)と同じ考え方で、RUDDER_LEVELSの5段階
        //        (-1.0, -0.5, 0.0, 0.5, 1.0)を押すたびに1段ずつ移動する。離しても保持される。
        // 両者は同じdynamics_.targetRudderを操作するが、「離したら中央へ戻る」性質を持つのは
        // A/Dだけなので、A/Dをちょうど離した瞬間だけ明示的に中央(rudderIndex_も含め)へリセットする
        // (rudderHeldLastFrame_で前フレームの保持状態を覚えておき、そこから外れた瞬間を検知する)。
        // これにより、Q/Eで進めた段階はA/Dに一切触れていない限りそのまま保持される。
        const float MAX_RUDDER = 1.0f;

        const bool aHeld = input->isHeld(gmAction::Ship_RudderLeftHold);
        const bool dHeld = input->isHeld(gmAction::Ship_RudderRightHold);

        if (aHeld) {
            dynamics_.targetRudder = -MAX_RUDDER;
        }
        else if (dHeld) {
            dynamics_.targetRudder = MAX_RUDDER;
        }
        else if (rudderHeldLastFrame_) {
            // A/Dをちょうど離した瞬間: 中央へ戻す(Q/E側の段階も中央(index=2)に同期させる)
            rudderIndex_ = 2;
            dynamics_.targetRudder = RUDDER_LEVELS[rudderIndex_];
        }
        else {
            if (input->consumePress(gmAction::Ship_RudderLeftStep, gmInputCallerId::PlayerShip_RudderLeftStep))
                rudderIndex_--;

            if (input->consumePress(gmAction::Ship_RudderRightStep, gmInputCallerId::PlayerShip_RudderRightStep))
                rudderIndex_++;

            rudderIndex_ = std::clamp(rudderIndex_, 0, 4);

            dynamics_.targetRudder = RUDDER_LEVELS[rudderIndex_];
        }

        rudderHeldLastFrame_ = aHeld || dHeld;
    }

}
