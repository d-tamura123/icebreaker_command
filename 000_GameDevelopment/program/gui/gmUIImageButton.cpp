// gmUIImageButton.cpp
#include "gmUIImageButton.h"
#include "../util/gmCursorUtil.h"
#include <DxLib.h>

namespace gm {

    gmUIImageButton::gmUIImageButton(
        const tnl::Vector2f& pos,
        float width,
        float height,
        const std::string& normalImagePath,
        const std::string& hoverImagePath,
        const std::string& pressedImagePath,
        std::function<void()> onClick)
        : gmUIObjectBase(pos)
        , width_(width)
        , height_(height)
        , onClick_(std::move(onClick))
    {
        hNormal_  = LoadGraph(normalImagePath.c_str());
        hHover_   = LoadGraph(hoverImagePath.c_str());
        hPressed_ = LoadGraph(pressedImagePath.c_str());

        input_ = dxe::Input::Create();
    }

    void gmUIImageButton::update(float dt)
    {
        if (!inputEnabled_ || !input_) {
            // カーソルモードでない間はボタンが反応する必要が無いので、常に通常状態にしておく
            state_ = eState::Normal;
            wasPressedWhileHovering_ = false;
            return;
        }

        int mx = 0, my = 0;
        gmCursorUtil::GetMousePoint(mx, my);

        const bool hovering =
            static_cast<float>(mx) >= position_.x && static_cast<float>(mx) <= position_.x + width_ &&
            static_cast<float>(my) >= position_.y && static_cast<float>(my) <= position_.y + height_;

        const bool leftHeld     = input_->keep(dxe::Input::eButton::MOUSE_LEFT);
        const bool leftReleased = input_->released(dxe::Input::eButton::MOUSE_LEFT);

        if (hovering && leftHeld) {
            state_ = eState::Pressed;
        }
        else if (hovering) {
            state_ = eState::Hover;
        }
        else {
            state_ = eState::Normal;
        }

        // クリック確定: ボタン上で押されていた状態のまま、ボタン上で離された瞬間だけ発火する
        // (押した後にボタンの外へドラッグして離した場合はクリック扱いにしない)
        if (hovering && leftReleased && wasPressedWhileHovering_) {
            if (onClick_) onClick_();
        }

        wasPressedWhileHovering_ = (hovering && leftHeld);
    }

    void gmUIImageButton::draw()
    {
        int handle = hNormal_;
        switch (state_) {
            case eState::Hover:   handle = hHover_;   break;
            case eState::Pressed: handle = hPressed_; break;
            default: break;
        }

        DrawExtendGraph(
            static_cast<int>(position_.x),
            static_cast<int>(position_.y),
            static_cast<int>(position_.x + width_),
            static_cast<int>(position_.y + height_),
            handle,
            TRUE
        );
    }

}
