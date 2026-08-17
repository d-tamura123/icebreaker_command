// gmInputCallerId.h
// gmInputManager::consumePress()の呼び出し元を一意に識別するID。
//
// 同じgmActionを複数箇所からconsumePress()する際、それぞれが独立した消費フラグを
// 持てるようにするためのキー(gmAction単体だけをキーにすると、呼び出し元ごとの
// 消費状態が共有されてしまい、片方だけ常にfalseが返り続ける不具合につながるため)。
//
// 命名規則: 
// 　「呼び出し元のクラス名_役割」で分かる名前にする
// 　(例: PlayerShip_SpeedUp、GameScene_WeaponFire 等)。
// 　以後、新しい呼び出し元(consumePress()の新規呼び出し箇所)が増えるたびにここへ追記する。
#pragma once

namespace gm {

    enum class gmInputCallerId {
        PlayerShip_SpeedUp,
        PlayerShip_SpeedDown,
        PlayerShip_RudderLeftStep,
        PlayerShip_RudderRightStep,

        GameScene_WeaponFire,
        GameScene_WeaponSwitch1,
        GameScene_WeaponSwitch2,
        GameScene_WeaponSwitch3,
        GameScene_Recovery,
        GameScene_TogglePauseMenu,
    };

}
