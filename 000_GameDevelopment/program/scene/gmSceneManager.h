#pragma once
#include <memory>
#include "gmSceneBase.h"
#include "gmSceneTransitionEffectBase.h"
#include "../gmGameContext.h"

namespace gm
{
    class gmSceneManager : public std::enable_shared_from_this<gmSceneManager>
    {
    public:
        gmSceneManager();
        ~gmSceneManager() = default;

        // 初期シーン設定
        void setInitialScene(std::shared_ptr<gmSceneBase> initialScene);

        // シーン切り替え要求（フェードアウト→切替→フェードイン）
        void requestSceneChange(std::shared_ptr<gmSceneBase> nextScene);

        // 毎フレーム更新
        void update();

        // 毎フレーム描画
        void draw();

        // GameContext 取得
        std::shared_ptr<gmGameContext> getContext() const { return context_; }

        // 現在のシーン取得
        std::shared_ptr<gmSceneBase> getCurrentScene() const { return currentScene_; }

    private:
        // ゲーム全体の依存コンテナ
        std::shared_ptr<gmGameContext> context_;

        // 現在のシーン
        std::shared_ptr<gmSceneBase> currentScene_;

        // 次のシーン（切り替え要求時にセット）
        std::shared_ptr<gmSceneBase> nextScene_;

        // フェード中か？
        bool isTransitioning_ = false;
    };
}
