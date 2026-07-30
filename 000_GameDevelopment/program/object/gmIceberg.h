#pragma once
#include <memory>
#include "gmMeshBase.h"

namespace gm {

    class gmMapManager;
    class gmWaterPlane;

    // ------------------------------------------------------------
    // 海流に流される流氷
    // gmShip(自機/NPC)と違い、自律的な操舵は行わず、
    // 位置は常に海流ベクトル(+慣性)だけに従う。
    // ------------------------------------------------------------
    class gmIceberg : public gmMeshBase {
    public:
        // mesh は gm::MeshEX::CreateIceChunk で生成済みのものをそのまま渡す
        gmIceberg(const std::string& id, const tnl::Vector3& pos, const Shared<dxe::Mesh>& mesh);

        void update(float deltaTime) override;
        void render(const Shared<dxe::Camera>& camera) override;

        void setMap(const std::shared_ptr<gmMapManager>& map);
        void setWater(const std::shared_ptr<gmWaterPlane>& water);

        // 衝突検出イベント
        // : 相手のカテゴリによって挙動を分岐する
        // (船・島・他の氷山とは移動抑止、砲弾は溶解処理をおこなう)
        void onCollisionEnter(gmObjectBase* other) override;

    private:
        void updateDrift(float deltaTime); // 海流+慣性による移動(位置のみ、向きには影響しない)
        void updateWave(float deltaTime);  // 波の揺らぎ(gmShipと同じ仕組みだが係数で薄めて重量感を出す)
        void updateSpin(float deltaTime);  // ゆるやかな自転(演出用。海流方向とは無関係)
        void checkOutOfBounds();           // マップ範囲外まで流されたら消滅させる(kill())

        std::weak_ptr<gmMapManager> map_;
        std::weak_ptr<gmWaterPlane> water_;

        tnl::Vector3 velocity_{ 0.0f, 0.0f, 0.0f };
        float waveTime_ = 0.0f;
        float spinSpeed_ = 0.0f; // 個体ごとにランダムな自転速度(生成時に決定)

        // ---- 調整用パラメータ ----
        static constexpr float DRIFT_SCALE = 22.0f;             // 海流ベクトル→速度への倍率
        static constexpr float INERTIA_RATE_PER_SEC = 0.075f;   // 慣性の強さ(小さいほど鈍い=重い動き)
        static constexpr float WAVE_DAMPING = 0.1f;             // 上下動の減衰(1.0だと船と同じ強さになる)
        static constexpr float TILT_DAMPING = 0.1f;             // 傾きの減衰
        static constexpr float OUT_OF_BOUNDS_MARGIN = 50.0f;    // マップ端からの余裕(world単位)

        // 実測で球が見た目よりひと回り大きかったため、
        // 対角線ベースではなく最大軸ベースの半径にした上でさらにこの倍率を掛ける。
        // 1.0で「最大軸の半分」、小さくするほど控えめになる。
        static constexpr float ICEBERG_COLLIDER_RADIUS_SCALE = 0.93f;
    };
}
