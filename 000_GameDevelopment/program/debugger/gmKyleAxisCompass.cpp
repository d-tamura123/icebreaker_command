#include "gmKyleAxisCompass.h"
#include <DxLib.h>

namespace gm {

    void gmKyleAxisCompass::draw(const Shared<dxe::Camera>& camera)
    {
        if (!camera) return;

        // カメラの向きベクトル
        tnl::Vector3 f = camera->forward();     // +Z
        tnl::Vector3 r = camera->right();       // +X
        tnl::Vector3 u = camera->getUpper();    // +Y

        // 画面左上に固定表示
        tnl::Vector2f origin = { 80, 80 };
        float scale = 40.0f;

        auto to2D = [&](const tnl::Vector3& v, float s = 1.0f) {
            return tnl::Vector2f(
                origin.x + v.x * scale * s,
                origin.y - v.y * scale * s
            );
            };

        // +方向
        auto xPlus = to2D(r, +1.0f);
        auto yPlus = to2D(u, +1.0f);
        auto zPlus = to2D(f, +1.0f);

        // -方向
        auto xMinus = to2D(r, -1.0f);
        auto yMinus = to2D(u, -1.0f);
        auto zMinus = to2D(f, -1.0f);

        // 軸の描画 (+方向)
        DrawLine(origin.x, origin.y, xPlus.x, xPlus.y, GetColor(255, 0, 0)); // X+
        DrawLine(origin.x, origin.y, yPlus.x, yPlus.y, GetColor(0, 255, 0)); // Y+
        DrawLine(origin.x, origin.y, zPlus.x, zPlus.y, GetColor(0, 0, 255)); // Z+

        // 軸の描画 (-方向)
        DrawLine(origin.x, origin.y, xMinus.x, xMinus.y, GetColor(128, 0, 0)); // X-
        DrawLine(origin.x, origin.y, yMinus.x, yMinus.y, GetColor(0, 128, 0)); // Y-
        DrawLine(origin.x, origin.y, zMinus.x, zMinus.y, GetColor(0, 0, 128)); // Z-

        // ラベル (+方向)
        DrawString(xPlus.x, xPlus.y, "+X", GetColor(255, 0, 0));
        DrawString(yPlus.x, yPlus.y, "+Y", GetColor(0, 255, 0));
        DrawString(zPlus.x, zPlus.y, "+Z", GetColor(0, 0, 255));

        // ラベル (-方向)
        DrawString(xMinus.x, xMinus.y, "-X", GetColor(128, 0, 0));
        DrawString(yMinus.x, yMinus.y, "-Y", GetColor(0, 128, 0));
        DrawString(zMinus.x, zMinus.y, "-Z", GetColor(0, 0, 128));
    }

}
