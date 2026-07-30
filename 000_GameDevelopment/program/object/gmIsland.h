#pragma once
#include "gmMeshBase.h"
#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>

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


            // Sphereコライダーを自動設定する
            // Note: ハイトマップは黒(高さ0)の余白を含んだ正方形テクスチャで、
            // 実測にあわせてISLAND_COLLIDER_RADIUS_SCALEを手調整する。
            gmCollider collider;
            collider.type = ColliderShapeType::Sphere;
            collider.radius = std::min(width, depth) * 0.5f * ISLAND_COLLIDER_RADIUS_SCALE;
            // 島の基準位置(position_)から高さ方向に半分持ち上げた位置を中心にする
            collider.localOffset = tnl::Vector3(0.0f, heightMax * 0.5f, 0.0f);
            addCollider(collider);
            setCollisionCategory(gmCollisionCategory::Island);
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

        // ---- コライダー調整用パラメータ ----
        // ハイトマップの陸地(白)がマップ矩形に対してどれくらい余白を持つかの目安。
        // 1.0で矩形いっぱい、小さくするほど余白(黒)側にマージンを取る。
        static constexpr float ISLAND_COLLIDER_RADIUS_SCALE = 0.99f;
    };
}
