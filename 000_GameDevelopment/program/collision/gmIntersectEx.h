#pragma once
#include <dxe.h>


namespace gm {

    //=================================================================================================
    // tnl::intersect (common/tnl/tnl_intersect.h、変更不可のライブラリ) に無い組み合わせを補うための
    // 交差判定関数群。
    //=================================================================================================

    //-----------------------------------------------------------------------------------------------------
    // work... OBB(直方体・立方体) 同士の衝突判定 ※分離軸定理(SAT)を使用
    //
    // 【SAT(分離軸定理)】
    // 「2つの凸形状(へこみのない形状)が重なっていないなら、
    //   必ずどこかの向き(軸)に光を当てて2つの形状を"影"として壁に映したとき、
    //   2つの影が触れ合わずに離れて見える向きが存在する」という考え方。
    // この"影が離れて見える軸"のことを「分離軸」と呼ぶ。
    //
    // 逆に言うと、「考えられる分離軸の候補を全部試してみて、
    // どの軸に映しても影が重なっている」なら、2つの形状は衝突している、と判定できる。
    //
    // OBB(向きを持った直方体)同士の場合、分離軸の候補は以下の15本だけ調べればよいことが
    // 数学的に証明されている(直方体は面が6枚・辺の向きが3種類しかないため)。
    //   ・Aの3つの面の法線方向(3本)
    //   ・Bの3つの面の法線方向(3本)
    //   ・Aの辺の向きとBの辺の向きの外積、全部の組み合わせ(3×3=9本)
    //   合計 3 + 3 + 9 = 15本
    //
    // arg1... AのOBB中心座標
    // arg2... AのOBBサイズ(各軸の全長。半分の長さではない点に注意。tnl::ToMaxAABB/ToMinAABBと同じ規約)
    // arg3... AのOBBの姿勢(回転)
    // arg4... BのOBB中心座標
    // arg5... BのOBBサイズ(全長)
    // arg6... BのOBBの姿勢(回転)
    // ret.... [ 衝突している : true ]   [ 衝突していない : false ]
    bool IsIntersectOBB(const tnl::Vector3& a_pos, const tnl::Vector3& a_size, const tnl::Quaternion& a_rot,
        const tnl::Vector3& b_pos, const tnl::Vector3& b_size, const tnl::Quaternion& b_rot);

    //-----------------------------------------------------------------------------------------------------
    // work... カプセル同士の衝突判定
    //
    // Note:
    // カプセルは「太さのある線分」と考えることができる。
    // つまりカプセルAとカプセルBが衝突しているかどうかは、
    //   (1) カプセルAの芯となる線分(始点→終点)と、カプセルBの芯となる線分の
    //       「一番近い2点」を見つけ、
    //   (2) その2点間の距離が、Aの半径 + Bの半径 以下であれば衝突
    // という2ステップだけで判定できる。
    // (1)の「線分と線分の一番近い2点を求める」処理が本関数のほとんどを占める。
    //
    // arg1... カプセルAの始点
    // arg2... カプセルAの終点
    // arg3... カプセルAの半径
    // arg4... カプセルBの始点
    // arg5... カプセルBの終点
    // arg6... カプセルBの半径
    // arg7... カプセルAの始点(t=0.0)〜終点(t=1.0)のうち、最接近した位置(不要なら省略可)
    // arg8... カプセルBの始点(t=0.0)〜終点(t=1.0)のうち、最接近した位置(不要なら省略可)
    // ret.... [ 衝突している : true ]   [ 衝突していない : false ]
    bool IsIntersectCapsuleCapsule(const tnl::Vector3& a_st, const tnl::Vector3& a_en, float a_r,
        const tnl::Vector3& b_st, const tnl::Vector3& b_en, float b_r,
        float* a_t = nullptr, float* b_t = nullptr);

