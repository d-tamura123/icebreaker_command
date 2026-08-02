// gmVFXManager.h
#pragma once
#include <dxe.h>
#include <vector>
#include <memory>
#include <string>
#include "gmSpriteAnimInstance.h"

namespace gm {

    class gmSpriteAnimRegistry;

    // ------------------------------------------------------------
    // 発生した全エフェクトインスタンスの生成・毎フレーム更新・描画・
    // 後片付けを一元管理する。
    // ------------------------------------------------------------
    class gmVFXManager {
    public:
        explicit gmVFXManager(const std::shared_ptr<gmSpriteAnimRegistry>& registry);

        // ワンショット再生(着弾エフェクト等)。再生完了後、自動的に片付けられる。
        // arg1... クリップ名(例: "tktk_Impact_19")
        // arg2... 発生位置(ワールド座標)
        // arg3... ビルボードの一辺の大きさ(world単位)
        void play(const std::string& clipName, const tnl::Vector3& worldPos, float size = 100.0f);

        // 持続系再生(火炎放射等)。戻り値のインスタンスに対して
        // 呼び出し側がrequestStop()を呼ぶことで終了させる。
        // 注意: 戻り値のポインタは、そのエフェクトが完全に終了する(isFinished()==true)まで
        //       gmVFXManager内部で保持され続けるが、それ以降は無効になるため、
        //       攻撃継続中(火を噴いている間)だけ短期的に保持する用途に留めること。
        gmSpriteAnimInstance* playSustained(const std::string& clipName, const tnl::Vector3& worldPos, float size = 100.0f);

        void update(float deltaTime);
        void render(const Shared<dxe::Camera>& camera);

    private:
        std::shared_ptr<gmSpriteAnimRegistry> registry_;
        std::vector<std::unique_ptr<gmSpriteAnimInstance>> instances_; // 再生中の全エフェクトインスタンス
    };
}
