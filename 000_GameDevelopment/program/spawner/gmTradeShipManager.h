// gmTradeShipManager.h
#pragma once
#include <memory>
#include <string>
#include "gmSpawnerManagerBase.h"
#include "../object/gmTradeShip.h"

namespace gm {

    class gmMapManager;
    class gmWaterPlane;
    class gmCollisionSystem;

    // ------------------------------------------------------------
    // NPC交易船のスポーンと一元管理。
    // 
    // gmIcebergManagerと同じく
    // gmSpawnerManagerBase<TEntity>を土台にする。
    //
    // ルートごとに個別のスポナーを持つのではなく、1つのスポナーが
    // context_->map->GetRouteCount()本すべての航路を見て、スポーンの都度
    // どの航路を使うかをランダムに選ぶ。
    // これにより、Excel側でRoute_3等を追加してもこのクラスの変更は不要になる。
    // ------------------------------------------------------------
    class gmTradeShipManager : public gmSpawnerManagerBase<gmTradeShip> {
    public:
        gmTradeShipManager(
            const std::shared_ptr<gmMapManager>& map,
            const std::shared_ptr<gmWaterPlane>& water,
            const std::shared_ptr<gmCollisionSystem>& collisionSystem);

    protected:
        void trySpawn() override;
        float rollNextInterval() const override;

    private:
        std::shared_ptr<gmMapManager> map_;
        std::shared_ptr<gmWaterPlane> water_;
        std::shared_ptr<gmCollisionSystem> collisionSystem_;

        Shared<dxe::Texture> texture_;
        Shared<dxe::Texture> normalMapTexture_;
    };

}