    //-----------------------------------------------------------------------------------------------------
    // work... 楕円球の「ある方向を向いたときの半径」を求める
    //
    // Note:
    // 楕円球は、球を3つの軸それぞれ違う倍率で引き伸ばした(潰した)形。
    // 球なら半径はどの方向でも同じ値だが、楕円球は方向によって中心から表面までの距離が変わる。
    // この関数は「楕円球のローカル座標系(回転を打ち消した状態)で、
    // ある方向(dir)を向いたときに、中心から表面までの距離が何になるか」を計算する。
    //
    // 「2つの中心を結ぶ方向」についてこの半径を求めれば、
    // その方向に限っては楕円球を「その半径を持つ球」として扱ってよいことになる。
    // これを利用して、楕円球が絡む交差判定を「球としての交差判定」に近似できる
    // (楕円球同士や、楕円球と他の形状との判定すべてに共通して使うヘルパー)。
    //
    // arg1... 楕円球の各軸の半径(x, y, z)
    // arg2... 調べたい方向ベクトル(楕円球のローカル座標系、正規化されていなくてもよい)
    // ret.... その方向を向いたときの、中心から表面までの距離
    float GetEllipsoidRadiusInDirection(const tnl::Vector3& radii, const tnl::Vector3& dir);

    //-----------------------------------------------------------------------------------------------------
    // work... 楕円球と球の衝突判定(近似)
    // tips... 2点を結ぶ方向における楕円球の半径を求め、その方向限定の球として扱って判定する
    // arg1... 楕円球の中心座標
    // arg2... 楕円球の各軸の半径
    // arg3... 楕円球の姿勢(回転)
    // arg4... 球の中心座標
    // arg5... 球の半径
    // ret.... [ 衝突している(近似) : true ]   [ 衝突していない : false ]
    bool IsIntersectEllipsoidSphere(const tnl::Vector3& ellip_pos, const tnl::Vector3& ellip_radii, const tnl::Quaternion& ellip_rot,
        const tnl::Vector3& sphere_pos, float sphere_r);

    //-----------------------------------------------------------------------------------------------------
    // work... 楕円球とOBBの衝突判定(近似)
    // tips... OBBの中心方向における楕円球の半径を求め、その方向限定の球としてSphere-OBB判定を流用する
    // arg1... 楕円球の中心座標
    // arg2... 楕円球の各軸の半径
    // arg3... 楕円球の姿勢(回転)
    // arg4... OBBの中心座標
    // arg5... OBBのサイズ(全長)
    // arg6... OBBの姿勢(回転)
    // ret.... [ 衝突している(近似) : true ]   [ 衝突していない : false ]
    bool IsIntersectEllipsoidOBB(const tnl::Vector3& ellip_pos, const tnl::Vector3& ellip_radii, const tnl::Quaternion& ellip_rot,
        const tnl::Vector3& obb_pos, const tnl::Vector3& obb_size, const tnl::Quaternion& obb_rot);

    //-----------------------------------------------------------------------------------------------------
    // work... 楕円球とカプセルの衝突判定(近似)
    // tips... カプセルの芯線分上で楕円球中心に最も近い点を求め、その方向限定の球としてSphere-Capsule相当の判定を行う
    // arg1... 楕円球の中心座標
    // arg2... 楕円球の各軸の半径
    // arg3... 楕円球の姿勢(回転)
    // arg4... カプセルの始点
    // arg5... カプセルの終点
    // arg6... カプセルの半径
    // ret.... [ 衝突している(近似) : true ]   [ 衝突していない : false ]
    bool IsIntersectEllipsoidCapsule(const tnl::Vector3& ellip_pos, const tnl::Vector3& ellip_radii, const tnl::Quaternion& ellip_rot,
        const tnl::Vector3& cap_st, const tnl::Vector3& cap_en, float cap_r);

    //-----------------------------------------------------------------------------------------------------
    // work... 楕円球同士の衝突判定(近似)
    // tips... 双方とも「相手中心への方向」における半径を求め、その2つの球としてtnl::IsIntersectSphereへ委譲する
    // arg1... 楕円球Aの中心座標
    // arg2... 楕円球Aの各軸の半径
    // arg3... 楕円球Aの姿勢(回転)
    // arg4... 楕円球Bの中心座標
    // arg5... 楕円球Bの各軸の半径
    // arg6... 楕円球Bの姿勢(回転)
    // ret.... [ 衝突している(近似) : true ]   [ 衝突していない : false ]
    bool IsIntersectEllipsoidEllipsoid(const tnl::Vector3& a_pos, const tnl::Vector3& a_radii, const tnl::Quaternion& a_rot,
        const tnl::Vector3& b_pos, const tnl::Vector3& b_radii, const tnl::Quaternion& b_rot);
}
