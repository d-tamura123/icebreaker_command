#include "gmKyleWorldRuler.h"
#include <DxLib.h>

namespace gm {

    void gmKyleWorldRuler::draw(
        const Shared<dxe::Camera>& camera,
        const tnl::Vector3& origin,
        float unit,
        float length)
    {
        // --- 1. カメラ行列を DXLib にセット ---
        // これを行うことで、DrawLine3D や ConvWorldPosToScreenPos がカメラと連動します
        MATRIX view, proj;
        std::memcpy(view.m, camera->getViewMatrix().m, sizeof(float) * 16);
        std::memcpy(proj.m, camera->getProjectionMatrix().m, sizeof(float) * 16);
        SetCameraViewMatrix(view);
        SetupCamera_ProjectionMatrix(proj);

        // --- 2. ワールド行列を Identity（単位行列）にリセット ---
        // 他の描画物の座標変換を引きずらないようにします
        MATRIX im;
        CreateIdentityMatrix(&im);
        SetTransformToWorld(&im);

        // 目盛りの数
        int steps = static_cast<int>(length / unit);

        // --- 3. 世界軸自体の3D線描画 ---
        // X軸 (赤)
        DrawLine3D(
            VGet(origin.x - length, origin.y, origin.z),
            VGet(origin.x + length, origin.y, origin.z),
            GetColor(255, 0, 0)
        );
        // Y軸 (緑)
        DrawLine3D(
            VGet(origin.x, origin.y - length, origin.z),
            VGet(origin.x, origin.y + length, origin.z),
            GetColor(0, 255, 0)
        );
        // Z軸 (青)
        DrawLine3D(
            VGet(origin.x, origin.y, origin.z - length),
            VGet(origin.x, origin.y, origin.z + length),
            GetColor(0, 0, 255)
        );

        // --- 4. 目盛りと文字ラベルの描画 ---
        // ラベルの描画位置決定にのみ、DxLib標準の座標変換関数を使用する

        // X軸の目盛り
        for (int i = -steps; i <= steps; i++) {
            if (i == 0) continue; // 原点はスキップ

            float x = origin.x + i * unit;
            VECTOR wp = VGet(x, origin.y, origin.z);

            // ① 目盛りのヒゲ（これは3Dで描画）
            DrawLine3D(wp, VGet(x, origin.y + (unit * 0.1f), origin.z), GetColor(255, 0, 0));

            // ② 世界座標を直接DxLibに画面座標へ変換してもらう（変換バグの混入を防ぐ）
            VECTOR screenPos = ConvWorldPosToScreenPos(wp);

            // 画面の裏（カメラより後方）にある場合は描画しない処理
            if (screenPos.z >= 0.0f && screenPos.z <= 1.0f) {
                char buf[32];
                sprintf_s(buf, "%+.0f", i * unit);
                DrawString(static_cast<int>(screenPos.x), static_cast<int>(screenPos.y), buf, GetColor(255, 0, 0));
            }
        }

        // Y軸の目盛り
        for (int i = -steps; i <= steps; i++) {
            if (i == 0) continue;

            float y = origin.y + i * unit;
            VECTOR wp = VGet(origin.x, y, origin.z);

            DrawLine3D(wp, VGet(origin.x + (unit * 0.1f), y, origin.z), GetColor(0, 255, 0));

            VECTOR screenPos = ConvWorldPosToScreenPos(wp);
            if (screenPos.z >= 0.0f && screenPos.z <= 1.0f) {
                char buf[32];
                sprintf_s(buf, "Y:%+.0f", y);
                DrawString(static_cast<int>(screenPos.x + 5), static_cast<int>(screenPos.y), buf, GetColor(0, 255, 0));
            }
        }

        // Z軸の目盛り
        for (int i = -steps; i <= steps; i++) {
            if (i == 0) continue;

            float z = origin.z + i * unit;
            VECTOR wp = VGet(origin.x, origin.y, z);

            DrawLine3D(wp, VGet(origin.x + (unit * 0.1f), origin.y, z), GetColor(0, 0, 255));

            VECTOR screenPos = ConvWorldPosToScreenPos(wp);
            if (screenPos.z >= 0.0f && screenPos.z <= 1.0f) {
                char buf[32];
                sprintf_s(buf, "%+.0f", i * unit);
                DrawString(static_cast<int>(screenPos.x), static_cast<int>(screenPos.y), buf, GetColor(0, 0, 255));
            }
        }






    }

}
