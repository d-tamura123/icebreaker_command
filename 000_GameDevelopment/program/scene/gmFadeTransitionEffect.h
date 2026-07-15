#pragma once
#include "gmSceneTransitionEffectBase.h"
#include "dxe_build_setting.h"
#include <functional>

class gmFadeTransitionEffect : public gmSceneTransitionEffectBase {
public:
    gmFadeTransitionEffect();

    // 設定
    void setFadeOutDuration(float seconds);
    void setFadeInDuration(float seconds);
    void setFadeColor(int r, int g, int b);
    void setScreenSize(int width, int height) override;

    // 高レベル API：フェードアウト → 切替 → フェードイン を自動処理
    void fadeOutIn(std::function<void()> onSwitch);

    // SceneTransitionEffect の実装
    void start() override;
    void update(float deltaTime) override;
    void draw() override;
    bool isSwitchTiming() const override;
    bool isFinished() const override;
    void reset() override;

private:
    enum class Phase { Idle, FadeOut, SwitchWait, FadeIn, Done };
    Phase phase_ = Phase::Idle;

    // 設定値
    float fadeOutDuration_ = 0.3f;
    float fadeInDuration_ = 0.3f;
    int colorR_ = 0, colorG_ = 0, colorB_ = 0;

    // 状態
    float elapsed_ = 0.0f;
    int alpha_ = 0;
    bool switchTiming_ = false;

    int screenW_ = DXE_WINDOW_WIDTH;
    int screenH_ = DXE_WINDOW_HEIGHT;

    // 改善点：コールバック内蔵
    std::function<void()> onSwitchCallback_;
};
