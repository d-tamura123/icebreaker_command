#include "gmSceneManager.h"
#include "dxe.h"

namespace gm
{
    gmSceneManager::gmSceneManager()
    {
        // GameContext を生成
        context_ = std::make_shared<gmGameContext>();

        // 入力
        context_->input = std::make_shared<gmInputManager>();
        context_->input->initialize(gmInputLayer::Gameplay);

        // サウンド
        //context_->sound = std::make_shared<gmSoundManager>();

        // マップ
        context_->map = std::make_shared<gmMapManager>();
        context_->map->LoadMap(gm::MAP_FILE_PATH);
        context_->map->LoadOceanFlow(gm::FLOW_STO_N_PATH);
        context_->map->LoadRoutes();                            // route_1.bin, route_2.bin, ... を読めるだけ読み込む(NPC交易船の航路)

        // カメラ
        context_->camera = std::make_shared<dxe::Camera>(
            DXE_WINDOW_WIDTH_F, DXE_WINDOW_HEIGHT_F
        );

        // フェード
        context_->fade = std::make_shared<gmFadeTransitionEffect>();
        context_->fade->setScreenSize(DXE_WINDOW_WIDTH, DXE_WINDOW_HEIGHT);
        context_->fade->setFadeColor(0, 0, 0); // 黒フェード

        // ウォレット
        context_->wallet = std::make_shared<gmWallet>();
    }

    // ------------------------------------------------------------
    // 初期シーン設定
    // ------------------------------------------------------------
    void gmSceneManager::setInitialScene(std::shared_ptr<gmSceneBase> initialScene)
    {
        currentScene_ = std::move(initialScene);
        currentScene_->onEnter(shared_from_this());
        isTransitioning_ = false;
    }

    // ------------------------------------------------------------
    // シーン切り替え要求
    // ------------------------------------------------------------
    void gmSceneManager::requestSceneChange(std::shared_ptr<gmSceneBase> nextScene)
    {
        if (isTransitioning_) return;

        nextScene_ = std::move(nextScene);
        isTransitioning_ = true;

        // フェードアウト → 切替 → フェードイン
        context_->fade->fadeOutIn([&]() {
            if (currentScene_) currentScene_->onExit();

            currentScene_ = std::move(nextScene_);
            currentScene_->onEnter(shared_from_this());
            });
    }

    // ------------------------------------------------------------
    // 更新
    // ------------------------------------------------------------
    void gmSceneManager::update()
    {
        float dt = dxe::GetDeltaTime();

        // フェード中
        if (isTransitioning_) {
            context_->fade->update(dt);

            if (context_->fade->isFinished()) {
                isTransitioning_ = false;
            }
            return;
        }

        // 入力管理の更新(consumePress()の消費フラグのリセット処理)
        // dxe::Input自体の更新はcommon/dxe/dxe.cpp側で既に毎フレーム行われているため、ここでは行わない。
        if (context_->input) {
            context_->input->update();
        }

        // 通常更新
        if (currentScene_) {
            currentScene_->update();
        }
    }

    // ------------------------------------------------------------
    // 描画
    // ------------------------------------------------------------
    void gmSceneManager::draw()
    {
        if (currentScene_) {
            currentScene_->draw();
        }

        // フェード描画
        if (isTransitioning_) {
            context_->fade->draw();
        }
    }
}
