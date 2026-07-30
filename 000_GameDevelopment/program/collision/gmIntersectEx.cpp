#include "gmIntersectEx.h"
#include <dxe.h>
#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>
#include <cmath>

namespace gm {

    //----------------------------------------------------------------------------------------------
    // OBB(直方体・立方体) 同士の衝突判定 ※分離軸定理(SAT)
    //
    // 実装の流れ:
    //   1. AとBそれぞれの「向き」を表す3本の軸ベクトル(ローカルX/Y/Z軸)を、
    //      回転(クォータニオン)から求める。
    //   2. AとBの中心を結ぶベクトル(2つの中心の距離と方向)を求める。
    //   3. 15本の候補軸それぞれについて、
    //        ・Aをその軸に投影した「半分の幅」
    //        ・Bをその軸に投影した「半分の幅」
    //        ・中心間ベクトルをその軸に投影した長さ
    //      を計算し、「中心間の投影長 > Aの半分の幅 + Bの半分の幅」であれば、
    //      その軸で2つの影が離れている＝分離軸が見つかった＝衝突していない、と判定する。
    //   4. 15本すべて調べて分離軸が1本も見つからなければ、衝突していると判定する。
    //----------------------------------------------------------------------------------------------
    bool IsIntersectOBB(const tnl::Vector3& a_pos, const tnl::Vector3& a_size, const tnl::Quaternion& a_rot,
        const tnl::Vector3& b_pos, const tnl::Vector3& b_size, const tnl::Quaternion& b_rot)
    {
        // ------------------------------------------------------------
        // 1. それぞれのローカルXYZ軸(ワールド空間での向き)を求める。
        //    回転していない状態のX軸(1,0,0)・Y軸(0,1,0)・Z軸(0,0,1)を、
        //    そのOBBの回転で回してやれば、実際に向いている方向が分かる。
        // ------------------------------------------------------------
        tnl::Vector3 a_axis[3] = {
            tnl::Vector3::TransformCoord(tnl::Vector3(1, 0, 0), a_rot),
            tnl::Vector3::TransformCoord(tnl::Vector3(0, 1, 0), a_rot),
            tnl::Vector3::TransformCoord(tnl::Vector3(0, 0, 1), a_rot),
        };
        tnl::Vector3 b_axis[3] = {
            tnl::Vector3::TransformCoord(tnl::Vector3(1, 0, 0), b_rot),
            tnl::Vector3::TransformCoord(tnl::Vector3(0, 1, 0), b_rot),
            tnl::Vector3::TransformCoord(tnl::Vector3(0, 0, 1), b_rot),
        };

        // サイズは「全長」で渡ってくる規約なので、半分にして中心からの半幅にしておく
        float a_half[3] = { a_size.x * 0.5f, a_size.y * 0.5f, a_size.z * 0.5f };
        float b_half[3] = { b_size.x * 0.5f, b_size.y * 0.5f, b_size.z * 0.5f };

        // 2つの中心を結ぶベクトル
        tnl::Vector3 centerDiff = b_pos - a_pos;

        // ------------------------------------------------------------
        // 2. 判定に使う15本の候補軸を1本ずつ試す。
        //    候補軸ごとに、以下の3つを比較する:
        //      ・A自身をこの軸に投影したときの半分の幅(projA)
        //      ・B自身をこの軸に投影したときの半分の幅(projB)
        //      ・中心間ベクトルをこの軸に投影した長さ(projDist、絶対値)
        //    projDist > projA + projB なら、この軸方向には隙間がある
        //    ＝分離軸が見つかった＝衝突していない、で即falseを返す。
        // ------------------------------------------------------------
        auto testAxis = [&](const tnl::Vector3& axisRaw) -> bool
            {
                // 軸の長さがほぼ0の場合(2つの辺がほぼ平行なときに外積で発生しうる)は、
                // その軸では判定できないのでスキップ扱いにする(=分離軸ではないとみなす)
                float lenSq = tnl::Vector3::Dot(axisRaw, axisRaw);
                if (lenSq < 1e-8f) {
                    return true; // この軸はスキップ(=分離していないものとして扱う)
                }
                tnl::Vector3 axis = axisRaw; // 正規化しなくても両辺に同じ倍率がかかるだけなので比較には影響しない

                // Aをこの軸に投影した半分の幅
                //   Aの3つの辺ベクトル(半幅×軸)それぞれを、判定軸に投影して絶対値の合計を取る
                float projA =
                    std::abs(tnl::Vector3::Dot(a_axis[0], axis)) * a_half[0] +
                    std::abs(tnl::Vector3::Dot(a_axis[1], axis)) * a_half[1] +
                    std::abs(tnl::Vector3::Dot(a_axis[2], axis)) * a_half[2];

                // Bをこの軸に投影した半分の幅も同様に求める
                float projB =
                    std::abs(tnl::Vector3::Dot(b_axis[0], axis)) * b_half[0] +
                    std::abs(tnl::Vector3::Dot(b_axis[1], axis)) * b_half[1] +
                    std::abs(tnl::Vector3::Dot(b_axis[2], axis)) * b_half[2];

                // 中心間ベクトルをこの軸に投影した長さ(符号は問わないので絶対値)
                float projDist = std::abs(tnl::Vector3::Dot(centerDiff, axis));

                // 隙間があれば(=この軸が分離軸なら) false を返して「衝突していない」と伝える
                return projDist <= (projA + projB);
            };

        // --- 候補軸 (1)〜(3): Aの3つの面の法線方向 ---
        for (int i = 0; i < 3; ++i) {
            if (!testAxis(a_axis[i])) return false;
        }

        // --- 候補軸 (4)〜(6): Bの3つの面の法線方向 ---
        for (int i = 0; i < 3; ++i) {
            if (!testAxis(b_axis[i])) return false;
        }

        // --- 候補軸 (7)〜(15): Aの辺の向きとBの辺の向きの外積、全組み合わせ(3×3=9本) ---
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                tnl::Vector3 axis = tnl::Vector3::Cross(a_axis[i], b_axis[j]);
                if (!testAxis(axis)) return false;
            }
        }

        // 15本すべてで分離軸が見つからなかった＝重なっている方向しかない＝衝突している
        return true;
    }

    //----------------------------------------------------------------------------------------------
    // カプセル同士の衝突判定
    //
    // 実装の流れ(2本の線分の最近接点を求める。):
    //   1. 線分Aをパラメータs(0〜1)、線分Bをパラメータt(0〜1)で表す。
    //      A上の点は a_st + s * (a_en - a_st)、B上の点は b_st + t * (b_en - b_st)。
    //   2. 「2点間の距離が最小になるs, t」を、連立方程式を解いて求める。
    //      (これは高校数学の「点と直線の距離」を3次元・線分同士に拡張したもの)
    //   3. 求めたs, tが0〜1の範囲に収まっていない場合(=最も近い点が線分の"外"にはみ出す場合)は、
    //      該当する端点に固定してから、もう片方の線分に対して再度最も近い点を求め直す。
    //      (これを線分の「クランプ処理」と呼ぶ)
    //   4. 最終的に求まった2点間の距離と、半径Aと半径Bの和を比較する。
    //----------------------------------------------------------------------------------------------
    bool IsIntersectCapsuleCapsule(const tnl::Vector3& a_st, const tnl::Vector3& a_en, float a_r,
        const tnl::Vector3& b_st, const tnl::Vector3& b_en, float b_r,
        float* a_t, float* b_t)
    {
        // d1: 線分Aの方向ベクトル(始点→終点)
        // d2: 線分Bの方向ベクトル(始点→終点)
        // r : 線分Aの始点から線分Bの始点へのベクトル
        tnl::Vector3 d1 = a_en - a_st;
        tnl::Vector3 d2 = b_en - b_st;
        tnl::Vector3 r = a_st - b_st;

        float a = tnl::Vector3::Dot(d1, d1); // 線分Aの長さの2乗
        float e = tnl::Vector3::Dot(d2, d2); // 線分Bの長さの2乗
        float f = tnl::Vector3::Dot(d2, r);

        float s = 0.0f; // 線分A上の位置(0〜1)
        float t = 0.0f; // 線分B上の位置(0〜1)

        const float EPS = 1e-8f;

        // ------------------------------------------------------------
        // 特殊ケース: 両方とも「線分」ではなく「点」に近い(長さがほぼ0)場合、
        // 最近接点の計算をせずに、そのまま始点同士を比較すればよい。
        // ------------------------------------------------------------
        if (a <= EPS && e <= EPS) {
            s = 0.0f;
            t = 0.0f;
        }
        else if (a <= EPS) {
            // 線分Aが点の場合: 線分B上でAの始点に最も近い点だけを求める
            s = 0.0f;
            t = std::clamp(f / e, 0.0f, 1.0f);
        }
        else {
            float c = tnl::Vector3::Dot(d1, r);

            if (e <= EPS) {
                // 線分Bが点の場合: 線分A上でBの始点に最も近い点だけを求める
                t = 0.0f;
                s = std::clamp(-c / a, 0.0f, 1.0f);
            }
            else {
                // ------------------------------------------------------------
                // 一般ケース: 両方とも実際に長さのある線分。
                // 「2直線(無限に伸ばした場合)の最近接点」をまず求める。
                //   b = d1・d2
                //   分母 = a*e - b*b (2つの方向ベクトルが張る平行四辺形の面積の2乗に相当)
                //   分母がほぼ0 = 2本の線分がほぼ平行、というケース
                // ------------------------------------------------------------
                float b = tnl::Vector3::Dot(d1, d2);
                float denom = a * e - b * b;

                if (denom > EPS) {
                    s = std::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
                }
                else {
                    // ほぼ平行な場合は、s=0を基準にしてしまってよい
                    // (どちらを基準にしても最終的な最短距離はほぼ変わらないため)
                    s = 0.0f;
                }

                // sが決まったので、それに対応するtを求める
                t = (b * s + f) / e;

                // ------------------------------------------------------------
                // tが0〜1の範囲からはみ出した場合、tを範囲内にクランプしたうえで、
                // そのtに対応する最適なsを求め直す(クランプのやり直し)。
                // これをしないと、線分の"端点同士が最も近い"ケースで誤差が出る。
                // ------------------------------------------------------------
                if (t < 0.0f) {
                    t = 0.0f;
                    s = std::clamp(-c / a, 0.0f, 1.0f);
                }
                else if (t > 1.0f) {
                    t = 1.0f;
                    s = std::clamp((b - c) / a, 0.0f, 1.0f);
                }
            }
        }

        // s, t から実際の最近接点を求め、その距離を計算する
        tnl::Vector3 closestA = a_st + d1 * s;
        tnl::Vector3 closestB = b_st + d2 * t;
        float dist = (closestA - closestB).length();

        if (a_t) *a_t = s;
        if (b_t) *b_t = t;

        // 最近接点同士の距離が、半径の合計以下なら衝突している
        return dist <= (a_r + b_r);
    }

    //----------------------------------------------------------------------------------------------
    // 楕円球の「ある方向を向いたときの半径」を求める
    //
    // Note:
    // 楕円球の方程式は (x/rx)^2 + (y/ry)^2 + (z/rz)^2 = 1 で表される
    // (rx, ry, rz はそれぞれの軸の半径)。
    // 中心から方向ベクトル u = (ux, uy, uz)(正規化済み)に向かって進んだとき、
    // 表面に当たる距離を R とすると、表面の点は (R*ux, R*uy, R*uz) になるので、
    // これを楕円球の方程式に代入すると
    //     (R*ux/rx)^2 + (R*uy/ry)^2 + (R*uz/rz)^2 = 1
    //   → R^2 * ( (ux/rx)^2 + (uy/ry)^2 + (uz/rz)^2 ) = 1
    //   → R = 1 / sqrt( (ux/rx)^2 + (uy/ry)^2 + (uz/rz)^2 )
    // という単純な式に落ち着く。これがこの関数の中身。
    //----------------------------------------------------------------------------------------------
    float GetEllipsoidRadiusInDirection(const tnl::Vector3& radii, const tnl::Vector3& dir)
    {
        tnl::Vector3 u = tnl::Vector3::Normalize(dir);

        // 半径が0(潰れている軸)による0除算を避けるための下限値
        const float MIN_RADIUS = 1e-4f;
        float rx = std::max(radii.x, MIN_RADIUS);
        float ry = std::max(radii.y, MIN_RADIUS);
        float rz = std::max(radii.z, MIN_RADIUS);

        float denom =
            (u.x / rx) * (u.x / rx) +
            (u.y / ry) * (u.y / ry) +
            (u.z / rz) * (u.z / rz);

        if (denom < 1e-8f) {
            // 方向ベクトルがほぼゼロ等、異常な入力に対する保険
            return (rx + ry + rz) / 3.0f;
        }

        return 1.0f / std::sqrt(denom);
    }

    //----------------------------------------------------------------------------------------------
    // 楕円球と球の衝突判定(近似)
    //
    // 近似の考え方:
    // 楕円球の中心から見て「球のある方向」を求め、その方向における楕円球の半径(＝上のヘルパー関数)
    // を計算する。この半径を使えば、"その方向に限っては" 楕円球を1つの球として扱ってよい。
    // そこで「楕円球中心を中心とする半径Rの球」と「相手の球」の、通常の球-球判定に置き換える。
    //
    // 精度についての注意:
    // これは正確な楕円球同士の交差判定ではなく近似。楕円球が細長いほど、
    // 中心を結ぶ直線からズレた位置での接触判定に誤差が出やすい。
    // ただしゲームの当たり判定としては十分実用的な精度になる。
    //----------------------------------------------------------------------------------------------
    bool IsIntersectEllipsoidSphere(const tnl::Vector3& ellip_pos, const tnl::Vector3& ellip_radii, const tnl::Quaternion& ellip_rot,
        const tnl::Vector3& sphere_pos, float sphere_r)
    {
        // 相手方向をワールド座標で求めたのち、楕円球のローカル座標系(回転を打ち消した状態)に変換する
        tnl::Vector3 dirWorld = sphere_pos - ellip_pos;
        tnl::Vector3 dirLocal = tnl::Vector3::InverseTransformCoord(dirWorld, ellip_rot);

        float ellipRadiusThisDir = GetEllipsoidRadiusInDirection(ellip_radii, dirLocal);

        // あとは通常の球-球判定に委譲するだけ
        return tnl::IsIntersectSphere(ellip_pos, ellipRadiusThisDir, sphere_pos, sphere_r);
    }

    //----------------------------------------------------------------------------------------------
    // 楕円球とOBBの衝突判定(近似)
    // tips... OBBの中心方向で楕円球を球に近似し、既存のSphere-OBB判定へ委譲する
    //----------------------------------------------------------------------------------------------
    bool IsIntersectEllipsoidOBB(const tnl::Vector3& ellip_pos, const tnl::Vector3& ellip_radii, const tnl::Quaternion& ellip_rot,
        const tnl::Vector3& obb_pos, const tnl::Vector3& obb_size, const tnl::Quaternion& obb_rot)
    {
        tnl::Vector3 dirWorld = obb_pos - ellip_pos;
        tnl::Vector3 dirLocal = tnl::Vector3::InverseTransformCoord(dirWorld, ellip_rot);

        float ellipRadiusThisDir = GetEllipsoidRadiusInDirection(ellip_radii, dirLocal);

        return tnl::IsIntersectSphereOBB(ellip_pos, ellipRadiusThisDir, obb_pos, obb_size, obb_rot);
    }

    //----------------------------------------------------------------------------------------------
    // 楕円球とカプセルの衝突判定(近似)
    // tips... カプセルの芯線分上で楕円球中心に最も近い点を先に求め、その方向で楕円球を球に近似する
    //----------------------------------------------------------------------------------------------
    bool IsIntersectEllipsoidCapsule(const tnl::Vector3& ellip_pos, const tnl::Vector3& ellip_radii, const tnl::Quaternion& ellip_rot,
        const tnl::Vector3& cap_st, const tnl::Vector3& cap_en, float cap_r)
    {
        // カプセルの芯線分上で、楕円球の中心に最も近い点を求める
        // (線分ABの上でPに最も近い点を求める、基本的な公式)
        tnl::Vector3 capDir = cap_en - cap_st;
        float capLenSq = tnl::Vector3::Dot(capDir, capDir);

        tnl::Vector3 closestOnCapsule;
        if (capLenSq < 1e-8f) {
            // カプセルがほぼ点の場合
            closestOnCapsule = cap_st;
        }
        else {
            float t = tnl::Vector3::Dot(ellip_pos - cap_st, capDir) / capLenSq;
            t = std::clamp(t, 0.0f, 1.0f);
            closestOnCapsule = cap_st + capDir * t;
        }

        tnl::Vector3 dirWorld = closestOnCapsule - ellip_pos;
        tnl::Vector3 dirLocal = tnl::Vector3::InverseTransformCoord(dirWorld, ellip_rot);

        float ellipRadiusThisDir = GetEllipsoidRadiusInDirection(ellip_radii, dirLocal);

        // 楕円球を球とみなせたので、あとは「球の中心から線分までの距離」を求めて半径と比較すればよい
        float distToLine = (closestOnCapsule - ellip_pos).length();
        return distToLine <= (ellipRadiusThisDir + cap_r);
    }

    //----------------------------------------------------------------------------------------------
    // 楕円球同士の衝突判定(近似)
    // tips... 双方とも「相手中心への方向」で自分自身を球に近似し、球-球判定へ委譲する
    //----------------------------------------------------------------------------------------------
    bool IsIntersectEllipsoidEllipsoid(const tnl::Vector3& a_pos, const tnl::Vector3& a_radii, const tnl::Quaternion& a_rot,
        const tnl::Vector3& b_pos, const tnl::Vector3& b_radii, const tnl::Quaternion& b_rot)
    {
        // Aから見たBの方向で、Aの半径を近似する
        tnl::Vector3 dirAtoB_world = b_pos - a_pos;
        tnl::Vector3 dirAtoB_local = tnl::Vector3::InverseTransformCoord(dirAtoB_world, a_rot);
        float radiusA = GetEllipsoidRadiusInDirection(a_radii, dirAtoB_local);

        // Bから見たAの方向で、Bの半径を近似する(Aから見た方向とちょうど逆向き)
        tnl::Vector3 dirBtoA_local = tnl::Vector3::InverseTransformCoord(-dirAtoB_world, b_rot);
        float radiusB = GetEllipsoidRadiusInDirection(b_radii, dirBtoA_local);

        // 2つとも球に近似できたので、あとは通常の球-球判定に委譲するだけ
        return tnl::IsIntersectSphere(a_pos, radiusA, b_pos, radiusB);
    }
}
