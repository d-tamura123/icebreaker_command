#pragma once
#pragma once
#include "gmMeshBase.h"

namespace gm {

    class gmIsland : public gmMeshBase {
    public:
        gmIsland(
            const std::string& id,
            const tnl::Vector3& pos,
            const std::string& heightmapPath,
            const std::string& diffusePath,
            float width,
            float depth,
            float heightMax,
            int divW = 200,
            int divH = 200
        )
            : gmMeshBase(id, pos)
        {
            // HeightMap から島メッシュ生成
            mesh_ = dxe::Mesh::CreateFromHeightMapMV(
                heightmapPath,
                width,
                depth,
                heightMax,
                divW,
                divH
            );

            // テクスチャ設定
            auto tex = dxe::Texture::CreateFromFile(diffusePath);
            mesh_->setTexture(tex);

            mesh_->setBlendMode(DX_BLENDMODE_ALPHA);
            mesh_->setDefaultLightEnable(true);

            // 初期位置
            mesh_->setPosition(pos);

            // 基準サイズ保持
            baseWidth_ = width;
            baseDepth_ = depth;
        }

        virtual ~gmIsland() = default;

        // 必要なら update を拡張
        virtual void update(float deltaTime) override {
            // 島は基本動かないが、
            // 海流AIで揺らすならここに処理を書く
        }

        virtual void render(const Shared<dxe::Camera>& camera) override;


        // 現在のスケールを加味した「実際の幅」を計算して返す
        float getActualWidth() const {
            return baseWidth_ * scale_.x;
        }
        float getActualDepth() const {
            return baseDepth_ * scale_.z;
        }

    private:
        // 生成時の基準サイズ（生成したら不変）
        float baseWidth_ = 0.0f;
        float baseDepth_ = 0.0f;
    };
}
