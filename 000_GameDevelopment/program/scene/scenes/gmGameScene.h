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
#include "../../spawner/gmTradeShipManager.h"

#include "../gmFadeTransitionEffect.h"

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

        // プレイヤー撃沈演出完了時のコールバック本体。
        // フェードアウト→(コールバック内で)HP全回復・初期位置へ再配置・カメラリセット→フェードイン、
        // という一連の処理を行う。playerShip_->setOnDestroyedCompleteCallback()に渡す。
        void respawnPlayer();

        // デバッグ専用: NPC交易船の異常系(進捗停滞タイムアウト+ワープ/島衝突時のバック)を
        // O/Pキーで意図的に発生させ、動作確認しやすくする(gmTradeShipのdebug*系メソッド参照)。
        // debugger_->isDebugModeOn()の間だけ有効。リリースビルドの成果物には含めない
        // (呼び出し先のgmTradeShip側のdebug*系メソッドも同様に#ifdef _DEBUGで囲んである)。
#ifdef _DEBUG
        void updateTradeShipDebugHotkeys();
#endif

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
        std::unique_ptr<gmRouteVisualizer>       routeVisualizer_;      // NPC交易船の航路をリボンメッシュで可視化する(判定には関与しない)
        std::shared_ptr<gmTradeShipManager>      tradeShipManager_;     // NPC交易船のスポーンと一元管理
        std::shared_ptr<gmFadeTransitionEffect>  respawnFade_;          // プレイヤー撃沈時の再配置演出専用のフェード
        
        // デバッグ専用(updateTradeShipDebugHotkeys()参照): Oキーでのトグル状態を保持する。
        // 新しくスポーンした交易船にも継続して適用するため、単発のトリガーではなく状態として持つ。
        bool debugTradeShipForcedBadSteering_ = false;

        // ---- カメラ操作用 ----
        bool         isDrag_ = false;                   // ドラッグ中かどうか(フリーカメラ操作用)
        float        yaw_ = 0.0f;                       // カメラの水平方向の回転角(ラジアン)
        float        pitch_ = 0.0f;                     // カメラの垂直方向の回転角(ラジアン)
        float        dist_ = 250.0f;                    // 注視点からカメラまでの距離
        tnl::Vector3 camTarget_ = { 0, 0, 0 };          // カメラの注視点
        tnl::Vector3 camOffset_ = { 0, 200, 0 };        // 注視点からのオフセット
    };
}
