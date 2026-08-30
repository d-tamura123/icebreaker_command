// gmSkybox.h
#pragma once
#include <dxe.h>

namespace gm {

    // ------------------------------------------------------------
    // 背景として、非常に大きな立方体メッシュを表示するスカイボックス。
    // ゆっくり回転させることで、静止した背景よりも空気感を出す。
    // 判定処理には一切関与しない、見た目だけのクラス。
    //
    // Note: フォグを併用する場合、フォグの設定(SetFogEnable等)は必ずこのクラスの
    // render()の"後"に行うこと。スカイボックス自体がフォグの影響を受けて不自然に
    // 霞んでしまうのを防ぐため(呼び出し側であるgmGameScene::draw()を参照)。
    // ------------------------------------------------------------
    class gmSkybox {
    public:
        gmSkybox();

        // 回転の更新に加え、カメラのX/Zへ追従させる(水面のgmWaterPlaneと同じ考え方。
        // マップは原点から離れた範囲に広がっているため、追従させないとカメラがすぐに
        // スカイボックスの外側へ出てしまい、見た目が破綻する)。高さ(Y)は追従させず、
        // メッシュ生成時の中心のままにする。
        void update(float deltaTime, const Shared<dxe::Camera>& camera);

        void render(const Shared<dxe::Camera>& camera);

    private:
        Shared<dxe::Mesh> mesh_;
    };
}
