#pragma once
#include <memory>
#include "gmMeshBase.h"

namespace gm {

    // 前方宣言
    class gmMapManager;
    class gmWaterPlane;
    class gmWallet;

    // ------------------------------------------------------------
    // 海流に流される流氷
    // gmShip(自機/NPC)と違い、自律的な操舵は行わず、
    // 位置は常に海流ベクトル(+慣性)だけに従う。
    //
    // サイズは 大(4ピース)/中(2ピース)/小(1ピース) の3段階固定。
    // 割る弾で命中すると、大→中→小の順にちょうど半分へ分裂する
    // (小はこれ以上分裂できないため、割る弾に対しては無反応)。
    // どのファミリー(families_の何番目由来か)の、どのティアかは
    // familyIndex_ / tier_ / mediumIndex_ で管理し、実際の分裂処理
    // (新しい氷山の生成)はgmIcebergManager側が担当する。
    // gmIcebergはあくまで「分裂したい」という意思表示(pendingSplit_)だけ持つ。
    // ------------------------------------------------------------
    class gmIceberg : public gmMeshBase {
    public:
        // ティア
        enum class Tier { Small, Medium, Large };

        // mesh は gm::MeshEX::CreateIceChunk で生成済みのものをそのまま渡す
        gmIceberg(const std::string& id, const tnl::Vector3& pos, const Shared<dxe::Mesh>& mesh);

        void update(float deltaTime) override;
        void render(const Shared<dxe::Camera>& camera) override;

        void setMap(const std::shared_ptr<gmMapManager>& map);
        void setWater(const std::shared_ptr<gmWaterPlane>& water);

        // 溶かすダメージに応じた経験値の付与先。分裂で生まれる子氷山にも
        // gmIcebergManager側で必ず設定される想定(applyMeltDamage()参照)。
        void setWallet(const std::shared_ptr<gmWallet>& wallet);

        // 衝突検出イベント
        // : 相手のカテゴリによって挙動を分岐する
        // (船・島・他の氷山とは移動抑止、砲弾は溶解処理をおこなう)
        void onCollisionEnter(gmObjectBase* other) override;


        // ---- 分裂・生成まわりのメタデータ(gmIcebergManagerが読み書きする) ----
        // どのファミリー(families_の何番目)由来かを覚えておく。
        // 分裂時、同じファミリーの1段階下のメッシュを取り出すために使う。
        void setFamilyIndex(int index) { familyIndex_ = index; }
        int getFamilyIndex() const { return familyIndex_; }

        void setTier(Tier tier) { tier_ = tier; }
        Tier getTier() const { return tier_; }

        // 中(Medium)ティアの場合、大の何番目の半分から生まれたか(0 or 1)。
        // さらに分裂して小を生成する際、どちらの小ペアを使うかの判定に使う。
        void setMediumIndex(int index) { mediumIndex_ = index; }
        int getMediumIndex() const { return mediumIndex_; }

        // 分裂直後の押し出し初速を設定する(通常のvelocity_=海流追従とは別軸で、
        // 独立して素早く減衰し0へ収束する)。自転も合わせて設定する。
        void setSplitPushVelocity(const tnl::Vector3& pushVelocity) { splitPushVelocity_ = pushVelocity; }
        void setSpinSpeed(float spinSpeed) { spinSpeed_ = spinSpeed; }

        // 生成直後だけ、他の氷山との押し戻し判定(revertToLastSafePosition)を無視する猶予時間を設定する。
        // 用途: 分裂直後の兄弟フラグメント2個は同じ座標で生まれるため、押し出しで
        // 十分に離れるまでの間、氷山どうしの押し戻しが働くとお互いを固定してしまう
        // (moveできてもrevertで巻き戻され続ける)ため。
        // 通常スポーン(gmIcebergManager::trySpawn)では呼ばない(=既定の0秒 = 即座に通常の判定)。
        void setCollisionGracePeriod(float seconds) { collisionGraceTimer_ = seconds; }

        // ---- 分裂リクエスト(意思表示のみ。実際の生成はgmIcebergManagerが行う) ----
        bool hasPendingSplitRequest() const { return pendingSplit_; }
        // 呼ぶと同時にリクエストは消費(リセット)される
        tnl::Vector3 consumePendingSplitDirection();


    private:
        void updateDrift(float deltaTime); // 海流+慣性による移動(位置のみ、向きには影響しない)
        void updateWave(float deltaTime);  // 波の揺らぎ(gmShipと同じ仕組みだが係数で薄めて重量感を出す)
        void updateSpin(float deltaTime);  // ゆるやかな自転(演出用。海流方向とは無関係)
        void checkOutOfBounds();           // マップ範囲外まで流されたら消滅させる(kill())

        // ---- 溶解(耐久値)まわり ----
        void applyMeltDamage(float amount);         // 耐久値を減らし、見た目(スケール)とコライダーを追従させる
        void updateMeltVisual(float deltaTime);     // health_に応じてscale_・colliders_[0]を再計算する

        // ティアに応じたダメージ倍率 (ダメージ値計算用のゲームロジック)
        // (戦略性の設計: 割ってから攻撃する方が効率が良くなるようにする)
        float getShotDamageMultiplier() const;
        float getFlameDamageMultiplier() const;

        std::weak_ptr<gmMapManager> map_;
        std::weak_ptr<gmWaterPlane> water_;
        std::weak_ptr<gmWallet> wallet_;

        tnl::Vector3 velocity_{ 0.0f, 0.0f, 0.0f };
        
