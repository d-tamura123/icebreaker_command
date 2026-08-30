// gmSkybox.cpp
#include "gmSkybox.h"
#include "../gmGameConfig.h"
#include <DxLib.h>

namespace gm {

    gmSkybox::gmSkybox()
    {
        mesh_ = dxe::Mesh::CreateCubeMV(SKYBOX_SIZE, 20, 20);
        mesh_->setDefaultLightEnable(false); // 光源の影響を受けず、常に一定の見た目にする
        mesh_->setTexture(dxe::Texture::CreateFromFile(SKYBOX_TEXTURE_FILE_PATH));
        mesh_->loadMaterial(SKYBOX_MATERIAL_FILE_PATH); // エミッシブのみのマテリアル(dxe開発者側で用意済み)
    }

    void gmSkybox::update(float deltaTime, const Shared<dxe::Camera>& camera)
    {
        mesh_->mulRotation(tnl::Quaternion::RotationAxis({ 0, 1, 0 }, tnl::ToRadian(SKYBOX_ROTATION_SPEED_DEG_PER_SEC * deltaTime)));

        // カメラのX/Zへ追従させる(gmWaterPlane::update()と同じ考え方)。
        // Yは、テクスチャ自体の空・水面反射の比率と、実際のゲーム画面での見え方の比率が
        // ズレている(海面の映る割合が想定よりかなり多い)ため、
        // SKYBOX_Y_OFFSETで見た目の帳尻を合わせている
        // (精密なUVマッピングの解析はせず、実機で見ながら調整する前提の簡易的な値)。
        const tnl::Vector3 camPos = camera->getPosition();
        mesh_->setPosition({ camPos.x, 0.0f + SKYBOX_Y_OFFSET, camPos.z });
    }

    void gmSkybox::render(const Shared<dxe::Camera>& camera)
    {
        mesh_->render(camera);
    }
}
