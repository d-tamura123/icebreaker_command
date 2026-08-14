// gmInputLayer.h
// 入力の有効/無効を切り替えるレイヤー。
//
// Note:
// dxe::Input側の「ステージ」機構(ChangeEnableStage()等)は今回使用しない。
// このゲームの規模ではそこまでのレイヤー分けは不要と判断し、gmInputManager自前の
// レイヤー×アクション対応表(layerActions_)だけでフィルタリングする方針とする。
// (将来的にレイヤーが複雑化するようであれば、改めてdxe::Inputのステージ機構の
//  採用を検討する)
#pragma once

namespace gm {

    enum class gmInputLayer {
        Gameplay,   // 通常プレイ中。船操作・武器がすべて有効
        Menu,       // ポーズメニュー等(ToDo: 将来のタスク8用に予約。中身は未実装)
    };

}
