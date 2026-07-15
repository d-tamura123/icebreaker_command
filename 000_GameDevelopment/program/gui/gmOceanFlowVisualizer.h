#pragma once
#include <memory>
#include <dxe.h>
#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>

namespace gm {
    class gmMapManager;

    class gmOceanFlowVisualizer {
    public:
        explicit gmOceanFlowVisualizer(std::shared_ptr<gmMapManager> map);
        ~gmOceanFlowVisualizer();

        void update(); // F1 トグルなど
        void draw(const Shared<dxe::Camera>& camera);

        void setSampleStep(int step) { sampleStep_ = std::max(1, step); }
    private:
        std::weak_ptr<gmMapManager> map_;
        int arrowHandle_ = -1;
        bool visible_ = false;
        int sampleStep_ = 1;

        void loadResources();
        void releaseResources();
    };
}
