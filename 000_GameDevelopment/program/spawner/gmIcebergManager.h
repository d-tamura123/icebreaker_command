#pragma once
#include <string>
#include <vector>
#include <memory>
#include "gmSpawnerManagerBase.h"
#include "../object/gmIceberg.h"
#include "../gmGameConfig.h"

namespace gm {

    class gmMapManager;
    class gmWaterPlane;
    class gmCollisionSystem;

    // ------------------------------------------------------------
    // 氷山のスポーンと一元管理
    // マップ南端の帯状エリアからランダムな間隔・位置で氷山を生成する。
    // ------------------------------------------------------------
    class gmIcebergManager : public gmSpawnerManagerBase<gmIceberg> {
    public:
        gmIcebergManager(
            const std::shared_ptr<gmMapManager>& map,
            const std::shared_ptr<gmWaterPlane>& water,
            const std::vector<std::string>& crystalPaths,
            const std::string& texturePath,
            const std::shared_ptr<gmCollisionSystem>& collisionSystem);

    protected:
        void trySpawn() override;
        float rollNextInterval() const override;

        // 毎フレーム、分裂待ちの氷山を拾って実際に生成する
        void onPostUpdate(float deltaTime) override;

    private:
        // 氷山1系統(ファミリー)ぶんの、大/中/小すべてのメッシュを事前に焼成したもの。
        // 中・小は「大」のピース配置をそのまま半分/4分の1に間引いたものなので、
        // 分裂しても元のピースの位置がワープせずに引き継がれる。
        // Note: 中2個・小4個は、必ずインデックスが対応する
        //       (mediumMesh[0]の半分がsmallMesh[0]とsmallMesh[1]、
        //        mediumMesh[1]の半分がsmallMesh[2]とsmallMesh[3])。
        struct IcebergFamily {
            Shared<dxe::Mesh> largeMesh;       // 大(4ピース)
            Shared<dxe::Mesh> mediumMesh[2];   // 中(2ピース)×2
            Shared<dxe::Mesh> smallMesh[4];    // 小(1ピース)×4
        };

        // 分裂リクエストを実際に処理し、1段階下のティアの氷山を2個生成する
        void spawnSplitFragments(const gmIceberg& source, const tnl::Vector3& pushDir);

        // メッシュが正しく焼成できたか検証し、ライティング設定を仕込む共通処理
        static bool finalizeBakedMesh(Shared<dxe::Mesh>& mesh);

        std::shared_ptr<gmMapManager> map_;
        std::shared_ptr<gmWaterPlane> water_;
        std::vector<std::string> crystalPaths_;
        Shared<dxe::Texture> texture_;

        // 衝突システム
        std::shared_ptr<gmCollisionSystem> collisionSystem_;

        // 氷山ファミリー(起動時にまとめて生成し、以後使い回す。分裂時も再焼成しない)
        std::vector<IcebergFamily> families_;
        static constexpr int MESH_TEMPLATE_COUNT = 5; // 見た目のバリエーション(ファミリー)数

        // 大(4ピース)の基準サイズ・ピース数
        // Note: 1ピースの基本サイズは体感で小さく見えたため、以前の1.0fから拡大している。
        //       実機で見ながら調整した値。
        static constexpr float PIECE_BASE_SIZE = 6.0f;
        static constexpr int   LARGE_PIECE_COUNT = 4;

        // ---- 分裂時の押し出し・自転 ----
        static constexpr float SPLIT_PUSH_SPEED = 120.0f;      // 押し出し初速(world単位/秒)。以後は既存の慣性減衰に任せる
        static constexpr float SPLIT_SPIN_BOOST = 0.5f;        // 分裂直後だけ自転を強める倍率

        // 分裂直後、兄弟フラグメント同士の押し戻し判定を無視する猶予秒数。
        // SPLIT_PUSH_SPEEDで離れるのに十分な時間を確保する(短すぎると押し戻しループが再発する)。
        static constexpr float SPLIT_COLLISION_GRACE_SEC = 3.0f;

        // ---- 生成範囲(グリッド単位。南端の帯状エリア) ----
        // worldZ = -y * CELL_SIZE のため、"南端"は行番号(y)が大きい側
        static constexpr int SPAWN_ROW_MIN = MAP_CHIP_HEIGHT - 3;  // 南端から3セル分の帯
        static constexpr int SPAWN_ROW_MAX = MAP_CHIP_HEIGHT - 2;  // 端ぎりぎりは避ける
        static constexpr int SPAWN_COL_MARGIN = 4;                 // 東西の端だけ避ける

        static constexpr float SPAWN_INTERVAL_MIN = 22.5f;   // 秒
        static constexpr float SPAWN_INTERVAL_MAX = 45.0f;

        static constexpr int SPAWN_LAND_RETRY = 8; // 候補セルが陸地だった場合のリトライ回数
    };
}
