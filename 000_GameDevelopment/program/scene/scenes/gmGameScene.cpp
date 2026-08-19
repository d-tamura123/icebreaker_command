// gmGameScene.cpp
#include "gmGameScene.h"
#include "gmTitleScene.h"
#include "../gmSceneManager.h"
#include "../../gui/gmUIManager.h"
#include "../../gui/gmGameStopUIManager.h"
#include "dxe.h"
#include <cmath>

namespace gm
{
    gmGameScene::gmGameScene()
    {
    }

    gmGameScene::~gmGameScene()
    {
    }

    // ------------------------------------------------------------
    // シーン開始時の初期化。
    // 船・水面・氷塊・島・氷山マネージャー・各種マネージャー(衝突/VFX/弾/UI)・
    // カメラの初期位置を、順番に構築していく。
    // ------------------------------------------------------------
    void gmGameScene::onEnter(std::shared_ptr<gmSceneManager> manager)
    {
        sceneManager_ = manager;
        context_ = manager->getContext();

        // デバッガ
        debugger_ = std::make_shared<gmKyleDebugger>();

        // 水面
        water_ = std::make_shared<gmWaterPlane>(
            "resource/dxe_parameters/water_plane/water_plane.bin"
        );

        // プレイヤー初期位置（map.bin の bit1）
        tnl::Vector2f startPos2D = context_->map->GetPlayerStartWorld();
        tnl::Vector3 startPos3D(startPos2D.x, 0.0f, startPos2D.y);

        // プレイヤー船
        playerShip_ = std::make_shared<gmPlayerShip>("player", startPos3D);
        playerShip_->create("resource/mesh/mv/test/S1.mv1", 0.5f);

        auto tex = dxe::Texture::CreateFromFile("resource/graphics/test/S1_BaseColor.png");
        playerShip_->getMesh()->setTexture(tex);
        playerShip_->setWater(water_);

        // プレイヤー船の向きの設定
        // ゲームルール状、南向きに初期配置
        playerShip_->setYaw(tnl::PI);

        // 衝突システム
        collisionSystem_ = std::make_shared<gmCollisionSystem>();

        // プレイヤー船のコライダーを自動生成して登録
        // したかったが、メッシュ形状が複雑で自動化は骨が折れるので手動で調整した値を直接指定することとする。。
        //playerShip_->setupDefaultCollider();
        playerShip_->setupManualCollider(40.0f, 120.0f, 0.0f);
        collisionSystem_->registerObject(playerShip_);

        // プレイヤー撃沈時の再配置演出専用のフェード(gmSceneManagerのシーン切り替え用フェードとは別物)
        respawnFade_ = std::make_shared<gmFadeTransitionEffect>();
        respawnFade_->setScreenSize(DXE_WINDOW_WIDTH, DXE_WINDOW_HEIGHT);
        respawnFade_->setFadeColor(0, 0, 0);

        // 撃沈開始の瞬間(HP0到達時)、沈む姿を見せるための固定俯瞰カメラへ切り替える
       // (エイムモード中に死亡した場合、船非表示+狭い画角のままだと沈む姿が見えないため)
        playerShip_->setOnDeathCallback([this]() {
            cameraController_->enterDestroyedShowcase(playerShip_, context_->camera);
            });

        // プレイヤー撃沈演出(gmShip::updateDestroyed())が完了した瞬間に呼ばれるコールバックを設定する
        playerShip_->setOnDestroyedCompleteCallback([this]() { respawnPlayer(); });

        // 入力管理をプレイヤー船へ接続
        playerShip_->setInputManager(context_->input);


        // 氷塊
        auto iceTex = dxe::Texture::CreateFromFile("resource/graphics/test/White-Ice4.jpg");

        std::vector<std::string> crystalPaths = {
            "resource/mesh/mv/test/crystals/Crystal_13.mv1",
            "resource/mesh/mv/test/crystals/Crystal_14.mv1",
            "resource/mesh/mv/test/crystals/Crystal_16.mv1",
            "resource/mesh/mv/test/crystals/Crystal_17.mv1",
            "resource/mesh/mv/test/crystals/Crystal_18.mv1",
            "resource/mesh/mv/test/crystals/Crystal_19.mv1",
            "resource/mesh/mv/test/crystals/Crystal_20.mv1"
        };

        iceChunk_ = gm::MeshEX::CreateIceChunk(
            crystalPaths,
            iceTex,
            1.0f,
            8,
            -1
        );
        iceChunk_->setDefaultLightEnable(true);
        iceChunk_->setPosition({ 100, 20, 200 });

        // 島
        auto islandList = context_->map->GetIslands();
        for (auto& isl : islandList) {

            // マネージャーが計算済みの座標をもとに島を生成
            auto islandObj = std::make_shared<gmIsland>(
                "island",
                tnl::Vector3(isl.worldX, -20.0f, isl.worldZ),
                "resource/graphics/test/heightmap_island.png",
                "resource/graphics/test/lawn.png",
                isl.width,
                isl.depth,
                60.0f,       // heightMax
                120, 120     // 分割数
            );

            islands_.push_back(islandObj);
            collisionSystem_->registerObject(islandObj);
        }

        // 氷山マネージャー（南端スポナー + 海流連動の動的な氷山群）
        // 上のcrystalPaths/iceTexをそのまま使い回す
        icebergManager_ = std::make_shared<gmIcebergManager>(
            context_->map,
            water_,
            crystalPaths,
            "resource/graphics/test/White-Ice4.jpg",
            collisionSystem_,
            context_->wallet
        );

        // スプライトアニメーション(VFX)のメタデータを起動時に1回だけ読み込む
        spriteAnimRegistry_ = std::make_shared<gmSpriteAnimRegistry>();
        spriteAnimRegistry_->loadFromCSV(gm::VFX_SPRITE_METADATA_CSV_PATH);

        // 単発の演出用(着弾エフェクト等)。現時点では未使用だが、先に配線しておく
        vfxManager_ = std::make_shared<gmVFXManager>(spriteAnimRegistry_);

        // 通常弾の発射・管理
        projectileManager_ = std::make_shared<gmProjectileManager>(collisionSystem_, spriteAnimRegistry_);

        // 火炎放射攻撃の発動・管理
        flameThrowerManager_ = std::make_shared<gmFlameThrowerManager>(collisionSystem_, spriteAnimRegistry_);

        // 武器選択・リキャスト・リカバリのクールダウン状態
        weaponSelection_ = std::make_shared<gmWeaponSelectionState>();

        // プレイヤー船の位置
        tnl::Vector3 shipPos = playerShip_->getPosition();

        // プレイヤーカメラ(周回モード/エイムモードの制御)
        cameraController_ = std::make_unique<gmPlayerCameraController>();

        // カメラ初期位置（船の後方・上方）
        tnl::Vector3 camPos = {
            shipPos.x,
            shipPos.y + 200.0f,
            shipPos.z - 250.0f
        };

        // カメラ初期化
        context_->camera->setPosition(camPos);
        context_->camera->setTarget(shipPos);
        context_->camera->update();

        // NPC交易船のスポナー
        tradeShipManager_ = std::make_shared<gmTradeShipManager>(context_->map, water_, collisionSystem_, context_->wallet);

        // UIマネージャーの初期化
        // 右マージン24px(1280 - 1000 - 256 = 24)、下マージン20pxで統一
        tnl::Vector2f miniMapPos(1000.0f, DXE_WINDOW_HEIGHT_F - 256.0f - 20.0f);

        uiManager_ = std::make_unique<gmUIManager>(
            miniMapPos, context_->map, playerShip_, icebergManager_, tradeShipManager_,
        
            context_->wallet,
            [this]() {
                // ポーズメニューを開く
                if (gameStopUIManager_) gameStopUIManager_->togglePauseMenu();
                if (context_->input) {
                    context_->input->setActiveLayer(
                        gameStopUIManager_ && gameStopUIManager_->isGameStopped()
                        ? gmInputLayer::Menu : gmInputLayer::Gameplay);
                }
            },

            weaponSelection_,
            [this]() {
                tryUseRecovery();
            }
        );

        // 航路の可視化(NPC交易船の航路をリボンメッシュで描画。判定には関与しない)
        // context_->map は既にLoadRoutes()済み(gmSceneManagerのコンストラクタで実行)の前提
        routeVisualizer_ = std::make_unique<gmRouteVisualizer>(context_->map);

        // マップ外枠の可視化(移動可能範囲の境界をリボンメッシュで描画。判定には関与しない)
        mapBoundaryVisualizer_ = std::make_unique<gmMapBoundaryVisualizer>(context_->map);

        // 新しいシーン開始時は、直前のシーン(前回のポーズ中断等)がMenuレイヤーのまま
        // 残っている可能性があるため、念のため明示的にGameplayへ戻しておく
        // (context_(と、その中のinput)はシーンをまたいで共有されているため)。
        if (context_->input) {
            context_->input->setActiveLayer(gmInputLayer::Gameplay);
        }

        // ポーズメニュー等、ゲームを止めて表示するUIの中間管理層
        gameStopUIManager_ = std::make_unique<gmGameStopUIManager>(
            // Resume: メニューを閉じてゲームプレイへ戻る
            [this]() {
                if (gameStopUIManager_) gameStopUIManager_->closePauseMenu();
                if (context_->input) context_->input->setActiveLayer(gmInputLayer::Gameplay);
            },
            // Restart(確認済み): 資金・経験値・全オブジェクトを含めて完全にやり直す。
            // 個別のリセット処理は書かず、新しいgmGameSceneに丸ごと差し替えるだけで済ませる
            // (onEnter()の初期化処理がそのまま「完全リセット」として機能するため)。
            [this]() {
                if (sceneManager_) {
                    sceneManager_->requestSceneChange(std::make_shared<gmGameScene>());
                }
            },
            // Return to Start(確認済み): 「自爆相当」の即時再配置。
            // 撃沈演出(数秒かかる沈み込みアニメーション)は経由せず、respawnPlayer()
            // (フェードアウト→HP全回復・初期位置再配置→フェードイン)を直接呼ぶことで、
            // 可動部品を減らし、地形スタック等からの脱出手段として素早く・堅牢に機能させる。
            [this]() {
                if (gameStopUIManager_) gameStopUIManager_->closePauseMenu();
                if (context_->input) context_->input->setActiveLayer(gmInputLayer::Gameplay);
                respawnPlayer();
            },
            // Return to Title(確認済み): タイトルシーンへ切り替える
            [this]() {
                if (sceneManager_) {
                    sceneManager_->requestSceneChange(std::make_shared<gmTitleScene>());
                }
            }
        );



}

