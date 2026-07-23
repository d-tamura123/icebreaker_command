#include "gmGameScene.h"
#include "../gmSceneManager.h"
#include "../../gui/gmUIManager.h"
#include "dxe.h"

namespace gm
{
    gmGameScene::gmGameScene()
    {
    }

    gmGameScene::~gmGameScene()
    {
    }

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
        }

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
        uiManager_ = std::make_unique<gmUIManager>(miniMapPos, context_->map, playerShip_);

    }

    // ------------------------------------------------------------
    // 更新
    // ------------------------------------------------------------
    void gmGameScene::update()
    {
        float dt = dxe::GetDeltaTime();

        debugger_->update();
        playerShip_->update(dt);
        water_->update(context_->camera);

        // カメラ制御
        if (debugger_->isDebugModeOn()) {
            // フリーカメラモード
            debugger_->getFreeCamera()->update(context_->camera);
        }
        else {
            // 船追従カメラ
            tnl::Vector3 shipForward = playerShip_->getForward();
            tnl::Vector3 shipPos = playerShip_->getPosition();

            tnl::Vector3 camPos =
                shipPos - shipForward * 250.0f + tnl::Vector3(0, 100, 0);

            context_->camera->setPosition(camPos);
            context_->camera->setTarget(shipPos);
            context_->camera->update();
        }

        // ★ UIマネージャーの更新処理を呼び出す
        if (uiManager_) {
            uiManager_->update(dt, context_->camera);
        }

    }

    // ------------------------------------------------------------
    // 描画
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

        dxe::DirectXRenderBegin();
        water_->render(context_->camera);
        dxe::DirectXRenderEnd();
        
        if (uiManager_) {
            uiManager_->renderTerrainIntegration(context_->camera);
        }

        // リセット漏れ?
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); // ブレンドなし（または DX_BLENDMODE_ALPHA, 255）に戻す
        SetUseZBuffer3D(TRUE);       // Zバッファを使用する
        SetWriteZBuffer3D(TRUE);      // Zバッファへの書き込みを行う
        SetUseBackCulling(TRUE); // バックカリングを行う（デフォルトに戻す）

        if (uiManager_) {
            uiManager_->render(context_->camera);
        }

        debugger_->render(context_->camera);

        dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });
    }

    void gmGameScene::onExit()
    {
        // 特に破棄処理は不要
    }
}
