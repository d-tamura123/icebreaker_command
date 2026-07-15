// MeshEX.h
// Meshを生成するヘルパークラス
// 扱う形状のバリエーションを増やす
#pragma once
#include <vector>
#include <dxe.h>

namespace gm {

    //===============================================================
    // MeshEX
    // 目的：
    //   - dxe の Mesh を直接いじらずに「氷塊」を構成するためのヘルパークラス
    //   - 基礎形状（Cube / Sphere）を組み合わせて氷塊を作る
    //   - ライブラリのカプセル化を尊重する
    //
    // ポイント：
    //   - Mesh の内部構造（vtxs_ / idxs_）には一切触れない
    //   - dxe が提供する CreateCubeMV / CreateSphereMV を使う
    //   - StaticMeshGroupMV で複数メッシュを 1 つにまとめる
    //===============================================================
    class MeshEX {
    public:

        // 氷塊メッシュを生成する
        // baseSize : 氷片の基本サイズ
        // pieceCount : 氷片の数（5〜10個など）
        // seed : ランダムシード（-1なら現在時刻）
        static Shared<dxe::Mesh> CreateIceChunk(
            Shared<dxe::Texture> texture,
            float baseSize,
            int pieceCount = 6,
            int seed = -1
        );
        static Shared<dxe::Mesh> CreateIceChunk(
            const std::vector<std::string>& crystalPaths,
            Shared<dxe::Texture> texture,
            float baseSize,
            int pieceCount = 6,
            int seed = -1
        );

    private:

        // 氷片（Cube or Sphere）をランダム生成する
        static Shared<dxe::Mesh> createRandomPiece(float baseSize);

        // 氷片をランダム位置・回転・スケールで配置する
        static tnl::Matrix createRandomTransform(float baseSize);

        static tnl::Matrix createRandomTransformCrystal(bool isCenter);
    };

}
