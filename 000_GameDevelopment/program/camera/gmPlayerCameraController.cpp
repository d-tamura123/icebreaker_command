// gmPlayerCameraController.cpp
#include "gmPlayerCameraController.h"
#include "../object/gmPlayerShip.h"
#include "../gmGameConfig.h"
#include "../util/gmCursorUtil.h"
#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>
#include <cmath>

namespace gm {

    gmPlayerCameraController::gmPlayerCameraController()
    {
        aimThreshold_ = CAMERA_ZOOM_AIM_THRESHOLD;
        input_ = dxe::Input::Create();
    }

    void gmPlayerCameraController::update(
        float dt,
        const std::shared_ptr<gmPlayerShip>& playerShip,
        const Shared<dxe::Camera>& camera,
        float weaponMaxRange
    )
    {
        if (!playerShip || !camera || !input_) return;

        // ------------------------------------------------------------
        // カーソルモード(Alt押しっぱなし): カーソルを表示・ウィンドウ内ロックを解除し、
        // カメラの角度・ズームは一切更新せず凍結する(UI操作中にカメラが暴れないように)。
        // ------------------------------------------------------------
        const bool cursorMode = input_->keep(dxe::Input::eButton::KB_LALT) || input_->keep(dxe::Input::eButton::KB_RALT);

        cursorModeActive_ = cursorMode;

        if (cursorMode) {
            dxe::SetVisibleMousePointer(true);
            gmCursorUtil::UnlockCursorFromWindow();

            // カーソルモード中はカーソルが自由に動くため、復帰後の1フレーム目に大きな
            // 誤差(そのフレームの移動量として跳ねてしまう)が乗らないよう基準点を破棄しておく。
            hasLastCursorPoint_ = false;

            return;
        }

        // ------------------------------------------------------------
        // 通常(カメラ捕捉)モード: カーソル非表示+ウィンドウ内ロック。
        // フォーカス喪失(Alt+Tab等)からの復帰対策として毎フレーム再アサートする
        // (Windows側がフォーカス喪失時にロックを自動解除するため)。
        // ------------------------------------------------------------
        dxe::SetVisibleMousePointer(false);
        dxe::LockCursorToWindow();

        // ---- マウスX/Y移動量を自前で計算する ----
        int curX = 0, curY = 0;
        gmCursorUtil::GetMousePoint(curX, curY);

        if (hasLastCursorPoint_) {
            frameMouseDeltaX_ = static_cast<float>(curX - lastCursorX_);
            // Y軸は符号を反転させている: 画面座標は下方向が正(Y増加)だが、
            // 「マウスを画面上へ動かす=視点が(船から)遠ざかる/カメラが遠くを見る」という
            // 2D画面感覚に合わせるため、orbitPitch_/aimPitch_の増減方向を直感と一致させたい。
            frameMouseDeltaY_ = static_cast<float>(lastCursorY_ - curY);
        }
        else {
            // 初回フレーム、またはカーソルモードから復帰した直後: 基準点が無いので移動量は0扱いにする
            frameMouseDeltaX_ = 0.0f;
            frameMouseDeltaY_ = 0.0f;
        }

        // ------------------------------------------------------------
        // カメラ制御をおこなう:
        //   1) ズーム倍率の更新(連続値)
        //   2) 現在のカメラモードを判定(エイム/周回)
        //   3) 周回→エイムへの切り替わりを検知し、直前まで見ていた向きを
        //      エイムモード側の角度へ引き継ぐ(引き継がないと、前回エイムモードを使った時の
        //      古い値、または未使用ならコンストラクタの初期値のまま向きがスナップしてしまう)。
        //      逆方向(エイム→周回)は今回対象外(周回モードのpitch可動範囲がエイムモードより
        //      大幅に狭いため、単純にコピーしてもupdateOrbitMode()側のクランプで意味を失う)。
        //   4) モードごとの角度・位置更新ロジックへ分岐
        // ------------------------------------------------------------
        updateZoomRatio();

        const bool nowAimMode = isAimMode();
        if (nowAimMode && !wasAimMode_) {
            // ズームイン/アウトモードでカメラ(ヨウとピッチ)を同期
            aimYaw_ = orbitYaw_;
            aimPitch_ = orbitPitch_;
        }
        wasAimMode_ = nowAimMode;

        if (nowAimMode) {
            updateAimMode(playerShip, camera, weaponMaxRange);
        }
        else {
            updateOrbitMode(playerShip, camera, weaponMaxRange);
        }

        // 今フレームの移動量読み取りは完了しているので、ここでカーソルを画面中央へ強制的に戻す。
        // (カーソルが画面端に到達すると、それ以上その方向の移動量が読めなくなるための対策)
        const int centerX = static_cast<int>(camera->getScreenWidth() * 0.5f);
        const int centerY = static_cast<int>(camera->getScreenHeight() * 0.5f);
        gmCursorUtil::SetMousePoint(centerX, centerY);

        // 次フレームの基準点は「ワープ後の中央」そのもの。SetMousePoint()が確実に中央へ
        // 移動させているため、わざわざGetMousePoint()で読み直す必要は無い。
        lastCursorX_ = centerX;
        lastCursorY_ = centerY;
        hasLastCursorPoint_ = true;
    }

