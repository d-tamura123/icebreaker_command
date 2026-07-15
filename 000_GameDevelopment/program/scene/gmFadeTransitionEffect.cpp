#include "gmFadeTransitionEffect.h"
#include "DxLib.h"

gmFadeTransitionEffect::gmFadeTransitionEffect()
{
    reset();
}

void gmFadeTransitionEffect::setFadeOutDuration(float seconds)
{
    fadeOutDuration_ = (seconds > 0.0f) ? seconds : 0.001f;
}

void gmFadeTransitionEffect::setFadeInDuration(float seconds)
{
    fadeInDuration_ = (seconds > 0.0f) ? seconds : 0.001f;
}

void gmFadeTransitionEffect::setFadeColor(int r, int g, int b)
{
    colorR_ = r;
    colorG_ = g;
    colorB_ = b;
}

void gmFadeTransitionEffect::setScreenSize(int width, int height)
{
    screenW_ = width;
    screenH_ = height;
}

// ------------------------------------------------------------
// 高レベル API：フェードアウト → 切替 → フェードイン
// ------------------------------------------------------------
void gmFadeTransitionEffect::fadeOutIn(std::function<void()> onSwitch)
{
    onSwitchCallback_ = onSwitch;
    reset();
    start();
}

// ------------------------------------------------------------
// フェード開始（フェードアウトからスタート）
// ------------------------------------------------------------
void gmFadeTransitionEffect::start()
{
    phase_ = Phase::FadeOut;
    elapsed_ = 0.0f;
    alpha_ = 0;
    switchTiming_ = false;
}

// ------------------------------------------------------------
// 更新処理
// ------------------------------------------------------------
void gmFadeTransitionEffect::update(float deltaTime)
{
    switchTiming_ = false;

    switch (phase_) {

    case Phase::Idle:
        break;

    case Phase::FadeOut: {
        elapsed_ += deltaTime;
        float t = elapsed_ / fadeOutDuration_;

        if (t >= 1.0f) {
            alpha_ = 255;
            phase_ = Phase::SwitchWait;
            elapsed_ = 0.0f;
            switchTiming_ = true;

            // ★ 改善点：ここで自動コールバック
            if (onSwitchCallback_) {
                onSwitchCallback_();
                onSwitchCallback_ = nullptr;
            }
        }
        else {
            alpha_ = static_cast<int>(t * 255.0f);
        }
        break;
    }

    case Phase::SwitchWait:
        phase_ = Phase::FadeIn;
        elapsed_ = 0.0f;
        break;

    case Phase::FadeIn: {
        elapsed_ += deltaTime;
        float t = elapsed_ / fadeInDuration_;

        if (t >= 1.0f) {
            alpha_ = 0;
            phase_ = Phase::Done;
        }
        else {
            alpha_ = 255 - static_cast<int>(t * 255.0f);
        }
        break;
    }

    case Phase::Done:
        break;
    }
}

// ------------------------------------------------------------
// 描画
// ------------------------------------------------------------
void gmFadeTransitionEffect::draw()
{
    if (phase_ == Phase::Idle || alpha_ <= 0) return;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha_);
    int color = GetColor(colorR_, colorG_, colorB_);
    DrawBox(0, 0, screenW_, screenH_, color, TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

// ------------------------------------------------------------
// 状態チェック
// ------------------------------------------------------------
bool gmFadeTransitionEffect::isSwitchTiming() const
{
    return switchTiming_;
}

bool gmFadeTransitionEffect::isFinished() const
{
    return phase_ == Phase::Done;
}

// ------------------------------------------------------------
// リセット
// ------------------------------------------------------------
void gmFadeTransitionEffect::reset()
{
    phase_ = Phase::Idle;
    elapsed_ = 0.0f;
    alpha_ = 0;
    switchTiming_ = false;
    onSwitchCallback_ = nullptr;
}
