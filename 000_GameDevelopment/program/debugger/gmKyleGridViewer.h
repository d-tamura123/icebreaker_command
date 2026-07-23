#pragma once
#include "../object/gmObjectBase.h"
#include <dxe.h>
#include <DxLib.h>

namespace gm {

    class gmKyleGridViewer : public gmObjectBase {
    public:
        gmKyleGridViewer(
            const std::string& id,
            const tnl::Vector3& pos
        )
            : gmObjectBase(id, pos)
        {
        }

        virtual void render(const Shared<dxe::Camera>& camera) override;

    private:
        // ---- 決め打ちパラメタ ----
        static constexpr int   CELL_COUNT = 100;         // 描画するマス数
        static constexpr int   GRID_COLOR = 0xffcccccc;  // 基軸以外の色（薄いグレー）

        // ---- 内部処理 ----
        void drawGrid(const Shared<dxe::Camera>& camera);
    };

}
