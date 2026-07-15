// gmRenderUtil.cpp
#include "gmRenderUtil.h"
#include <DxLib.h>

namespace gm {
    void ApplyCamera3D(const Shared<dxe::Camera>& camera)
    {
        MATRIX view, proj;
        memcpy(view.m, camera->getViewMatrix().m, sizeof(float) * 16);
        memcpy(proj.m, camera->getProjectionMatrix().m, sizeof(float) * 16);
        SetCameraViewMatrix(view);
        SetupCamera_ProjectionMatrix(proj);
    }
}