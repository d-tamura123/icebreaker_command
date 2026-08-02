// gmVFXManager.cpp
#include "gmVFXManager.h"
#include "gmSpriteAnimRegistry.h"
#include "../gmGameConfig.h"
#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>

namespace gm {

    gmVFXManager::gmVFXManager(const std::shared_ptr<gmSpriteAnimRegistry>& registry)
        : registry_(registry)
    {
    }

    // ------------------------------------------------------------
    // ワンショット再生のエフェクトを1つ生成し、管理リスト(instances_)へ追加する。
    // 再生が終わったかどうかの判定・後片付けはupdate()側で行う。
    // ------------------------------------------------------------
    void gmVFXManager::play(const std::string& clipName, const tnl::Vector3& worldPos, float size)
    {
        if (!registry_) return;

        const gmSpriteAnimClip* clip = registry_->getClip(clipName);
        if (!clip) {
            // 該当するメタデータが無い(CSVに無い名前を指定した等)。
            // 静かに無視する(エフェクトが1個出なかっただけでゲームを止めるほどではないため)。
            return;
        }

        Shared<dxe::Texture> tex = registry_->getTexture(clipName, VFX_EFFECT_GRAPHICS_DIR);

        auto instance = std::make_unique<gmSpriteAnimInstance>();
        instance->start(*clip, tex, worldPos, size, /*sustain=*/false);
        instances_.push_back(std::move(instance));
    }

    // ------------------------------------------------------------
    // 持続系再生のエフェクトを1つ生成し、管理リスト(instances_)へ追加した上で、
    // 呼び出し側が終了指示(requestStop())を送れるよう生ポインタを返す。
    // ------------------------------------------------------------
    gmSpriteAnimInstance* gmVFXManager::playSustained(const std::string& clipName, const tnl::Vector3& worldPos, float size)
    {
        if (!registry_) return nullptr;

        const gmSpriteAnimClip* clip = registry_->getClip(clipName);
        if (!clip) {
            return nullptr;
        }

        Shared<dxe::Texture> tex = registry_->getTexture(clipName, VFX_EFFECT_GRAPHICS_DIR);

        auto instance = std::make_unique<gmSpriteAnimInstance>();
        instance->start(*clip, tex, worldPos, size, /*sustain=*/true);

        gmSpriteAnimInstance* instancePtr = instance.get();
        instances_.push_back(std::move(instance));
        return instancePtr;
    }

    // ------------------------------------------------------------
    // 管理下の全エフェクトを更新し、再生が完全に終わったものを取り除く。
    // ------------------------------------------------------------
    void gmVFXManager::update(float deltaTime)
    {
        for (auto& instance : instances_) {
            instance->update(deltaTime);
        }

        // 再生が完全に終わったインスタンスを取り除く
        instances_.erase(
            std::remove_if(instances_.begin(), instances_.end(),
                [](const std::unique_ptr<gmSpriteAnimInstance>& instance) { return instance->isFinished(); }),
            instances_.end()
        );
    }

    // ------------------------------------------------------------
    // 管理下の全エフェクトを描画する。
    // ------------------------------------------------------------
    void gmVFXManager::render(const Shared<dxe::Camera>& camera)
    {
        for (auto& instance : instances_) {
            instance->render(camera);
        }
    }
}
