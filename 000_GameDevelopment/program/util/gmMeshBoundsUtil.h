#pragma once
#include <dxe.h>

namespace gm {

    // ------------------------------------------------------------
    // MV1モデルの「真の」バウンディングボックスを計算する。
    //
    // 【背景】
    // dxe::Mesh::getBoundingBoxSize()は内部でDxLibの
    // MV1GetMeshMaxPosition(handle, 0) を使っているが、第2引数の 0 は
    // 「メッシュインデックス」であり、モデルが複数パーツ(船体・マスト等)で
    // 構成されている場合、インデックス0は全体の一部分しか表さないことがある。
    //
    // この関数は MV1GetMeshNum() で数えた全サブメッシュ分の
    // Min/Maxをそれぞれ合算することで、パーツ数に関わらずモデル全体を
    // 覆う正しいバウンディングボックスを求める。
    //
    // arg1... 対象モデルのハンドル(dxe::Mesh::getDxMvHdl()で取得)
    // arg2... [out] バウンディングボックスの中心(ローカル座標)
    // arg3... [out] バウンディングボックスのサイズ(全長。半分の長さではない)
    // ret.... 計算できた場合はtrue(サブメッシュが1つも無い異常なモデルの場合はfalse)
    // ------------------------------------------------------------
    bool ComputeMeshBounds(int mvHandle, tnl::Vector3& outCenter, tnl::Vector3& outSize);
}
