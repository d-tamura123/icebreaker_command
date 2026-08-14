// gmAction.h
// 入力アクション(意味のある操作単位)の一覧。
//
// キーそのもの(KB_W等)ではなく「船の速度を上げる」のような意味単位で定義することで、
// キー割り当てが変わっても呼び出し側(gmPlayerShip等)のコードは変更不要にする。
// 実際のキー/ボタンとの対応は gmInputManager::initializeDefaultBindings() 側で持つ。
#pragma once

namespace gm {

    enum class gmAction {
        // ---- 船操作 ----
        Ship_SpeedUp,           // 速度段階を1つ上げる(trigger)
        Ship_SpeedDown,         // 速度段階を1つ下げる(trigger)
        Ship_RudderLeftHold,    // 押している間、左舵いっぱい(held)
        Ship_RudderRightHold,   // 押している間、右舵いっぱい(held)
        Ship_RudderLeftStep,    // 舵角段階を1つ左へ(trigger)
        Ship_RudderRightStep,   // 舵角段階を1つ右へ(trigger)
        Ship_Recovery,          // HPリカバリー発動(trigger)

        // ---- 武器 ----
        Weapon_Switch1,         // 溶かす弾へ切り替え(trigger)
        Weapon_Switch2,         // 割る弾へ切り替え(trigger)
        Weapon_Switch3,         // 火炎放射へ切り替え(trigger)
        Weapon_Fire,            // 現在選択中の武器を発射(trigger)

        // Note: カメラのズーム(マウスホイール)・周回/エイム時のマウス移動量は、
        // トリガー/ホールドの離散判定になじまない連続値のため、
        // gmActionの枠組みには含めない(dxe::Input::getValue()を直接読む)。
    };

}
