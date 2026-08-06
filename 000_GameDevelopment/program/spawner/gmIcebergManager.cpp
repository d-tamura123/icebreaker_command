#include "gmIcebergManager.h"
#include "../map/gmMapManager.h"
#include "../object/gmWaterPlane.h"
#include "../mesh_ex/gmMeshEX.h"
#include "../collision/gmCollisionSystem.h"
#include <dxe.h>

namespace gm {

    gmIcebergManager::gmIcebergManager(
        const std::shared_ptr<gmMapManager>& map,
        const std::shared_ptr<gmWaterPlane>& water,
        const std::vector<std::string>& crystalPaths,
        const std::string& texturePath,
        const std::shared_ptr<gmCollisionSystem>& collisionSystem)
        : map_(map)
        , water_(water)
        , crystalPaths_(crystalPaths)
        , collisionSystem_(collisionSystem)
    {
        texture_ = dxe::Texture::CreateFromFile(texturePath);

        // 氷山メッシュ(大/中/小すべて)はここでまとめて生成しておき、以後使い回す。
        // Note:
        // MeshEX::CreateIceChunk(Pieces)は内部でモデルロード+X形式変換を伴う重い処理のため、
        // スポーンや分裂のたびに毎回呼ぶと負荷/安定性の両面で問題になる。
        // そのため「大」のピース配置を起動時に1回だけ生成し、そこから間引いた
        // 「中」「小」もまとめてこの場で焼成しておく(分裂時は再焼成せず取り出すだけにする)。
        families_.reserve(MESH_TEMPLATE_COUNT);
        for (int i = 0; i < MESH_TEMPLATE_COUNT; ++i) {
            MeshEX::IceChunkPieces pieces = MeshEX::CreateIceChunkPieces(
                crystalPaths_, texture_, PIECE_BASE_SIZE, LARGE_PIECE_COUNT, -1);

            if (!pieces.baseMesh || (int)pieces.pieceMatrices.size() < LARGE_PIECE_COUNT) {
                continue;
            }

            IcebergFamily family;

            // 大: 全4ピース
            family.largeMesh = dxe::Mesh::CreateStaticMeshGroupMV(pieces.baseMesh, pieces.pieceMatrices);
            if (!finalizeBakedMesh(family.largeMesh)) continue;

            // 中: {0,1}と{2,3}に固定で半分ずつ分ける(単純固定ルール)
            bool mediumOk = true;
            for (int m = 0; m < 2 && mediumOk; ++m) {
                std::vector<tnl::Matrix> subset = {
                    pieces.pieceMatrices[m * 2 + 0],
                    pieces.pieceMatrices[m * 2 + 1]
                };
                family.mediumMesh[m] = dxe::Mesh::CreateStaticMeshGroupMV(pieces.baseMesh, subset);
                mediumOk = finalizeBakedMesh(family.mediumMesh[m]);
            }
            if (!mediumOk) continue;

            // 小: ピース単体×4(中のさらに半分。インデックスはピース番号にそのまま対応)
            bool smallOk = true;
            for (int s = 0; s < 4 && smallOk; ++s) {
                std::vector<tnl::Matrix> subset = { pieces.pieceMatrices[s] };
                family.smallMesh[s] = dxe::Mesh::CreateStaticMeshGroupMV(pieces.baseMesh, subset);
                smallOk = finalizeBakedMesh(family.smallMesh[s]);
            }
            if (!smallOk) continue;

            families_.push_back(family);
        }

        maxEntities_ = 64;                  // 同時存在数の上限
        spawnTimer_ = rollNextInterval();   // 開始直後にまとめて湧かないよう最初の間隔を設定
    }

    // ------------------------------------------------------------
    // 焼成したメッシュのハンドルが正しいか検証し、ライティング設定を仕込む。
    // 無効なら引数のmeshをnullptrにして呼び出し側へ知らせる。
    // ------------------------------------------------------------
    bool gmIcebergManager::finalizeBakedMesh(Shared<dxe::Mesh>& mesh)
    {
        if (!mesh || mesh->getDxMvHdl() == 0) {
#ifdef _DEBUG
            OutputDebugStringA("gmIcebergManager: mesh bake FAILED (invalid handle)\n");
#endif
            mesh = nullptr;
            return false;
        }

        mesh->setDefaultLightEnable(true);
        return true;
    }