    // ------------------------------------------------------------
    // 毎フレームの更新。各オブジェクト・マネージャーのupdate()を、
    // 依存関係のある順番(船→カメラ→流氷→発射入力→弾→VFX→衝突判定→UI)に呼んでいく。
    // ------------------------------------------------------------
    void gmGameScene::update()
    {
        float dt = dxe::GetDeltaTime();

        respawnFade_->update(dt); // プレイヤー撃沈時の再配置演出用フェード(Idle中は何もしない)

        // ------------------------------------------------------------
        // ポーズメニュー更新
        // ------------------------------------------------------------
        // ポーズメニューの開閉(Escキー)。一時停止中かどうかに関わらず毎フレーム処理する
        // (開く操作・閉じる操作のどちらもここで拾うため)。
        updateSystemInput();

        if (gameStopUIManager_) {
            gameStopUIManager_->update(dt);

            // 一時停止中は、ここから下のゲームプレイ系の更新を丸ごとスキップする
            // (船・NPC・衝突判定・武器入力等、すべて静止させる)。
            if (gameStopUIManager_->isGameStopped()) {
                return;
            }
        }

        // ------------------------------------------------------------
        // 通常ゲーム更新
        // ------------------------------------------------------------

        debugger_->update();
        playerShip_->update(dt);
        water_->update(context_->camera);

        // カメラ制御
        if (debugger_->isDebugModeOn() && debugger_->isFreeCameraEnabled()) {
            // フリーカメラモード。
            // cameraController_->update()が非表示+ウィンドウ内ロックしたままのカーソルを、
            // フリーカメラ操作(右クリックドラッグ想定)のために戻しておく。
            dxe::SetVisibleMousePointer(true);
            gmCursorUtil::UnlockCursorFromWindow();

            debugger_->getFreeCamera()->update(context_->camera);
        }
        else if (!playerShip_->isDestroyed()) {
            // プレイヤーカメラ(周回モード/エイムモード。)
            // Note: 撃沈中(isDestroyed())はこの分岐自体をスキップし、カメラを据え置きにする。
            if (cameraController_) {
                float weaponMaxRange = WEAPON_PROJECTILE_MAX_RANGE;
                if (weaponSelection_ && weaponSelection_->getSelectedWeapon() == gmWeaponType::Flamethrower) {
                    weaponMaxRange = WEAPON_FLAMETHROWER_MAX_RANGE;
                }
                cameraController_->update(dt, playerShip_, context_->camera, weaponMaxRange);
            }
        }

        // 流氷(スポーン判定 + 海流に乗せた移動)
        if (icebergManager_) {
            icebergManager_->update(dt);
        }

        // NPC交易船(スポーン判定 + 航路追従)
        if (tradeShipManager_) {
            tradeShipManager_->update(dt);
        }

        // NPC交易船の異常系デバッグ用ホットキー(O/P。デバッグモード時のみ。リリースビルドには含めない)
#ifdef _DEBUG
        updateTradeShipDebugHotkeys();
#endif

        // プレイヤーのクリック発射(デバッグモード中はフリーカメラ操作を優先し、発射は行わない)
        if (!debugger_->isDebugModeOn() || !debugger_->isFreeCameraEnabled()) {

            updateWeaponSelectionInput(); // 1/2/3キーでの武器切替、5キーでのリカバリ発動

            tryFireProjectileOnClick();
        }

        // 武器選択・リキャスト・リカバリのクールダウン更新
        if (weaponSelection_) {
            weaponSelection_->update(dt);
        }

        // 発射済みの弾の更新
        if (projectileManager_) {
            projectileManager_->update(dt);
        }

        // 発動中の火炎放射攻撃の更新
        if (flameThrowerManager_) {
            flameThrowerManager_->update(dt);
        }

        // VFX(単発演出)の更新
        if (vfxManager_) {
            vfxManager_->update(dt);
        }

        // 衝突判定
        // 検出のみ。応答は各オブジェクトのonCollisionEnter()で行うこと。
        if (collisionSystem_) {
            collisionSystem_->update();
        }

        // UIマネージャーの更新処理
        if (uiManager_) {
            const bool isFreeCameraActive = debugger_->isDebugModeOn() && debugger_->isFreeCameraEnabled();
            const bool showAimDot = cameraController_ && !isFreeCameraActive && !playerShip_->isDestroyed();

            uiManager_->update(
                dt, context_->camera,
                cameraController_ && cameraController_->isCursorModeActive(),
                showAimDot,
                cameraController_ && cameraController_->isAimMode(),
                cameraController_ ? cameraController_->getAimTargetDistance() : 0.0f);
        }

        // 航路可視化のUVスクロール更新(ジオメトリ自体は起動時に生成済みのため再生成しない)
        if (routeVisualizer_) {
            routeVisualizer_->update(dt);
        }

        // マップ外枠可視化のUVスクロール更新(ジオメトリ自体は起動時に生成済みのため再生成しない)
        if (mapBoundaryVisualizer_) {
            mapBoundaryVisualizer_->update(dt);
        }
    }

