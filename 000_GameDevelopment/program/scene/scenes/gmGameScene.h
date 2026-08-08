// gmGameScene.h
#pragma once
#include "../gmSceneBase.h"
#include "../../gmGameContext.h"

#include "../../object/gmPlayerShip.h"
#include "../../object/gmWaterPlane.h"
#include "../../object/gmIsland.h"
#include "../../spawner/gmIcebergManager.h"
#include "../../collision/gmCollisionSystem.h"
#include "../../effect/gmSpriteAnimRegistry.h"
#include "../../effect/gmVFXManager.h"
#include "../../weapon/gmProjectileManager.h"
#include "../../weapon/gmFlameThrowerManager.h"
#include "../../mesh_ex/gmMeshEX.h"

#include "../../effect/gmRouteVisualizer.h"

#include "../../debugger/gmKyleDebugger.h"

namespace gm
{
    // 前方宣言
    class gmUIManager;

    // ------------------------------------------------------------
    // 本編プレイ中のメインシーン。
    // 船・水面・島・流氷・弾・エフェクト・UIなど、ゲームプレイに関わる
    // オブジェクト一式の生成・毎フレーム更新・描画をここで統括する。
    // ------------------------------------------------------------
    class gmGameScene : public gmSceneBase
    {
    public:
        gmGameScene();
        ~gmGameScene();

        void onEnter(std::shared_ptr<gmSceneManager> manager) override;
        void update() override;
        void draw() override;
        void onExit() override;

    private:
        // クリック位置を海面(y=0)へレイキャストし、命中すれば通常弾を発射する
        void tryFireProjectileOnClick();

        std::shared_ptr<gmGameContext> context_;

        std::shared_ptr<gmKyleDebugger>          debugger_;
        std::shared_ptr<gmPlayerShip>            playerShip_;
        std::shared_ptr<gmWaterPlane>            water_;
        std::vector<std::shared_ptr<gmIsland>>   islands_;
        Shared<dxe::Mesh>                        iceChunk_;
        std::shared_ptr<gmIcebergManager>        icebergManager_;
        std::shared_ptr<gmCollisionSystem>       collisionSystem_;
        std::shared_ptr<gmSpriteAnimRegistry>    spriteAnimRegistry_;
        std::shared_ptr<gmVFXManager>            vfxManager_;
        std::shared_ptr<gmProjectileManager>     projectileManager_;
        std::shared_ptr<gmFlameThrowerManager>   flameThrowerManager_;
        std::unique_ptr<gmUIManager>             uiManager_;
        std::unique_ptr<gmRouteVisualizer>       routeVisualizer_;   // NPC交易船の航路をリボンメッシュで可視化する(判定には関与しない)

        // ---- カメラ操作用 ----
        bool         isDrag_ = false;                   // ドラッグ中かどうか(フリーカメラ操作用)
        float        yaw_ = 0.0f;                       // カメラの水平方向の回転角(ラジアン)
        float        pitch_ = 0.0f;                     // カメラの垂直方向の回転角(ラジアン)
        float        dist_ = 250.0f;                    // 注視点からカメラまでの距離
        tnl::Vector3 camTarget_ = { 0, 0, 0 };          // カメラの注視点
        tnl::Vector3 camOffset_ = { 0, 200, 0 };        // 注視点からのオフセット
    };
}
