#pragma once
#include "../gmSceneBase.h"
#include "../../gmGameContext.h"

#include "../../object/gmPlayerShip.h"
#include "../../object/gmWaterPlane.h"
#include "../../object/gmIsland.h"
#include "../../mesh_ex/gmMeshEX.h"

#include "../../debug/gmKyleDebugger.h"

namespace gm
{
    // ‘O•ûéŒ¾
    class gmUIManager;

    class gmGameScene : public gmSceneBase
    {
    public:
        gmGameScene();
        ~gmGameScene();

        void onEnter(std::shared_ptr<gmSceneManager> manager) override;
        void update() override;
        void draw() override;
        void onExit() override;

    private:
        std::shared_ptr<gmGameContext> context_;

        std::shared_ptr<gmKyleDebugger> debugger_;
        std::shared_ptr<gmPlayerShip> playerShip_;
        std::shared_ptr<gmWaterPlane> water_;
        std::vector<std::shared_ptr<gmIsland>> islands_;
        Shared<dxe::Mesh> iceChunk_;
        std::unique_ptr<gmUIManager> uiManager_;

        // ƒJƒƒ‰‘€ì—p
        bool isDrag_ = false;
        float yaw_ = 0.0f;
        float pitch_ = 0.0f;
        float dist_ = 250.0f;
        tnl::Vector3 camTarget_ = { 0, 0, 0 };
        tnl::Vector3 camOffset_ = { 0, 200, 0 };
    };
}
