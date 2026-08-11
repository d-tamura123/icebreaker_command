// gmTradeShipManager.cpp
#include "gmTradeShipManager.h"
#include "../map/gmMapManager.h"
#include "../wallet/gmWallet.h"
#include "../object/gmWaterPlane.h"
#include "../collision/gmCollisionSystem.h"
#include "../util/gmRouteCenterlineUtil.h"
#include "../gmGameConfig.h"
#include <dxe.h>

namespace gm {

    gmTradeShipManager::gmTradeShipManager(
        const std::shared_ptr<gmMapManager>& map,
        const std::shared_ptr<gmWaterPlane>& water,
        const std::shared_ptr<gmCollisionSystem>& collisionSystem,
        const std::shared_ptr<gmWallet>& wallet
        )
        : map_(map)
        , water_(water)
        , collisionSystem_(collisionSystem)
        , wallet_(wallet)
    {
        texture_ = dxe::Texture::CreateFromFile(TRADE_SHIP_TEXTURE_FILE_PATH);
        normalMapTexture_ = dxe::Texture::CreateFromFile(TRADE_SHIP_NORMAL_MAP_FILE_PATH);

        maxEntities_ = TRADE_SHIP_MAX_ENTITIES;
        spawnTimer_ = rollNextInterval(); // 開始直後にまとめて湧かないよう最初の間隔を設定
    }

    // ------------------------------------------------------------
    // 読み込めている航路の中からランダムに1本選び、
    // その始点(S)から交易船を1隻スポーンする。
    // 
    // 「ルートごとに個別のスポナーを持たない」設計のため、
    // この関数だけが航路の本数(GetRouteCount())に依存する箇所であり、
    // Excel側でRoute_3等を追加してもここを含め一切変更不要になる。
    // ------------------------------------------------------------
    void gmTradeShipManager::trySpawn()
    {
        if (!map_) return;

        const size_t routeCount = map_->GetRouteCount();
        if (routeCount == 0) return;

        const size_t routeIndex = static_cast<size_t>(
            tnl::GetRandomDistribution<int>(0, static_cast<int>(routeCount) - 1));

        const std::vector<tnl::Vector2f> waypointsWorld = map_->GetRouteWorldPoints(routeIndex);
        if (waypointsWorld.size() < 2) return; // 異常データ(S・Gすら無い)は無視して次回の抽選に任せる

        // 見た目のリボン(gmRouteVisualizer)と全く同じ計算式・同じサンプリング間隔で
        // 中心線を求める。「見た目の航路」と「実際に船が通る経路」を一致させるため
        // (util/gmRouteCenterlineUtil.h参照)。
        const std::vector<tnl::Vector2f> centerline2D =
            SampleRouteCenterline(waypointsWorld, ROUTE_RIBBON_SAMPLE_STEP);
        if (centerline2D.size() < 2) return;

        std::vector<tnl::Vector3> centerline3D;
        centerline3D.reserve(centerline2D.size());
        for (const auto& p : centerline2D) {
            centerline3D.emplace_back(p.x, 0.0f, p.y); // Vector2f(x,z) -> Vector3(x,y=0,z)
        }

        auto ship = std::make_shared<gmTradeShip>("trade_ship", centerline3D);

        ship->create(TRADE_SHIP_MESH_FILE_PATH, TRADE_SHIP_MESH_SCALE);
        if (auto mesh = ship->getMesh()) {

            mesh->setTexture(texture_);

            // ノーマルマップ。
            // TODO: 保留中。dxe開発者にMV1のノーマルマップの扱い・対応状況を確認中のため、
            // 結論が出るまでコメントアウトしておく。
            // (調査過程のメモ: MV1SetTextureGraphHandle()の第2引数は「マテリアルの種類
            //  (ディフューズ/法線等)」ではなく「モデル内部のテクスチャ一覧の何番目か」を表す
            //  通し番号らしいという情報はあるが、dxe::Mesh::BUMPが正しい値かは未確定。
            //  ライティング周りの検証(gmKyleDebugger側)も原因切り分けの決め手にならず保留。)
//            if (normalMapTexture_) {
//                // mesh->setTexture(normalMapTexture_, dxe::Mesh::BUMP);
//                // mesh->setTexture(normalMapTexture_, 2);
//            }

            // プレイヤー船と見分けやすいよう色味を変える(交易船=金のコンセプト。TODO参照)
            mesh->setMtrlDiffuse(tnl::Vector3(
                TRADE_SHIP_TINT_COLOR_R, TRADE_SHIP_TINT_COLOR_G, TRADE_SHIP_TINT_COLOR_B));
        }
        ship->setWater(water_);
        ship->setupManualCollider(TRADE_SHIP_COLLIDER_RADIUS, TRADE_SHIP_COLLIDER_LENGTH, 0.0f);

        // 終点到達時の資金報酬。基準額×到着時のHP比率(ダメージを受けているほど減額される)。
        std::weak_ptr<gmWallet> walletWeak = wallet_;
        ship->setOnArrivedCallback([walletWeak](float hpRatio) {
            if (auto wallet = walletWeak.lock()) {
                const int reward = static_cast<int>(TRADE_SHIP_ARRIVAL_REWARD_BASE * hpRatio);
                wallet->addFunds(reward);
            }
        });

        entities_.push_back(ship);

        // ------------------------------------------------------------------------
        // 衝突システムへ登録(weak_ptrとして保持されるため、船がkill()されて
        // entities_から取り除かれれば、衝突システム側も自動的に対象から外れる)
        // ------------------------------------------------------------------------
        if (collisionSystem_) {
            collisionSystem_->registerObject(ship);
        }
    }

    float gmTradeShipManager::rollNextInterval() const
    {
        return tnl::GetRandomDistribution<float>(TRADE_SHIP_SPAWN_INTERVAL_MIN, TRADE_SHIP_SPAWN_INTERVAL_MAX);
    }

}
