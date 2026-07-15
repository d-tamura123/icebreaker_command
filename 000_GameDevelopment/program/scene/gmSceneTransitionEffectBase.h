#pragma once
#include <cstdint>

class gmSceneTransitionEffectBase
{
public:
    virtual ~gmSceneTransitionEffectBase() = default;

    // トランジション開始
    virtual void start() = 0;

    // 更新処理（deltaTime は秒）
    virtual void update(float deltaTime) = 0;

    // 描画処理
    virtual void draw() = 0;

    // シーン切り替えタイミングか？
    virtual bool isSwitchTiming() const = 0;

    // トランジションが完全終了したか？
    virtual bool isFinished() const = 0;

    // 状態リセット
    virtual void reset() = 0;

    // 画面サイズ設定（フェードなどで使用）
    virtual void setScreenSize(int width, int height) = 0;
};
