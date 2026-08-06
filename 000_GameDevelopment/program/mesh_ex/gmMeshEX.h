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

        // 氷塊を構成する「ベースメッシュ(結晶1個ぶん)+ピースごとのローカル変換行列」の組。
        // CreateIceChunk()はこれをCreateStaticMeshGroupMVで1つに焼き込んだ結果だけを返すが、
        // 氷山の分裂(大→中→小)のように「焼き込む前のピース単位の情報」が必要な場面のために
        // 別出しして返せるようにしておく。
        // 用途: 起動時に一括で「大メッシュ+分割済みの中/小メッシュ」を事前焼成する(gmIcebergManager参照)。
        // 焼成自体はCreateStaticMeshGroupMVが内部でMV1LoadModelFromMemを伴う重い処理のため、
        // 実行中(戦闘中)に都度呼ぶのは避け、起動時にまとめて済ませる前提。
        struct IceChunkPieces {
            Shared<dxe::Mesh> baseMesh;                // 共通のベースメッシュ(結晶1個ぶん、テクスチャ設定済み)
            std::vector<tnl::Matrix> pieceMatrices;    // ピースごとのローカル変換行列
        };

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

        // CreateIceChunk(crystalPaths版)の「焼き込む前」のピース情報を返す版。
        // 呼び出し側で好きな部分集合を選んでdxe::Mesh::CreateStaticMeshGroupMV(baseMesh, subset)に
        // 渡せば、その部分集合だけのメッシュを別途焼成できる(氷山の分裂用途を想定)。
        static IceChunkPieces CreateIceChunkPieces(
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
