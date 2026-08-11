// gmGameScene.cpp
#include "gmGameScene.h"
#include "../gmSceneManager.h"
#include "../../gui/gmUIManager.h"
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

        // プレイヤー撃沈演出(gmShip::updateDestroyed())が完了した瞬間に呼ばれるコールバックを設定する
        playerShip_->setOnDestroyedCompleteCallback([this]() { respawnPlayer(); });




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
            collisionSystem_
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

        // プレイヤー船の位置
        tnl::Vector3 shipPos = playerShip_->getPosition();

        // カメラターゲットを船に合わせる（重要）
        camTarget_ = shipPos;

        // カメラの初期角度と距離
        pitch_ = -0.3f;    // 少し見下ろす
        yaw_ = 0.0f;
        dist_ = 300.0f;

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

        // UIマネージャーの初期化
        tnl::Vector2f miniMapPos(1000.0f, 20.0f);
        uiManager_ = std::make_unique<gmUIManager>(miniMapPos, context_->map, playerShip_, icebergManager_);

        // 航路の可視化(NPC交易船の航路をリボンメッシュで描画。判定には関与しない)
        // context_->map は既にLoadRoutes()済み(gmSceneManagerのコンストラクタで実行)の前提
        routeVisualizer_ = std::make_unique<gmRouteVisualizer>(context_->map);

        // NPC交易船のスポナー
        tradeShipManager_ = std::make_shared<gmTradeShipManager>(context_->map, water_, collisionSystem_);
    }

    // ------------------------------------------------------------
    // 毎フレームの更新。各オブジェクト・マネージャーのupdate()を、
    // 依存関係のある順番(船→カメラ→流氷→発射入力→弾→VFX→衝突判定→UI)に呼んでいく。
    // ------------------------------------------------------------
    void gmGameScene::update()
    {
        float dt = dxe::GetDeltaTime();

        respawnFade_->update(dt); // プレイヤー撃沈時の再配置演出用フェード(Idle中は何もしない)

        debugger_->update();
        playerShip_->update(dt);
        water_->update(context_->camera);

        // カメラ制御
        if (debugger_->isDebugModeOn() && debugger_->isFreeCameraEnabled()) {
            // フリーカメラモード
            debugger_->getFreeCamera()->update(context_->camera);
        }
        else if(!playerShip_->isDestroyed()) {
            // 船追従カメラ
            // Note: 撃沈中(isDestroyed())はこの分岐自体をスキップし、カメラを据え置きにする。
            tnl::Vector3 shipForward = playerShip_->getForward();
            tnl::Vector3 shipPos = playerShip_->getPosition();

            tnl::Vector3 camPos =
                shipPos - shipForward * 250.0f + tnl::Vector3(0, 100, 0);

            context_->camera->setPosition(camPos);
            context_->camera->setTarget(shipPos);
            context_->camera->update();
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
            tryFireProjectileOnClick();
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
            uiManager_->update(dt, context_->camera);
        }

        // 航路可視化のUVスクロール更新(ジオメトリ自体は起動時に生成済みのため再生成しない)
        if (routeVisualizer_) {
            routeVisualizer_->update(dt);
        }
    }

    // ------------------------------------------------------------
    // 描画。奥にあるもの(船・氷塊・島・流氷)→半透明の水面→
    // 弾・VFX→UIの順に描画する(半透明合成が絡むオブジェクトの
    // 見た目の前後関係が崩れないよう、描画順序を意図的に揃えている)。
    // ------------------------------------------------------------
    void gmGameScene::draw()
    {
        playerShip_->render(context_->camera);

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

        dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });
    }

    void gmGameScene::onExit()
    {
        // 特に破棄処理は不要
    }


    // ------------------------------------------------------------
    // プレイヤー撃沈演出完了時のコールバック本体。
    // フェードアウト→(暗転中に)HP全回復・初期位置へ再配置→フェードイン、という流れ。
    // カメラは追従カメラ(update()内)がplayerShip_の位置・向きを毎フレーム見て自動追従する
    // ため、ここで明示的にリセットする必要は無い(yaw_/pitch_/dist_はフリーカメラ専用の変数で、
    // 通常の追従カメラの計算には使われていない)。
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
    // クリック位置を海面(y=0の平面)へレイキャストし、命中すれば割り砲弾を発射する。
    //
    //   手順1: マウス座標から、カメラ位置を始点とするレイ(半直線)の方向を求める
    //   手順2: そのレイが海面(y=0の平面)と交わる点を計算する
    //   手順3: 交点(=クリックした地点)を目標に、弾を発射する
    //
    // レイの方向は、このプロジェクトに既にある tnl::Vector3::CreateScreenRay() を使う
    // (ビュー行列の平行移動成分を除いた上で逆行列変換し、スクリーン座標に対応する
    //  ワールド空間の方向ベクトルを求める、自前実装ではなくテスト済みの共通関数)。
    // レイの始点はカメラのワールド座標そのものを使う。
    // ------------------------------------------------------------
    void gmGameScene::tryFireProjectileOnClick()
    {
        if (!tnl::Input::IsMouseTrigger(tnl::Input::eMouseTrigger::IN_LEFT)) {
            return;
        }
        if (!projectileManager_ || !playerShip_) {
            return;
        }
        if (!flameThrowerManager_ || !playerShip_) {
            return;
        }


        // ---- 手順1: レイ(半直線)の始点と方向を求める ----
        const tnl::Vector3 mousePos = tnl::Input::GetMousePosition();
        const float screenW = context_->camera->getScreenWidth();
        const float screenH = context_->camera->getScreenHeight();

        const tnl::Vector3 rayOrigin = context_->camera->getPosition();
        const tnl::Vector3 rayDir = tnl::Vector3::CreateScreenRay(
            mousePos.x, mousePos.y, screenW, screenH,
            context_->camera->getViewMatrix(), context_->camera->getProjectionMatrix());

        // ---- 手順2: レイと海面(y=0の平面)との交点を求める ----
        // (実際の波の高さは無視する。狙い先の判定用途であれば十分な精度のため)
        //
        // レイ上の点は、始点rayOriginから方向rayDirへtだけ進んだ点として
        //   点 = rayOrigin + rayDir × t
        // と表せる(tは「どれだけ進んだか」を表す媒介変数)。
        // この点のY座標がちょうど0になるtを求めれば、それが海面との交点になる。
        //   rayOrigin.y + rayDir.y × t = 0
        //   ⇔ t = -rayOrigin.y / rayDir.y
        if (std::abs(rayDir.y) < 1e-5f) {
            return; // レイがほぼ水平で、海面と交わらない(rayDir.yが0に近いと上の式で0除算になるため)
        }

        const float t = (0.0f - rayOrigin.y) / rayDir.y;
        if (t <= 0.0f) {
            return; // 海面がカメラの後ろ側にある(通常は起こらないはずだが念のため)
        }

        // ---- 手順3: 交点を目標地点として、割り砲弾を発射する ----
        const tnl::Vector3 targetPos = rayOrigin + rayDir * t;

        // projectileManager_->fire(playerShip_->getPosition(), targetPos);
        projectileManager_->fireSplit(playerShip_->getPosition(), targetPos);
        // flameThrowerManager_->fire(playerShip_, targetPos);
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