    // ------------------------------------------------------------
    // マウスホイールの入力を蓄積し、0.0〜1.0にクランプする。
    // ------------------------------------------------------------
    void gmPlayerCameraController::updateZoomRatio()
    {
        const float wheel = input_->getValue(dxe::Input::eVariable::MOUSE_VEL_W);
        if (wheel != 0.0f) {
            // ホイールを手前(下)に回す=ズームアウト、奥(上)=ズームインという一般的な感覚に合わせる。
            zoomRatio_ += (wheel > 0 ? 1.0f : -1.0f) * CAMERA_ZOOM_STEP_PER_WHEEL_NOTCH;
            zoomRatio_ = std::clamp(zoomRatio_, 0.0f, 1.0f);
        }
    }

    // ------------------------------------------------------------
    // 周回モード: プレイヤー船を中心に、マウス移動でカメラが周回する。
    // (gmKyleFreeCameraControllerの回転計算と同じ考え方)
    //
    // 狙い先(=カメラの注視点)は、船から見た水平方向(orbitYaw_)へ、距離をpitch(orbitPitch_)
    // から線形補間で直接決めて置く(高さは常に0=海面上として扱う。tan()等の三角関数を距離の
    // 計算に経由しないため、急角度でも距離が破綻しない)。
    //   急角度(CAMERA_ORBIT_PITCH_MIN側) → 近距離(CAMERA_ORBIT_AIM_DIST_MIN)
    //   水平に近い(CAMERA_ORBIT_PITCH_MAX側) → 遠距離(CAMERA_ORBIT_AIM_DIST_MAX)
    // カメラの位置は船を中心に周回する(pos = 船の位置 - forward × dist)が、注視点は
    // 船そのものではなく狙い先にする。これにより船は自然と画面中央より手前・下寄りに映り、
    // 画面中央に描く照準ドットは狙い先そのものと厳密に一致する。
    // ------------------------------------------------------------
    void gmPlayerCameraController::updateOrbitMode(const std::shared_ptr<gmPlayerShip>& playerShip, const Shared<dxe::Camera>& camera, float weaponMaxRange)
    {
        orbitYaw_ += frameMouseDeltaX_ * CAMERA_ORBIT_MOUSE_SENSITIVITY;
        orbitPitch_ += frameMouseDeltaY_ * CAMERA_ORBIT_MOUSE_SENSITIVITY;
        orbitPitch_ = std::clamp(orbitPitch_, CAMERA_ORBIT_PITCH_MIN, CAMERA_ORBIT_PITCH_MAX);

        // zoomRatio_ = 0でDIST_MAX、閾値でDIST_MINになるよう線形補間
        const float t = std::clamp(zoomRatio_ / CAMERA_ZOOM_AIM_THRESHOLD, 0.0f, 1.0f);
        const float dist = CAMERA_ORBIT_DIST_MAX + (CAMERA_ORBIT_DIST_MIN - CAMERA_ORBIT_DIST_MAX) * t;

        const tnl::Vector3 forward = {
            cosf(orbitPitch_) * sinf(orbitYaw_),
            sinf(orbitPitch_),
            cosf(orbitPitch_) * cosf(orbitYaw_)
        };

        // ---- カメラ位置。船を中心に周回する ----
        tnl::Vector3 pos = playerShip->getPosition() - forward * dist;
        pos.y = std::clamp(pos.y, CAMERA_ORBIT_HEIGHT_MIN, CAMERA_ORBIT_HEIGHT_MAX);

        // ---- 狙い先(発射目標・HUD距離表示・照準ドット・カメラの注視点、全て共通で使う) ----
        // 船から見た水平方向(orbitYaw_のみ。高さは常に0として扱う)
        const tnl::Vector3 horizontalDir = { sinf(orbitYaw_), 0.0f, cosf(orbitYaw_) };

        // pitchをCAMERA_ORBIT_PITCH_MIN〜MAXの範囲で0.0〜1.0に正規化し、
        // CAMERA_ORBIT_AIM_DIST_MIN〜MAXへ線形補間する(武器に依存しない固定範囲)。
        const float pitchRatio = std::clamp(
            (orbitPitch_ - CAMERA_ORBIT_PITCH_MIN) / (CAMERA_ORBIT_PITCH_MAX - CAMERA_ORBIT_PITCH_MIN),
            0.0f, 1.0f);
        const float aimDistance = CAMERA_ORBIT_AIM_DIST_MIN + (CAMERA_ORBIT_AIM_DIST_MAX - CAMERA_ORBIT_AIM_DIST_MIN) * pitchRatio;

        const tnl::Vector3 aimHitPoint = playerShip->getPosition() + horizontalDir * aimDistance;

        // 実際の攻撃対象は、別途武器の最大射程でクランプする(火炎放射等、射程の短い武器では
        // 見た目の狙い先より手前で頭打ちになる。理由はgmGameConfig.hのCAMERA_ORBIT_AIM_DIST_MIN/MAX
        // のコメント参照)。
        aimTargetDistance_ = std::min(aimDistance, weaponMaxRange);
        aimTargetWorld_ = playerShip->getPosition() + horizontalDir * aimTargetDistance_;

        camera->setAngle(tnl::ToRadian(CAMERA_AIM_FOV_WIDE_DEG)); // 周回モードは常に基準画角(エイムモードのFOV_WIDEと同値を流用)
        camera->setPosition(pos);
        camera->setTarget(aimHitPoint); // 注視点=狙い先(船そのものではない)。武器射程による頭打ちの影響は受けない
        camera->update();
    }

