// gmPlayerCameraController.h
// プレイヤー用カメラの制御(フェーズ2)。
//
// マウスホイールで0.0〜1.0の連続的なズーム値(zoomRatio_)を蓄積し、
// 閾値未満なら「周回モード」、閾値以上なら「エイムモード」として振る舞う。
//   周回モード: プレイヤー船を表示。船を中心に、マウス移動でカメラが周回する。
//   エイムモード: プレイヤー船を非表示。カメラはほぼ船の位置に固定し、
//                マウス移動で狙う方向(yaw/pitch)を変える(画面中央が常に照準)。
//
// 上記の周回/エイムいずれのモードでも、カーソルは非表示にしウィンドウ内へロックした上、
// 毎フレーム画面中央へ強制的に戻す(FPS等でよくある「マウスルック」の実装方式)。
//
// このカーソルのワープは、マウスのX/Y移動量の求め方に直接影響する重要な注意点がある:
// dxe::Inputのマウス移動量(MOUSE_VEL_X/Y)は「前フレームとの絶対座標の差分」で計算されており、
// 内部キャッシュを外から書き換える手段が無い。このキャッシュはgmCursorUtil::SetMousePoint()での
// ワープを検知できないため、そのまま使うと「ワープした分」を実際のマウス移動として誤検知し、
// カメラが毎フレーム震える不具合になる。そのため、X/Y移動量は dxe::Input を使わず、
// gmCursorUtil::GetMousePoint()の座標を自前で1フレーム分保持して差分を取る(lastCursorX_/Y_)。
// マウスホイール(ズーム)はワープの影響を受けないため、引き続きdxe::Inputの値をそのまま使う。
// 
// Altキーを押している間は「カーソルモード」として、カーソルを表示・ロック解除し、
// 
// 入力の低レベルAPIは、既存のgmKyleFreeCameraControllerと同様の考え方(マウスの連続値を
// 直接読む)を踏襲しつつ、こちらはdxe::Inputを使用する(spec上の「連続値はdxe::Inputを
// 直接読む」という方針に合わせるため)。gmInputManagerへは依存しない
// (アクション化していない値を読むだけなので、レイヤー等の恩恵が無いため)。
#pragma once
#include <dxe.h>
#include <memory>

namespace gm {

    // 前方宣言
    class gmPlayerShip;

    class gmPlayerCameraController {
    public:
        gmPlayerCameraController();

        // arg1... 前フレームからの経過秒
        // arg2... 追従対象のプレイヤー船
        // arg3... 更新対象のカメラ
        void update(float dt, const std::shared_ptr<gmPlayerShip>& playerShip, const Shared<dxe::Camera>& camera);

        // true: エイムモード中(プレイヤー船を非表示にすべきタイミング)
        bool isAimMode() const { return zoomRatio_ >= aimThreshold_; }

        // エイムモード時の狙い先(海面上の点。最大射程でクランプ済み)。
        // フェーズ3のHUD(距離表示)・武器発射の目標に使う想定。
        // 周回モード中の値は未定義(呼び出し側でisAimMode()を確認してから使うこと)。
        const tnl::Vector3& getAimTargetWorldPosition() const { return aimTargetWorld_; }

        // プレイヤー船 〜 狙い先までの距離(world単位)。フェーズ3のHUD表示用。
        float getAimTargetDistance() const { return aimTargetDistance_; }

        // ------------------------------------------------------------
        // 撃沈演出(gmShip::updateDestroyed())の見せ場用に、カメラを固定の俯瞰位置へ切り替える。
        // gmPlayerShip::setOnDeathCallback()経由で、HPが0になった瞬間に1回だけ呼ぶ想定。
        //
        // 通常モード(周回/エイム)のupdate()はDestroyed中は呼ばれなくなる
        // (gmGameScene::update()側でガードされる)ため、この関数で設定した位置・向き・画角が
        // 撃沈演出が終わるまでそのまま維持される。
        // (撃沈中、船は水平方向には動かず傾き・沈み込みのみのため、1回だけの設定で十分)
        // ------------------------------------------------------------
        void enterDestroyedShowcase(const std::shared_ptr<gmPlayerShip>& playerShip, const Shared<dxe::Camera>& camera);

    private:
        void updateZoomRatio();
        void updateOrbitMode(const std::shared_ptr<gmPlayerShip>& playerShip, const Shared<dxe::Camera>& camera);
        void updateAimMode(const std::shared_ptr<gmPlayerShip>& playerShip, const Shared<dxe::Camera>& camera);

        Shared<dxe::Input> input_; // マウスの連続値(移動量・ホイール)専用。gmInputManagerとは別インスタンス

        // マウスX/Y移動量の自前計算用。前フレームで(カーソルを中央へ戻した直後に)記録した座標。
        // falseの間は「前回値が無効」を表し、この場合はその回の移動量計算をスキップして
        // 座標の記録だけ行う(初回フレーム、およびカーソルモードから復帰した直後の
        // 大きな誤差混入を防ぐため)。
        bool  hasLastCursorPoint_ = false;
        int   lastCursorX_ = 0;
        int   lastCursorY_ = 0;

        // このフレームで実際に検知したマウスX/Y移動量(updateZoomRatio()の前段でupdate()が計算し、
        // updateOrbitMode()/updateAimMode()がこれを読む)。
        float frameMouseDeltaX_ = 0.0f;
        float frameMouseDeltaY_ = 0.0f;

        // ズーム(0.0=最大ズームアウト 〜 1.0=最大ズームイン)。
        // aimThreshold_未満: 周回モード、以上: エイムモード。
        float zoomRatio_ = 0.0f;
        float aimThreshold_ = 0.0f; // initialize()相当の処理でgmGameConfig.hの値を積む(コンストラクタ参照)

        // 周回モード用の角度(gmKyleFreeCameraControllerと同じ考え方)
        float orbitYaw_ = tnl::PI;   // プレイヤー初期向き(南向き)に合わせた初期値
        float orbitPitch_ = -0.3f;   // 少し見下ろす

        // エイムモード用の角度
        float aimYaw_ = tnl::PI;
        float aimPitch_ = -0.3f;

        tnl::Vector3 aimTargetWorld_;
        float aimTargetDistance_ = 0.0f;
    };

}
