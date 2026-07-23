#include "gmKyleDebugger.h"
#include <DxLib.h>

namespace gm {

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

    }

    void gmKyleDebugger::update()
    {
        // Pauseキーで ON/OFF 切り替え
        // tnl::Input の押した瞬間検知
        if (tnl::Input::IsKeyDownTrigger(eKeys::KB_PAUSE)) {
            debugModeOn_ = !debugModeOn_;
        }
    }

    void gmKyleDebugger::render(const Shared<dxe::Camera>& camera)
    {
#ifdef _DEBUG
        if (axisCompass_) {
            axisCompass_->draw(camera);
        }

        if (worldRuler_) {
            worldRuler_->draw(camera, { 0,0,0 }, 50.0f, 500.0f);
        }
#endif

        if (!debugModeOn_) {
            return;
        }

        if (gridViewer_) {
            gridViewer_->render(camera);
        }



        // 今後ここに他のデバッグ描画を追加
    }

}
