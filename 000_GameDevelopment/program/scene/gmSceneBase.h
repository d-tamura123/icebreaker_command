#pragma once
#include <memory>

namespace gm
{
    // 前方宣言
    class gmSceneManager;

    class gmSceneBase
    {
    public:
        virtual ~gmSceneBase() = default;

        // シーン開始時に呼ばれる
        // SceneManager を受け取り、必要なら context を取得する
        virtual void onEnter(std::shared_ptr<gmSceneManager> manager) = 0;

        // 毎フレーム更新
        virtual void update() = 0;

        // 毎フレーム描画
        virtual void draw() = 0;

        // シーン終了時に呼ばれる
        virtual void onExit() = 0;

    protected:
        // SceneManager への参照（GameContext にアクセスするため）
        std::shared_ptr<gmSceneManager> sceneManager_;
    };
}
