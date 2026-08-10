#include "gmKyleFreeCameraController.h"
#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>

namespace gm {
    void gmKyleFreeCameraController::update(Shared<dxe::Camera>& camera)
    {
        // 1. マウスドラッグで回転角の更新
        if (tnl::Input::IsMouseDown(eMouse::RIGHT)) {
            auto v = tnl::Input::GetMouseVelocity();
            yaw_ += v.x * 0.005f;
            pitch_ += v.y * 0.005f;
            pitch_ = std::clamp(pitch_, -1.5f, 1.5f);
        }

        // 2. ホイールで距離（ズーム）の更新（常に最新の dist_ を維持）
        int wheel = tnl::Input::GetMouseWheel();
        if (wheel != 0) {
            float zoom = -wheel * std::max(dist_ * 0.001f, 0.1f);
            dist_ += zoom;
            dist_ = std::clamp(dist_, 10.0f, 5000.0f);
        }

        // 3. キーボードによる注視点（target_）の移動
        float moveSpeed = 20.0f;
        if (tnl::Input::IsKeyDown(tnl::Input::eKeys::KB_UP))    camTarget_.z += moveSpeed;
        if (tnl::Input::IsKeyDown(tnl::Input::eKeys::KB_DOWN))  camTarget_.z -= moveSpeed;
        if (tnl::Input::IsKeyDown(tnl::Input::eKeys::KB_LEFT))  camTarget_.x -= moveSpeed;
        if (tnl::Input::IsKeyDown(tnl::Input::eKeys::KB_RIGHT)) camTarget_.x += moveSpeed;
        if (tnl::Input::IsKeyDown(tnl::Input::eKeys::KB_PGUP))  camTarget_.y += moveSpeed;
        if (tnl::Input::IsKeyDown(tnl::Input::eKeys::KB_PGDN))  camTarget_.y -= moveSpeed;

        // 4. 回転角からカメラの向き（forward ベクトル）を計算
        tnl::Vector3 forward = {
            cosf(pitch_) * sinf(yaw_),
            sinf(pitch_),
            cosf(pitch_) * cosf(yaw_)
        };

        // 5. 注視点（target_）と向き（forward）、距離（dist_）から最終的なカメラ座標（pos）を算出
        // ※ホイールの有無に関わらず、毎フレーム必ず計算する
        tnl::Vector3 pos = camTarget_ - forward * dist_;

        // 6. カメラへ反映
        camera->setPosition(pos);
        camera->setTarget(camTarget_);
        camera->update();
    }
}
