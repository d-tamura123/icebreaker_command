#pragma once
#include <memory>
#include <vector>
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

        void update(); // F1 トグルなど
        void draw(const Shared<dxe::Camera>& camera);

        void setSampleStep(int step) { sampleStep_ = std::max(1, step); }
    private:
        std::weak_ptr<gmMapManager> map_;

        int arrowHandle_ = -1;
        bool visible_ = false;
        int sampleStep_ = 1;

        // ============================================================
        // 頂点バッチング用バッファ（"帳票"のレコード領域に相当）
        // セル単位でdxe::Meshを個別描画するのをやめ、
        // 1フレーム分の矢印ポリゴンをここに詰め込んでから
        // まとめて数回のドローコールで出力する。
        // ============================================================

        // WORD(16bit)インデックスの上限(65535)を超えないよう、
        // 1バッチあたりの最大頂点数を安全マージン込みで設定
        static constexpr size_t MAX_BATCH_VERTICES = 60000;

        std::vector<VERTEX3D> vtxBuf_;
        std::vector<WORD> idxBuf_;

        void loadResources();
        void releaseResources();
        void initializeBuffers();

        // バッファに溜まった内容を1回のドローコールで描画し、クリアする（＝改ページ）
        void flushBatch();
    };
}