    // ------------------------------------------------------------
    // 描画。奥にあるもの(船・氷塊・島・流氷)→半透明の水面→
    // 弾・VFX→UIの順に描画する(半透明合成が絡むオブジェクトの
    // 見た目の前後関係が崩れないよう、描画順序を意図的に揃えている)。
    // ------------------------------------------------------------
    void gmGameScene::draw()
    {
        // エイムモード中はプレイヤー船本体を非表示にする
        //
        // ただし撃沈演出中(isDestroyed())は、死亡した瞬間のエイム状態がそのまま残っていても
        // 必ず表示する(沈む姿を見せるため。カメラ側はenterDestroyedShowcase()で既に俯瞰へ切替済み)。
        if (playerShip_->isDestroyed() || !cameraController_ || !cameraController_->isAimMode()) {
            playerShip_->render(context_->camera);
        }

        if (iceChunk_) {
            iceChunk_->render(context_->camera);
        }

        for (auto& isl : islands_) {
            isl->render(context_->camera);
        }

        if (icebergManager_) {
            icebergManager_->render(context_->camera);
        }

        if (tradeShipManager_) {
            tradeShipManager_->render(context_->camera);
        }

        dxe::DirectXRenderBegin();
        water_->render(context_->camera);
        dxe::DirectXRenderEnd();

        // 航路の可視化(水面より奥、戦闘エフェクトより手前という背景寄りの扱い)
        if (routeVisualizer_) {
            routeVisualizer_->render(context_->camera);
        }

        // マップ外枠の可視化(航路と同じ扱い)
        if (mapBoundaryVisualizer_) {
            mapBoundaryVisualizer_->render(context_->camera);
        }

        if (projectileManager_) {
            projectileManager_->render(context_->camera);
        }

        if (flameThrowerManager_) {
            flameThrowerManager_->render(context_->camera);
        }

        if (vfxManager_) {
            vfxManager_->render(context_->camera);
        }


        if (uiManager_) {
            uiManager_->renderTerrainIntegration(context_->camera);
        }

        // リセット漏れ?
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);  // ブレンドなし（または DX_BLENDMODE_ALPHA, 255）に戻す
        SetUseZBuffer3D(TRUE);                      // Zバッファを使用する
        SetWriteZBuffer3D(TRUE);                    // Zバッファへの書き込みを行う
        SetUseBackCulling(TRUE);                    // バックカリングを行う（デフォルトに戻す）

