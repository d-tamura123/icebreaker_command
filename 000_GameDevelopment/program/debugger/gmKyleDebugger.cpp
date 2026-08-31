// gmKyleDebugger.cpp
#include "gmKyleDebugger.h"
#include <DxLib.h>

namespace gm {

    // ------------------------------------------------------------
    // 各デバッグ機能のインスタンスを生成しておく。
    // ------------------------------------------------------------
    gmKyleDebugger::gmKyleDebugger()
    {
        debugModeOn_ = false;

        gridViewer_ = std::make_shared<gmKyleGridViewer>(
            "grid",
            tnl::Vector3(0, 0, 0)
        );

        freeCam_ = std::make_shared<gmKyleFreeCameraController>();

        axisCompass_ = std::make_shared<gmKyleAxisCompass>();

        worldRuler_ = std::make_shared<gmKyleWorldRuler>();

        colliderGizmo_ = std::make_shared<gmKyleColliderGizmo>();
    }

    // ------------------------------------------------------------
    // キー入力を見て、デバッグモード・フリーカメラのON/OFFを切り替える。
    // ------------------------------------------------------------
    void gmKyleDebugger::update()
    {
        // Pauseキーで ON/OFF 切り替え
        // tnl::Input の押した瞬間検知
        if (tnl::Input::IsKeyDownTrigger(eKeys::KB_PAUSE)) {
            debugModeOn_ = !debugModeOn_;
        }

        // F9キーでフリーカメラをON/OFF
        if (tnl::Input::IsKeyDownTrigger(eKeys::KB_F9)) {
            freeCamEnabled_ = !freeCamEnabled_;
        }
    }

    // ------------------------------------------------------------
    // デバッグ用の各種表示を描画する。
    // 方位コンパス・定規は_DEBUGビルドなら常時表示、それ以外
    // (グリッド・コライダー可視化)はデバッグモードON時のみ表示する。
    // ------------------------------------------------------------
    void gmKyleDebugger::render(const Shared<dxe::Camera>& camera, const std::shared_ptr<gmCollisionSystem>& collisionSystem)
    {
        if (!debugModeOn_) {
            return;
        }

#ifdef _DEBUG
        if (axisCompass_) {
            axisCompass_->draw(camera);
        }

        if (worldRuler_) {
            worldRuler_->draw(camera, { 0,0,0 }, 50.0f, 500.0f);
        }
#endif

        if (gridViewer_) {
            gridViewer_->render(camera);
        }

        if (colliderGizmo_ && collisionSystem) {
            colliderGizmo_->render(collisionSystem, camera);
        }

        // 今後ここに他のデバッグ描画を追加
    }

}
