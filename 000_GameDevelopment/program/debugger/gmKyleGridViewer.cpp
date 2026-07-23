#include "gmKyleGridViewer.h"
#include "../gmGameConfig.h"
#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>

namespace gm {

    void gmKyleGridViewer::render(const Shared<dxe::Camera>& camera)
    {
        drawGrid(camera);
    }

    void gmKyleGridViewer::drawGrid(const Shared<dxe::Camera>& camera)
    {
        // --- 基準座標 ---
        const float gridY = 0.0f; // グリッドの高さは常にゼロとする

        // --- カメラ位置 ---
        const float cx = camera->getPosition().x;
        const float cz = camera->getPosition().z;

        // --- カメラ行列を DXLib にセット ---
        MATRIX view, proj;
        memcpy(view.m, camera->getViewMatrix().m, sizeof(float) * 16);
        memcpy(proj.m, camera->getProjectionMatrix().m, sizeof(float) * 16);
        SetCameraViewMatrix(view);
        SetupCamera_ProjectionMatrix(proj);

        // --- ワールド行列を Identity に ---
        MATRIX im;
        CreateIdentityMatrix(&im);
        SetTransformToWorld(&im);

        // --- マス数を偶数に揃える（中央線を作るため） ---
        int rn = (1 == CELL_COUNT % 2) ? CELL_COUNT + 1 : CELL_COUNT;

        float l = CELL_SIZE * rn * 0.5f;  // グリッドの半径

        // --- 描画範囲（カメラ周囲の矩形範囲を計算） ---
        const float VIEW_RADIUS = CELL_SIZE * 20.0f; // 適宜調整

        // カメラ周辺の描画ミニマム・マックス（グリッド全体のサイズ [-l, l] でクランプ）
        //float minX = (cx - VIEW_RADIUS < -l) ? -l : cx - VIEW_RADIUS;
        //float maxX = (cx + VIEW_RADIUS > l) ? l : cx + VIEW_RADIUS;
        //float minZ = (cz - VIEW_RADIUS < -l) ? -l : cz - VIEW_RADIUS;
        //float maxZ = (cz + VIEW_RADIUS > l) ? l : cz + VIEW_RADIUS;
        float minX = std::max(cx - VIEW_RADIUS, -l);
        float maxX = std::min(cx + VIEW_RADIUS,  l);
        float minZ = std::max(cz - VIEW_RADIUS, -l);
        float maxZ = std::min(cz + VIEW_RADIUS,  l);
        
        // --- 1. 縦線（Z軸方向の線：X座標をループ） ---
        for (int i = 0; i < rn + 1; ++i) {
            float gx = -l + i * CELL_SIZE;

            // カメラのX範囲外なら描画しない
            if (gx < minX || gx > maxX) continue;

            // 線の端点を Zの描画範囲（minZ 〜 maxZ）に制限する
            VECTOR a1 = VGet(gx, gridY, maxZ);
            VECTOR a2 = VGet(gx, gridY, minZ);

            if ((rn >> 1) == i) {
                DrawLine3D(a1, a2, 0xff0000ff); // 中央線（青）
            }
            else {
                DrawLine3D(a1, a2, GRID_COLOR);
            }
        }

        // --- 2. 横線（X軸方向の線：Z座標をループ） ---
        for (int i = 0; i < rn + 1; ++i) {
            float gz = -l + i * CELL_SIZE;

            // カメラのZ範囲外なら描画しない
            if (gz < minZ || gz > maxZ) continue;

            // X方向の線を描画範囲でクリップ
            // 線の端点を Xの描画範囲（minX 〜 maxX）に制限する
            VECTOR b1 = VGet(maxX, gridY, gz);
            VECTOR b2 = VGet(minX, gridY, gz);

            if ((rn >> 1) == i) {
                DrawLine3D(b1, b2, 0xffff0000); // 中央線（赤）
            }
            else {
                DrawLine3D(b1, b2, GRID_COLOR);
            }
        }

        // --- Y軸（緑） ---
        // Y軸はカメラ位置の直下に描画すると見やすいため原点(0,0)に固定
        DrawLine3D(
            VGet(0, gridY + l, 0),
            VGet(0, gridY - l, 0),
            0xff00ff00
        );
    }

}