    // ------------------------------------------------------------
    // エイムモード: カメラをプレイヤー船の位置(+見張り台程度の高さ)に固定し、
    // マウス移動で狙う方向(yaw/pitch)を変える。画面中央が常に照準(フェーズ3のクロスヘア)。
    // ------------------------------------------------------------
    void gmPlayerCameraController::updateAimMode(const std::shared_ptr<gmPlayerShip>& playerShip, const Shared<dxe::Camera>& camera, float weaponMaxRange)
    {
        aimYaw_ += frameMouseDeltaX_ * CAMERA_AIM_MOUSE_SENSITIVITY;
        aimPitch_ += frameMouseDeltaY_ * CAMERA_AIM_MOUSE_SENSITIVITY;
        
        // ------------------------------------------------------------
        // pitch(見下ろし角)の上限(水平に近い側の限界)を、選択中武器の最大射程から逆算する。
        //
        // カメラの高さ(CAMERA_AIM_HEIGHT_OFFSET)から角度pitchだけ見下ろした視線が、
        // 海面(y=0)と交わるまでの水平距離は、直角三角形(高さ=対辺、距離=隣辺)の関係から
        //   距離 = 高さ ÷ tan(見下ろし角)
        // で求まる。この「距離」がちょうど最大射程(weaponMaxRange)と一致するpitch角度を、
        // 上式をpitchについて解いて求める:
        //   高さ ÷ tan(pitch_限界) = 最大射程
        //   ⇔ tan(pitch_限界) = 高さ ÷ 最大射程
        //   ⇔ pitch_限界 = atan(高さ ÷ 最大射程)
        // ただしこのゲームの座標系ではpitchは「見下ろす向き」が負値なので、符号を反転させて
        //   pitch_限界 = -atan(高さ ÷ 最大射程)
        // となる。この角度より水平に近づける(=pitchを0に近づける)と、最大射程より先を
        // 見てしまうことになるため、この値をpitchの新たな上限として使う。
        //
        // 既存の固定値CAMERA_AIM_PITCH_MAX(水平ギリギリを避けるための安全マージン用)より
        // 必ず制限が厳しくなるとは限らないため、両者のうちより制限が厳しい方(より水平から
        // 遠い=より負側の値)を採用する。
        // ------------------------------------------------------------
        const float pitchLimitForMaxRange = -atanf(CAMERA_AIM_HEIGHT_OFFSET / weaponMaxRange);
        const float effectivePitchMax = std::min(CAMERA_AIM_PITCH_MAX, pitchLimitForMaxRange);
        aimPitch_ = std::clamp(aimPitch_, CAMERA_AIM_PITCH_MIN, effectivePitchMax);


        const tnl::Vector3 rayOrigin = playerShip->getPosition() + tnl::Vector3(0.0f, CAMERA_AIM_HEIGHT_OFFSET, 0.0f);
        const tnl::Vector3 rayDir = {
            cosf(aimPitch_) * sinf(aimYaw_),
            sinf(aimPitch_),
            cosf(aimPitch_) * cosf(aimYaw_)
        };

        // 海面(y=0)との交点。pitchを上でeffectivePitchMaxにより僅かに水平未満に抑えているため、
        // rayDir.yがちょうど0になることはない想定(0除算は起きない)。
        const float t = (0.0f - rayOrigin.y) / rayDir.y;
        tnl::Vector3 lookTarget = rayOrigin + rayDir * t;

        // ---- 狙い先(発射目標・HUD距離表示用) ----
        // 上のpitchクランプにより、この時点でdistanceは理論上ほぼ必ずweaponMaxRange以下に
        // 収まっているはずだが、浮動小数点誤差の保険として引き続きクランプしておく。
        tnl::Vector3 toTarget = lookTarget - playerShip->getPosition();
        float distance = toTarget.length();
        if (distance > weaponMaxRange) {
            toTarget = toTarget * (weaponMaxRange / distance);
            distance = weaponMaxRange;
        }
        aimTargetWorld_ = playerShip->getPosition() + toTarget;
        aimTargetDistance_ = distance;

        // ---- ズーム値(閾値〜1.0)に応じてFOVを狭める ----
        const float t2 = std::clamp((zoomRatio_ - CAMERA_ZOOM_AIM_THRESHOLD) / (1.0f - CAMERA_ZOOM_AIM_THRESHOLD), 0.0f, 1.0f);
        const float fovDeg = CAMERA_AIM_FOV_WIDE_DEG + (CAMERA_AIM_FOV_NARROW_DEG - CAMERA_AIM_FOV_WIDE_DEG) * t2;

        camera->setAngle(tnl::ToRadian(fovDeg));
        camera->setPosition(rayOrigin);
        camera->setTarget(lookTarget); // カメラの向き自体はクランプ前の生の照準先(画面中央=照準の見た目を優先)
        camera->update();
    }

