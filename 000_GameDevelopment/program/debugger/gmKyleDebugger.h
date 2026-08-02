// gmKyleDebugger.h
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

    // ------------------------------------------------------------
    // 開発中デバッグ機能をまとめて管理するクラス。
    // グリッド表示・フリーカメラ・方位コンパス・定規・コライダーの可視化などを
    // 一括でON/OFFできるようにする。リリースビルドでは基本的に使わない想定。
    // ------------------------------------------------------------
    class gmKyleDebugger {
    public:
        gmKyleDebugger();

        // 毎フレームの更新（キー入力など）
        void update();

        // 描画
        void render(const Shared<dxe::Camera>& camera, const std::shared_ptr<gmCollisionSystem>& collisionSystem = nullptr);

        // デバッグモード参照
        bool isDebugModeOn() const { return debugModeOn_; }

        // フリーカメラの有効/無効
        bool isFreeCameraEnabled() const { return freeCamEnabled_; }

        std::shared_ptr<gmKyleFreeCameraController> getFreeCamera() const { return freeCam_; }

    private:
        bool debugModeOn_ = false;      // デバッグモードのON/OFF
        bool freeCamEnabled_ = false;   // フリーカメラのON/OFF

        std::shared_ptr<gmKyleGridViewer>            gridViewer_;    // 地面グリッド表示
        std::shared_ptr<gmKyleFreeCameraController>  freeCam_;       // フリーカメラ操作
        std::shared_ptr<gmKyleAxisCompass>           axisCompass_;   // 方位コンパス表示
        std::shared_ptr<gmKyleWorldRuler>            worldRuler_;    // ワールド座標の目盛り表示
        std::shared_ptr<gmKyleColliderGizmo>         colliderGizmo_; // コライダー形状の可視化
    };

}
