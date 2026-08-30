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
        hasSwitchedScene_ = false;

        // フェードアウト → 切替 → フェードイン
        context_->fade->fadeOutIn([&]() {
            if (currentScene_) currentScene_->onExit();

            currentScene_ = std::move(nextScene_);
            currentScene_->onEnter(shared_from_this());
            // 新シーンへの切り替わりが完了した合図。以後(フェードイン中も)update()を再開する。
            hasSwitchedScene_ = true;
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

            // フェードアウト中(旧シーンがまだ画面に映っている間)は、意図的にupdate()を
            // 凍結する(見えないところでゲームがこっそり進行し続けるのを防ぐため)。
            //
            // ただし、新シーンへの切り替わりが完了した後(hasSwitchedScene_==true。
            // フェードイン中)は、ここでreturnせずに下まで進めてupdate()を再開する。
            // 切り替わり直後からフェードイン完了まで新シーンのupdate()を止めたままにすると、
            // 「一度もupdate()されていない生成直後の状態」がフェードイン中ずっと描画され続け、
            // フェード完了の瞬間に一気に更新が追いつく「スナップ」が見えてしまうため
            // (詳細な経緯はコミット時のコメント参照)。
            if (!hasSwitchedScene_) {
                return;
            }
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