        if (uiManager_) {
            uiManager_->render(context_->camera);
        }

        debugger_->render(context_->camera, collisionSystem_);

        respawnFade_->draw(); // プレイヤー撃沈時の再配置演出用フェード(最前面に描画)

        if (gameStopUIManager_) {
            gameStopUIManager_->draw(); // ポーズメニュー(一時停止中のみ、内部で判定して描画。最前面)
        }

        dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });
    }

    void gmGameScene::onExit()
    {
        // gmPlayerCameraController::update()がプレイ中に非表示+ウィンドウ内ロックしている
        // マウスカーソルを、他のシーン(タイトル/リザルト/将来のポーズメニュー等)へ
        // 抜ける前に必ず通常状態へ戻す。
        dxe::SetVisibleMousePointer(true);
        gmCursorUtil::UnlockCursorFromWindow();
    }


    // ------------------------------------------------------------
    // プレイヤー撃沈演出完了時のコールバック本体。
    // フェードアウト→(暗転中に)HP全回復・初期位置へ再配置→フェードイン、という流れ。
    // 
    // カメラはcameraController_->update()がplayerShip_の位置・向きを毎フレーム見て
    // 自動追従するため、ここで明示的にリセットする必要は無い。
    // ------------------------------------------------------------
    void gmGameScene::respawnPlayer()
    {
        respawnFade_->fadeOutIn([this]() {
            playerShip_->resetAfterRespawn(); // HP全回復・Destroyed状態解除・傾き/フェードのリセット

            tnl::Vector2f startPos2D = context_->map->GetPlayerStartWorld();
            playerShip_->setPosition(tnl::Vector3(startPos2D.x, 0.0f, startPos2D.y));
            playerShip_->setYaw(tnl::PI); // シーン開始時と同じ、南向き初期配置

            // TODO: 資金マイナス等のスコア処理はここに追加する(後タスクで実装予定)
            });
    }

    // ------------------------------------------------------------
    // ポーズメニューの開閉(Escキー)。gmActionのSystem_TogglePauseMenuは
    // Gameplay/Menu両レイヤーで有効なため、開く操作・閉じる操作のどちらもここで拾える。
    // ------------------------------------------------------------
    void gmGameScene::updateSystemInput()
    {
        if (!context_->input || !gameStopUIManager_) return;

        if (context_->input->consumePress(gmAction::System_TogglePauseMenu, gmInputCallerId::GameScene_TogglePauseMenu)) {
            gameStopUIManager_->togglePauseMenu();
            context_->input->setActiveLayer(
                gameStopUIManager_->isGameStopped() ? gmInputLayer::Menu : gmInputLayer::Gameplay);
        }
    }

    // ------------------------------------------------------------
    // 狙い先(海面上の点)へ、選択中の武器を発射する。
    //
    // 狙い先自体はgmPlayerCameraController側で、周回モード・エイムモードいずれの場合も
    // 「画面中央が指す海面上の座標」として毎フレーム計算済み(getAimTargetWorldPosition())。
    // ------------------------------------------------------------
    void gmGameScene::tryFireProjectileOnClick()
    {
        if (cameraController_ && cameraController_->isCursorModeActive()) {
            // カーソルモード(Alt押下中)はUI操作を優先する。攻撃発火はしない。
            return;
        }
        if (!context_->input ||
            !context_->input->consumePress(gmAction::Weapon_Fire, gmInputCallerId::GameScene_WeaponFire)) {
            return;
        }
        if (!projectileManager_ || !playerShip_) {
            return;
        }
        if (!flameThrowerManager_ || !playerShip_) {
            return;
        }
        if (!weaponSelection_ || !weaponSelection_->canFireSelectedWeapon()) {
            // リキャスト中(または状態が未初期化)なら発射しない
            return;
        }
        if (!cameraController_) {
            return;
        }

        const tnl::Vector3 targetPos = cameraController_->getAimTargetWorldPosition();

        // ---- 選択中の武器に応じて発射処理を振り分ける ----
        switch (weaponSelection_->getSelectedWeapon()) {
        case gmWeaponType::MeltBullet:
            projectileManager_->fire(playerShip_->getPosition(), targetPos);
            break;
        case gmWeaponType::BreakBullet:
            projectileManager_->fireSplit(playerShip_->getPosition(), targetPos);
            break;
        case gmWeaponType::Flamethrower:
            flameThrowerManager_->fire(playerShip_, targetPos);
            break;
        }

        weaponSelection_->notifyFired(); // 選択中武器のリキャストを開始する

    }

    // ------------------------------------------------------------
    // 1/2/3キーでの武器切替、5キーでのリカバリ発動。
    // カーソルモード(Alt押下中)はUI操作を優先し、ここでの入力は受け付けない
    // (武器選択HUDのボタンクリックで代わりに操作する)。
    // ------------------------------------------------------------
    void gmGameScene::updateWeaponSelectionInput()
    {
        if (!context_->input || !weaponSelection_) return;
        if (cameraController_ && cameraController_->isCursorModeActive()) return;

        if (context_->input->consumePress(gmAction::Weapon_Switch1, gmInputCallerId::GameScene_WeaponSwitch1)) {
            weaponSelection_->selectWeapon(gmWeaponType::MeltBullet);
        }
        if (context_->input->consumePress(gmAction::Weapon_Switch2, gmInputCallerId::GameScene_WeaponSwitch2)) {
            weaponSelection_->selectWeapon(gmWeaponType::BreakBullet);
        }
        if (context_->input->consumePress(gmAction::Weapon_Switch3, gmInputCallerId::GameScene_WeaponSwitch3)) {
            weaponSelection_->selectWeapon(gmWeaponType::Flamethrower);
        }
        if (context_->input->consumePress(gmAction::Ship_Recovery, gmInputCallerId::GameScene_Recovery)) {
            tryUseRecovery();
        }
    }

    // ------------------------------------------------------------
    // リカバリ(5キー、または武器選択HUDのリカバリボタンのクリック)発動処理の本体。
    // クールダウン判定→HPへじわじわ回復を適用開始→クールダウン開始、を行う。
    // ------------------------------------------------------------
    void gmGameScene::tryUseRecovery()
    {
        if (!weaponSelection_ || !playerShip_) return;
        if (!weaponSelection_->canUseRecovery()) return;
        if (playerShip_->isDestroyed()) return;

        playerShip_->startRecovery(RECOVERY_HEAL_RATIO * playerShip_->getMaxHp(), RECOVERY_HEAL_DURATION_SEC);
        weaponSelection_->notifyRecoveryUsed();
    }


