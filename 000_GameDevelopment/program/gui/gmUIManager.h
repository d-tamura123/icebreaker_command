#pragma once
#include <memory>
#include <dxe.h>

namespace gm {

    // 前方宣言
    class gmMiniMap;
    class gmOceanFlowVisualizer;
    class gmMapManager;
    class gmPlayerShip;

    class gmUIManager {
    public:
        gmUIManager(
            const tnl::Vector2f& miniMapPos,
            std::shared_ptr<gmMapManager> map,
            std::shared_ptr<gmPlayerShip> player);
        ~gmUIManager();

        // UI全体の更新
        void update(float dt);

        // UI全体の描画（要件に合わせてrenderという名前にしています）
        void render(const Shared<dxe::Camera>& camera);

    private:
        // 将来的に他のUIオブジェクト（体力バー、アイテムUI等）が増えた際にも
        // 管理しやすいよう、個別のポインタまたはコンテナで管理します
        std::unique_ptr<gmMiniMap> miniMap_;

        std::unique_ptr<gmOceanFlowVisualizer> flowVisualizer_;

    };
}
