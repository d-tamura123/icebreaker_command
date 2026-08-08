#pragma once
#include "gmShip.h"

namespace gm {

    class gmPlayerShip : public gmShip {
    public:
        gmPlayerShip(const std::string& id, const tnl::Vector3& pos)
            : gmShip(id, pos) {
        }

        void update(float deltaTime) override;

    private:
        void handleInput();

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
