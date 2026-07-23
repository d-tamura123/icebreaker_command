#include "gmUIManager.h"
#include "gmMiniMap.h" // 実装ファイル側で実体をインクルード
#include "gmOceanFlowVisualizer.h"

namespace gm {

    gmUIManager::gmUIManager(
        const tnl::Vector2f& miniMapPos,
        std::shared_ptr<gmMapManager> map,
        std::shared_ptr<gmPlayerShip> player)
    {
        // gmMiniMap のインスタンスを生成して保持
        miniMap_ = std::make_unique<gmMiniMap>(miniMapPos, map, player);

        // gmOceanFlowVisualizer のインスタンスを生成して保持
        flowVisualizer_ = std::make_unique<gmOceanFlowVisualizer>(map);

    }

    gmUIManager::~gmUIManager()
    {
        // std::unique_ptr が自動的に解放するため、明示的なdeleteは不要です
    }

    void gmUIManager::update(float dt)
    {
        if (flowVisualizer_) {
            flowVisualizer_->update();
        }

        if (miniMap_) {
            miniMap_->update(dt); // ミニマップの更新
        }

        // 今後、他のUIのupdate処理が増えたらここに追記します
    }

    void gmUIManager::renderTerrainIntegration(const Shared<dxe::Camera>& camera)
    {
        // Note: 描画順が雑実装。複雑化した場合はレイヤーの実装を検討すること
        if (flowVisualizer_) {
            flowVisualizer_->draw(camera);
        }
        // 今後、他のUIのdraw処理が増えたらここに追記します
    }

    void gmUIManager::render(const Shared<dxe::Camera>& camera)
    {

        // 【注意】DxLibは2D描画関数(DrawExtendGraph等)を呼ぶと
        // 内部の3Dカメラ・射影変換が無効化されるため、
        // 3Dワールド座標を扱う描画は必ず2D描画より先に行うこと。
        // またはgm::ApplyCamera3D(camera)の対策すること
        if (miniMap_) {

            miniMap_->draw(); // ミニマップの描画
        }

        // 今後、他のUIのdraw処理が増えたらここに追記します
    }
}
