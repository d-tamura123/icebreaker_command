// gmGameScene.h
#pragma once
#include "../gmSceneBase.h"
#include "../../gmGameContext.h"

#include "../../sound/gmSoundManager.h"

#include "../../camera/gmPlayerCameraController.h"
#include "../../object/gmPlayerShip.h"
#include "../../object/gmWaterPlane.h"
#include "../../object/gmIsland.h"
#include "../../spawner/gmIcebergManager.h"
#include "../../collision/gmCollisionSystem.h"
#include "../../effect/gmSpriteAnimRegistry.h"
#include "../../effect/gmVFXManager.h"
#include "../../weapon/gmProjectileManager.h"
#include "../../weapon/gmFlameThrowerManager.h"
#include "../../weapon/gmWeaponSelectionState.h"

#include "../../mesh_ex/gmMeshEX.h"
#include "../../util/gmCursorUtil.h"

#include "../../effect/gmRouteVisualizer.h"
#include "../../effect/gmMapBoundaryVisualizer.h"
#include "../../spawner/gmTradeShipManager.h"

#include "../gmFadeTransitionEffect.h"

#include "../../debugger/gmKyleDebugger.h"

namespace gm
{
    // 前方宣言
    class gmUIManager;
    class gmGameStopUIManager;

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

        // 1/2/3キーでの武器切替、5キーでのリカバリ発動を処理する
        void updateWeaponSelectionInput();

        // リカバリ(5キー、または武器選択HUDのリカバリボタンのクリック)発動処理の本体。
        // クールダウン判定→HPへの適用→クールダウン開始、を行う。
        void tryUseRecovery();

        // プレイヤー撃沈演出完了時のコールバック本体。
        // フェードアウト→(コールバック内で)HP全回復・初期位置へ再配置・カメラリセット→フェードイン、
        // という一連の処理を行う。playerShip_->setOnDestroyedCompleteCallback()に渡す。
        void respawnPlayer();

        // Escキーでのポーズメニュー開閉を処理する。一時停止中かどうかに関わらず毎フレーム呼ぶ
        // (開く/閉じる、どちらの操作もここで処理するため)。
        void updateSystemInput();

        // デバッグ専用: NPC交易船の異常系(進捗停滞タイムアウト+ワープ/島衝突時のバック)を
        // O/Pキーで意図的に発生させ、動作確認しやすくする(gmTradeShipのdebug*系メソッド参照)。
        // debugger_->isDebugModeOn()の間だけ有効。リリースビルドの成果物には含めない
        // (呼び出し先のgmTradeShip側のdebug*系メソッドも同様に#ifdef _DEBUGで囲んである)。
#ifdef _DEBUG
        void updateTradeShipDebugHotkeys();
#endif

        std::shared_ptr<gmGameContext> context_;

        gmSoundManager soundManager_;

        std::shared_ptr<gmKyleDebugger>          debugger_;
        std::unique_ptr<gmPlayerCameraController> cameraController_;
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
        std::shared_ptr<gmWeaponSelectionState>  weaponSelection_;          // 武器選択・リキャスト・リカバリのクールダウン状態(フェーズ1.4)
        std::unique_ptr<gmUIManager>             uiManager_;
        std::unique_ptr<gmGameStopUIManager>     gameStopUIManager_;        // ポーズメニュー等、ゲームを止めて表示するUIの中間管理層
        std::unique_ptr<gmRouteVisualizer>       routeVisualizer_;          // NPC交易船の航路をリボンメッシュで可視化する(判定には関与しない)
        std::unique_ptr<gmMapBoundaryVisualizer> mapBoundaryVisualizer_;    // マップの外枠をリボンメッシュで可視化する(判定には関与しない。移動抑止はgmPlayerShip側)
        std::shared_ptr<gmTradeShipManager>      tradeShipManager_;         // NPC交易船のスポーンと一元管理
        std::shared_ptr<gmFadeTransitionEffect>  respawnFade_;              // プレイヤー撃沈時の再配置演出専用のフェード
        

        bool bgmStartedAfterFade_ = false; // シーン切替の暗転フェードが明けた直後にBGMを鳴らすためのワンショットフラグ

        // デバッグ専用(updateTradeShipDebugHotkeys()参照): Oキーでのトグル状態を保持する。
        // 新しくスポーンした交易船にも継続して適用するため、単発のトリガーではなく状態として持つ。
        bool debugTradeShipForcedBadSteering_ = false;
        
    };
}