    // ------------------------------------------------------------
    // 撃沈演出用の固定俯瞰カメラ。onEnter()時の初期カメラと同じ考え方(船の後方・上方から見る)で、
    // 死亡した瞬間のプレイヤー船の向き(getForward())を基準に1回だけ位置・向きを計算する。
    //
    // エイムモード中に死亡していた場合(船が非表示・カメラが船の位置に張り付いた狭い画角)でも、
    // ここで画角を通常値に戻すため、沈む姿がちゃんと見える構図に切り替わる。
    // ------------------------------------------------------------
    void gmPlayerCameraController::enterDestroyedShowcase(const std::shared_ptr<gmPlayerShip>& playerShip, const Shared<dxe::Camera>& camera)
    {
        if (!playerShip || !camera) return;

        const tnl::Vector3 shipForward = playerShip->getForward();
        const tnl::Vector3 shipPos = playerShip->getPosition();

        const tnl::Vector3 camPos = shipPos - shipForward * 250.0f + tnl::Vector3(0.0f, 100.0f, 0.0f);

        camera->setAngle(tnl::ToRadian(CAMERA_AIM_FOV_WIDE_DEG)); // エイム中の狭い画角のまま死亡した場合の保険
        camera->setPosition(camPos);
        camera->setTarget(shipPos);
        camera->update();
    }
}
