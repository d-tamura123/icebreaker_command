#pragma once
#include <dxe.h>
#include "gmObjectBase.h"

namespace gm {

    class gmMeshBase : public gmObjectBase {
    public:
        gmMeshBase(const std::string& id, const tnl::Vector3& pos)
            : gmObjectBase(id, pos) {};

        virtual ~gmMeshBase() = default;
        void create(const std::string filePath, float scale = 1.0f);
        virtual void render(const Shared<dxe::Camera>& camera);

        Shared<dxe::Mesh> getMesh() const { return mesh_; }
        
    protected:
        Shared<dxe::Mesh> mesh_ = nullptr;
    };
}