#include "gmMeshEX.h"
#include <dxe.h>

using namespace gm;

//===============================================================
// 氷塊メッシュを生成する
//===============================================================
Shared<dxe::Mesh> MeshEX::CreateIceChunk(Shared<dxe::Texture> texture, float baseSize, int pieceCount,int seed)
{
    if (seed < 0) seed = (int)time(nullptr);
    tnl::SetSeedMersenneTwister32(seed);

    // 氷片を複数生成する
    std::vector<tnl::Matrix> matrices;
    matrices.reserve(pieceCount);

    // 氷片の元メッシュ（Cube でも Sphere でも OK）
    // ※ StaticMeshGroupMV は「元メッシュを複数配置する」仕組み
    Shared<dxe::Mesh> baseMesh = dxe::Mesh::CreateCubeMV(baseSize);

    // テクスチャを設定する
    if (texture) {
        baseMesh->setTexture(texture);
    }

    for (int i = 0; i < pieceCount; i++) {
        matrices.push_back(createRandomTransform(baseSize));
    }

    // 複数の氷片を 1 つのメッシュにまとめる
    // ※ Mesh の内部構造には触れず、dxe の機能だけで完結する
    Shared<dxe::Mesh> iceChunk =
        dxe::Mesh::CreateStaticMeshGroupMV(baseMesh, matrices);

    return iceChunk;
}

Shared<dxe::Mesh> MeshEX::CreateIceChunk(const std::vector<std::string>& crystalPaths, Shared<dxe::Texture> texture, float baseSize, int pieceCount, int seed)
{
    if (seed < 0) seed = (int)time(nullptr);
    tnl::SetSeedMersenneTwister32(seed);

    // ランダムにクリスタルを選ぶ（中心用）
    auto baseMesh = dxe::Mesh::CreateFromFileMV(
        crystalPaths[tnl::GetRandomDistribution<int>(0, (int)crystalPaths.size() - 1)],
        baseSize
    );

    baseMesh->setTexture(texture);
    baseMesh->setDefaultLightEnable(true);

    std::vector<tnl::Matrix> matrices;
    matrices.reserve(pieceCount);

    for (int i = 0; i < pieceCount; i++) {
        matrices.push_back(createRandomTransformCrystal(i == 0)); // ★中心は大きめ
    }

    return dxe::Mesh::CreateStaticMeshGroupMV(baseMesh, matrices);
}
//===============================================================
// 氷片（Cube or Sphere）をランダム生成する
// ※ 今回は Cube を使うが、Sphere に変えても OK
//===============================================================
Shared<dxe::Mesh> MeshEX::createRandomPiece(float baseSize)
{
    // ここでは Cube を使う（Sphere にしても良い）
    return dxe::Mesh::CreateCubeMV(baseSize);
}


//===============================================================
// 氷片のランダム変換行列を作る
//===============================================================
tnl::Matrix MeshEX::createRandomTransform(float baseSize)
{
    // ランダム位置
    float px = tnl::GetRandomDistribution<float>(-baseSize * 0.5f, baseSize * 0.5f);
    float py = tnl::GetRandomDistribution<float>(-baseSize * 0.3f, baseSize * 0.3f);
    float pz = tnl::GetRandomDistribution<float>(-baseSize * 0.5f, baseSize * 0.5f);

    // ランダム回転
    float pitch = tnl::GetRandomDistribution<float>(0.0f, tnl::PI);
    float yaw   = tnl::GetRandomDistribution<float>(0.0f, tnl::PI);
    float roll  = tnl::GetRandomDistribution<float>(0.0f, tnl::PI);

    // ランダムスケール
    float sx = tnl::GetRandomDistribution<float>(0.7f, 1.3f);
    float sy = tnl::GetRandomDistribution<float>(0.7f, 1.3f);
    float sz = tnl::GetRandomDistribution<float>(0.7f, 1.3f);

    // 行列を合成する
    // ※ tnl::Matrix は DirectXMath の XMMATRIX をラップしているので
    //   Scaling → Rotation → Translation の順で掛け合わせる
    tnl::Matrix mat = tnl::Matrix::Scaling(sx, sy, sz)
        * tnl::Matrix::RotationPitchYawRoll(pitch, yaw, roll)
        * tnl::Matrix::Translation(px, py, pz);

    return mat;
}

/*
tnl::Matrix MeshEX::createRandomTransformCrystal()
{
    float px = tnl::GetRandomDistribution<float>(-20.0f, 20.0f);
    float py = tnl::GetRandomDistribution<float>(-10.0f, 10.0f);
    float pz = tnl::GetRandomDistribution<float>(-20.0f, 20.0f);

    float pitch = tnl::GetRandomDistribution<float>(0.0f, tnl::PI * 2.0f);
    float yaw = tnl::GetRandomDistribution<float>(0.0f, tnl::PI * 2.0f);
    float roll = tnl::GetRandomDistribution<float>(0.0f, tnl::PI * 2.0f);

    float s = tnl::GetRandomDistribution<float>(0.7f, 1.4f);

    return tnl::Matrix::Scaling(s, s, s)
        * tnl::Matrix::RotationPitchYawRoll(pitch, yaw, roll)
        * tnl::Matrix::Translation(px, py, pz);
}
*/
tnl::Matrix MeshEX::createRandomTransformCrystal(bool isCenter)
{
    // 配置範囲（狭める）
    float px = tnl::GetRandomDistribution<float>(-15.0f, 15.0f);
    float py = tnl::GetRandomDistribution<float>(-8.0f, 8.0f);
    float pz = tnl::GetRandomDistribution<float>(-15.0f, 15.0f);

    // 回転（方向性を揃える）
    float pitch = tnl::GetRandomDistribution<float>(-0.3f, 0.3f);     // X回転は小さく
    float yaw = tnl::GetRandomDistribution<float>(0.0f, tnl::PI * 2.0f); // Yは自由
    float roll = tnl::GetRandomDistribution<float>(-0.3f, 0.3f);     // Z回転は小さく

    // スケール（中心は大きめ）
    float s;
    if (isCenter) {
        s = tnl::GetRandomDistribution<float>(1.3f, 1.9f); // ★核となる氷片
    }
    else {
        s = tnl::GetRandomDistribution<float>(0.8f, 1.3f);
    }

    return tnl::Matrix::Scaling(s, s, s)
        * tnl::Matrix::RotationPitchYawRoll(pitch, yaw, roll)
        * tnl::Matrix::Translation(px, py, pz);
}