        // 分裂の押し出しぶんの速度。velocity_(海流追従)とは独立させて、
        // 自分専用の速い減衰レートで0へ収束させる(SPLIT_PUSH_DECAY_RATE_PER_SEC参照)。
        tnl::Vector3 splitPushVelocity_{ 0.0f, 0.0f, 0.0f };
        
        float waveTime_     = 0.0f;
        float spinSpeed_    = 0.0f; // 個体ごとにランダムな自転速度(生成時に決定)

        // ---- 分裂メタデータ ----
        int  familyIndex_   = 0;
        Tier tier_          = Tier::Large;
        int  mediumIndex_   = -1; // Mediumティアのときだけ意味を持つ(0 or 1)

        bool pendingSplit_ = false;
        tnl::Vector3 pendingSplitDir_{ 0.0f, 0.0f, 1.0f }; // 分裂の押し出し軸(水平, 正規化済み)

        // 生成直後、氷山どうしの押し戻し判定を無視する残り秒数(0以下で通常判定に戻る)
        float collisionGraceTimer_ = 0.0f;

        // ---- 溶解(耐久値)まわり ----
        // health_ = 1.0が満タン、0.0で完全に溶けてkill()される。
        // 見た目はscale_ = health_に応じた係数で縮小させ、コライダー半径も追従させる。
        float health_                   = 1.0f;
        float displayScale_ = 1.0f;
        float baseColliderRadius_       = 0.0f;                 // 満タン時のコライダー半径(コンストラクタでComputeMeshBoundsから設定)
        tnl::Vector3 baseColliderOffset_{ 0.0f, 0.0f, 0.0f };   // 満タン時のコライダーオフセット

        // 火炎放射(持続攻撃)によるダメージは、当たり判定が複数フレーム連続で
        // 発生し続けるため、onCollisionEnter内では即座に適用せず、
        // 「今フレーム炎に当たっているか」だけ記録し、update()内で
        // 自分のdeltaTimeを使ってフレームレートに依存しない形で適用する。
        bool flameHitThisFrame_ = false;


        // ---- 調整用パラメータ ----
        static constexpr float DRIFT_SCALE          = 22.0f;            // 海流ベクトル→速度への倍率
        static constexpr float INERTIA_RATE_PER_SEC = 0.075f;           // 慣性の強さ(小さいほど鈍い=重い動き)
        static constexpr float SPLIT_PUSH_DECAY_RATE_PER_SEC = 0.6f;    // 分裂の押し出し(splitPushVelocity_)専用の減衰レート
        static constexpr float WAVE_DAMPING         = 0.1f;             // 上下動の減衰(1.0だと船と同じ強さになる)
        static constexpr float TILT_DAMPING         = 0.1f;             // 傾きの減衰
        static constexpr float OUT_OF_BOUNDS_MARGIN = 50.0f;            // マップ端からの余裕(world単位)

        // 実測で球が見た目よりひと回り大きかったため、
        // 対角線ベースではなく最大軸ベースの半径にした上でさらにこの倍率を掛ける。
        // 1.0で「最大軸の半分」、小さくするほど控えめになる。
        static constexpr float ICEBERG_COLLIDER_RADIUS_SCALE = 0.93f;

        // 通常弾1発で溶ける量(health_を1.0とした割合)
        static constexpr float MELT_DAMAGE_PER_SHOT = 0.34f;
        // 火炎放射が当たっている間、1秒あたりに溶ける量(同上の割合/秒)
        static constexpr float MELT_DAMAGE_PER_SEC_FLAME = 0.5f;

        // 溶けても完全に潰れて見えないよう、scale_の下限を設けておく
        // (0に近づくとメッシュの見た目が破綻しやすいための保険。この値の少し下でkill()される)
        static constexpr float MIN_VISIBLE_SCALE = 0.15f;

        // 見た目のスケール(displayScale_)がhealth_由来の目標値へ近づく速さ。
        // FrameRateIndependentBlendと同じ考え方で、残り距離に比例して速度が変わる
        // (ダメージが大きいほど最初は素早く縮み、目標に近づくほど自然に減速する)。
        static constexpr float MELT_VISUAL_BLEND_RATE_PER_SEC = 3.0f;


        // ---- ティア別ダメージ倍率 ----
        // 戦略性の設計: 「割ってから攻撃する方が効率が良い」を成立させるための倍率。
        // 通常弾: 2ピースを1/2、4ピースを1/4にすると「割っても割らなくても効率は同じ(とんとん)」に
        // なってしまうため、それよりかなり効率を悪くしてある(1/6, 1/12)。
        // 火炎放射: 小1ピースまで分解しないと利きが悪い、
        //           中2ピースまで分解すればそこそこ効く
        //           大4ピースではぜんぜん効かない、という体感になるよう、
        // 通常弾よりもさらに急な落ち込み(1/8, 1/24)にしてある。
        static constexpr float SHOT_DAMAGE_MULT_SMALL   = 1.0f;
        static constexpr float SHOT_DAMAGE_MULT_MEDIUM  = 1.0f / 6.0f;
        static constexpr float SHOT_DAMAGE_MULT_LARGE   = 1.0f / 12.0f;

        static constexpr float FLAME_DAMAGE_MULT_SMALL  = 1.0f;
        static constexpr float FLAME_DAMAGE_MULT_MEDIUM = 1.0f / 8.0f;
        static constexpr float FLAME_DAMAGE_MULT_LARGE  = 1.0f / 24.0f;
    };
}
