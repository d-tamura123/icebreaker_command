#pragma once
#include <memory>
#include <vector>
#include <climits>       // INT_MIN
#include <dxe.h>
#include <DxLib.h>       // VERTEX3D, WORD を使うため
#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>

namespace gm {
    class gmMapManager;

    class gmOceanFlowVisualizer {
    public:
        explicit gmOceanFlowVisualizer(std::shared_ptr<gmMapManager> map);
        ~gmOceanFlowVisualizer();

        void update(const Shared<dxe::Camera>& camera);
        void draw(const Shared<dxe::Camera>& camera);

        void setSampleStep(int step) { sampleStep_ = std::max(1, step); }
    private:
        std::weak_ptr<gmMapManager> map_;

        int arrowHandle_ = -1;
        bool visible_ = false;
        int sampleStep_ = 1;

        // ============================================================
        // 頂点バッチング用バッファ
        // セル単位でdxe::Meshを個別描画するのをやめ、
        // 1フレーム分の矢印ポリゴンをここに詰め込んでから
        // まとめて数回のドローコールで出力する。
        // ============================================================

        // WORD(16bit)インデックスの上限(65535)を超えないよう、
        // 1バッチあたりの最大頂点数を安全マージン込みで設定
        static constexpr size_t MAX_BATCH_VERTICES = 60000;

        std::vector<VERTEX3D> vtxBuf_;
        std::vector<WORD> idxBuf_;

        // ============================================================
        // スロットリング(段階的更新)用
        // マップ/海流データは起動時ロード後は不変のため、
        // 「プレイヤーが今いるグリッドセルが変わったか」だけで
        // 再構築の要否を判定できる（＝距離しきい値より正確）。
        // それ以外のフレームは前回生成したバッチをそのまま描画に使い回す。
        // ============================================================
        bool hasCachedBatches_ = false;
        int lastCenterGX_ = INT_MIN;
        int lastCenterGY_ = INT_MIN;

        // 生成済みバッチ(複数ドローコール分)のキャッシュ
        std::vector<std::vector<VERTEX3D>> vtxBatches_;
        std::vector<std::vector<WORD>> idxBatches_;

        void loadResources();
        void releaseResources();
        void initializeBuffers();

        // グリッドセルが変わっていたら、マップ+海流データからバッチを再構築する
        // （CPU側ロジック。DxLibの描画APIは一切呼ばない）
        void rebuildBatches(
            const gmMapManager& map,
            const tnl::Vector3& camPos,
            const tnl::Vector3& camForward,
            int centerGX,
            int centerGY);

        // バッファに溜まった内容をキャッシュ(vtxBatches_/idxBatches_)へ積み、クリアする（＝改ページ）
        void flushBatch();

        // キャッシュ済みバッチを実際にドローコールへ流す（毎フレーム実行、軽量）
        void drawCachedBatches();
    };
}
