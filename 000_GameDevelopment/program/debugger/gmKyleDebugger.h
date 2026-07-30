#pragma once

#include "gmKyleGridViewer.h"
#include "gmKyleFreeCameraController.h"
#include "gmKyleAxisCompass.h"
#include "gmKyleWorldRuler.h"
#include "gmKyleColliderGizmo.h"

#include <memory>

namespace gm {

    // 前方宣言
    class gmCollisionSystem;

    class gmKyleDebugger {
    public:
        gmKyleDebugger();

        // 毎フレームの更新（キー入力など）
        void update();

        // 描画
        void render(const Shared<dxe::Camera>& camera, const std::shared_ptr<gmCollisionSystem>& collisionSystem = nullptr);

        // デバックモード参照
        bool isDebugModeOn() const { return debugModeOn_; }

        std::shared_ptr<gmKyleFreeCameraController> getFreeCamera() const { return freeCam_; }
    private:
        bool debugModeOn_ = false;   // デバックモードの ON/OFF

        std::shared_ptr<gmKyleGridViewer> gridViewer_;
        std::shared_ptr<gmKyleFreeCameraController> freeCam_;
        std::shared_ptr<gmKyleAxisCompass> axisCompass_;
        std::shared_ptr<gmKyleWorldRuler> worldRuler_;
        std::shared_ptr<gmKyleColliderGizmo> colliderGizmo_;

    };

}