    // ------------------------------------------------------------
    // 南端の帯状エリアから、陸地でないセルを探して1個生成する
    // ------------------------------------------------------------
    void gmIcebergManager::trySpawn()
    {
        if (!map_) return;
        if (families_.empty()) return; // 有効なファミリーが1つも無ければ何もしない

        int gx = 0;
        int gy = 0;
        bool found = false;

        for (int i = 0; i < SPAWN_LAND_RETRY; ++i) {
            gx = tnl::GetRandomDistribution<int>(SPAWN_COL_MARGIN, MAP_CHIP_WIDTH - 1 - SPAWN_COL_MARGIN);
            gy = tnl::GetRandomDistribution<int>(SPAWN_ROW_MIN, SPAWN_ROW_MAX);

            if (!map_->IsLand(gx, gy)) {
                found = true;
                break;
            }
        }

        // 陸地ばかりで候補が見つからなくても、次のタイマーで再挑戦すればよいので無視する
        if (!found) return;

        const float worldX = gx * CELL_SIZE + CELL_SIZE * 0.5f;
        const float worldZ = -gy * CELL_SIZE - CELL_SIZE * 0.5f;

        // 初期Y座標はその場の波高に合わせておく(固定値だと出現時に不自然にポップする)
        float initialY = 0.0f;
        if (water_) {
            initialY = water_->sampleHeight({ worldX, 0.0f, worldZ }, 0.0f);
        }

        // 流氷生成(新規スポーンは常に「大」)
        // あらかじめ用意したファミリーからランダムに1つ選ぶ(毎回生成しない)
        const int familyIndex = tnl::GetRandomDistribution<int>(0, (int)families_.size() - 1);
        auto& family = families_[familyIndex];

        auto iceberg = std::make_shared<gmIceberg>(
            "iceberg", tnl::Vector3(worldX, initialY, worldZ), family.largeMesh);
        iceberg->setMap(map_);
        iceberg->setWater(water_);
        iceberg->setFamilyIndex(familyIndex);
        iceberg->setTier(gmIceberg::Tier::Large);

        entities_.push_back(iceberg);

        // ------------------------------------------------------------------------
        // 衝突システムへ登録
        //  ※ weak_ptrとして保持されるので、氷山がkill()されて
        //     entities_から取り除かれれば、衝突システム側も自動的に対象から外れる
        // ------------------------------------------------------------------------
        if (collisionSystem_) {
            collisionSystem_->registerObject(iceberg);
        }
    }

    // ------------------------------------------------------------
    // 分裂待ち(hasPendingSplitRequest()==true)の氷山を拾って、
    // 実際に1段階下のティアの氷山を2個生成する。
    //
    // 先に「誰が分裂待ちか」を全部集めてから処理するのは、
    // entities_(継承元のベースクラスが持つ配列)へ新しい氷山をaddEntity()で
    // 追加する際、その場でentities_をイテレート中に書き換えると
    // イテレータが無効になりうるため。
    // ------------------------------------------------------------
    void gmIcebergManager::onPostUpdate(float /*deltaTime*/)
    {
        if (families_.empty()) return;

        struct PendingSplit {
            std::shared_ptr<gmIceberg> source;
            tnl::Vector3 pushDir;
        };
        std::vector<PendingSplit> pendings;

        for (auto& e : entities_) {
            if (e && e->isAlive() && e->hasPendingSplitRequest()) {
                pendings.push_back({ e, e->consumePendingSplitDirection() });
            }
        }

        for (auto& p : pendings) {
            spawnSplitFragments(*p.source, p.pushDir);
            p.source->kill();
        }
    }

    // ------------------------------------------------------------
    // 分裂元(source)のファミリー・ティアから、1段階下のティアの
    // 事前焼成済みメッシュを取り出して、押し出し方向(pushDir)の
    // 正負へ分かれるように2個の氷山を生成する。
    // ------------------------------------------------------------
    void gmIcebergManager::spawnSplitFragments(const gmIceberg& source, const tnl::Vector3& pushDir)
    {
        const int familyIndex = source.getFamilyIndex();
        if (familyIndex < 0 || familyIndex >= (int)families_.size()) return;

        auto& family = families_[familyIndex];

        Shared<dxe::Mesh> nextMesh[2] = { nullptr, nullptr };
        gmIceberg::Tier nextTier = gmIceberg::Tier::Small;
        int nextMediumIndex[2] = { -1, -1 };

        switch (source.getTier()) {
        case gmIceberg::Tier::Large:
            nextTier = gmIceberg::Tier::Medium;
            nextMesh[0] = family.mediumMesh[0];
            nextMesh[1] = family.mediumMesh[1];
            nextMediumIndex[0] = 0;
            nextMediumIndex[1] = 1;
            break;

        case gmIceberg::Tier::Medium: {
            const int m = source.getMediumIndex();
            if (m != 0 && m != 1) return; // 想定外のデータ(生成経路が正しければ起こらない)

            nextTier = gmIceberg::Tier::Small;
            nextMesh[0] = family.smallMesh[m * 2 + 0];
            nextMesh[1] = family.smallMesh[m * 2 + 1];
            break;
        }

        case gmIceberg::Tier::Small:
        default:
            // 小は割る弾に対して無反応(onCollisionEnter側で分裂要求を出さない)なので、通常ここへは来ない
            return;
        }

        const float pushSign[2] = { 1.0f, -1.0f };      // バキッと割れるように反発させる
        for (int i = 0; i < 2; ++i) {
            if (!nextMesh[i]) continue;

            auto fragment = std::make_shared<gmIceberg>(
                "iceberg_fragment", source.getPosition(), nextMesh[i]);
            fragment->setMap(map_);
            fragment->setWater(water_);
            fragment->setFamilyIndex(familyIndex);
            fragment->setTier(nextTier);
            fragment->setMediumIndex(nextMediumIndex[i]);
            fragment->setRotation(source.getRotation());
            fragment->setSplitPushVelocity(pushDir * (SPLIT_PUSH_SPEED * pushSign[i]));
            fragment->setSpinSpeed(tnl::GetRandomDistribution<float>(-0.15f, 0.15f) * SPLIT_SPIN_BOOST);
            fragment->setCollisionGracePeriod(SPLIT_COLLISION_GRACE_SEC);

            addEntity(fragment);
            if (collisionSystem_) {
                collisionSystem_->registerObject(fragment);
            }
        }
    }

    float gmIcebergManager::rollNextInterval() const
    {
        return tnl::GetRandomDistribution<float>(SPAWN_INTERVAL_MIN, SPAWN_INTERVAL_MAX);
    }
}