#ifdef _DEBUG
    // ------------------------------------------------------------
    // デバッグ専用: NPC交易船の異常系(進捗停滞タイムアウト+ワープ/島衝突時のバック)を、
    // O/Pキーで意図的に発生させる。デバッグモード時のみ有効。
    //   O: 強制操舵破綻のON/OFFをトグル(既存・新規スポーン問わず全船に適用し続ける)
    //      → しばらく放置すると、進捗停滞タイムアウト+ワープが発生するはず
    //   P: その時点で存在する全船に、島衝突時と同じバック挙動を即座に発生させる(単発)
    //
    // gmTradeShip側の実装(debugSetForcedBadSteering() / debugTriggerCollisionBackoff())は
    // 既存の航路追従・衝突ロジックを一切変更せず、末尾に追記する形で作られている。
    // ------------------------------------------------------------
    void gmGameScene::updateTradeShipDebugHotkeys()
    {
        if (!debugger_ || !debugger_->isDebugModeOn()) return;
        if (!tradeShipManager_) return;

        if (tnl::Input::IsKeyDownTrigger(tnl::Input::eKeys::KB_O)) {
            debugTradeShipForcedBadSteering_ = !debugTradeShipForcedBadSteering_;
        }

        const bool triggerBackoffTest = tnl::Input::IsKeyDownTrigger(tnl::Input::eKeys::KB_P);

        for (auto& ship : tradeShipManager_->getEntities()) {
            if (!ship) continue;

            ship->debugSetForcedBadSteering(debugTradeShipForcedBadSteering_);

            if (triggerBackoffTest) {
                ship->debugTriggerCollisionBackoff();
            }
        }
    }
#endif

}
