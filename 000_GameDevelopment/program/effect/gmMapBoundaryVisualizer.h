// gmMapBoundaryVisualizer.h
#pragma once
#include <memory>
#include <vector>
#include <dxe.h>
#include <DxLib.h>

namespace gm {

    class gmMapManager;

    // ------------------------------------------------------------
    // マップの外枠(移動可能範囲の境界)を、ゲーム画面上にリボンメッシュとして
    // 描画するクラス。判定処理には一切関与しない、見た目だけのクラス
    // (移動抑止はgmPlayerShip::clampToMapBounds()側で別途行う)。
    //
    // gmRouteVisualizerと生成ロジックを共有するが、以下が異なる:
    //   ・中心線はCatmull-Rom補間ではなく、マップの四辺(直線)を一定間隔で
    //     サンプリングした閉ループ(始点=終点)
    //   ・閉ループのため始点・終点のアルファフェードは行わない(常に不透明)
    //   ・海面の影リボンは無し(水平・垂直の2層のみ。平面だけだとチープで
    //     視認性も悪いため、航路と同じ十字クロス構成にした)
    //   ・専用の青系テクスチャ(outerframe_ribbon_gradient.png)を使う
    // ------------------------------------------------------------
    class gmMapBoundaryVisualizer {
    public:
        // arg1... マップデータを保持するgmMapManager(範囲を読み取るだけで、所有はしない)
        explicit gmMapBoundaryVisualizer(const std::shared_ptr<gmMapManager>& map);
        ~gmMapBoundaryVisualizer();

        // UVスクロール用の時間だけ進める(ジオメトリの再生成は行わない)
        void update(float deltaTime);

        void render(const Shared<dxe::Camera>& camera);

    private:
        // 1チャンクぶんの描画データ(水平・垂直の2枚ぶんをまとめて持つ)
        struct RibbonChunk {
            std::vector<VERTEX3D> horizontalVtx;
            std::vector<WORD>     horizontalIdx;

            std::vector<VERTEX3D> verticalVtx;
            std::vector<WORD>     verticalIdx;

            tnl::Vector3 cullCenter{ 0.0f, 0.0f, 0.0f }; // カリング判定用のチャンク中心(水平面上の位置)
        };

        void loadResources();
        void buildBoundary(const gmMapManager& map);

        // マップの四辺(閉ループ)を、ROUTE_RIBBON_SAMPLE_STEP間隔でサンプリングした中心線を作る
        std::vector<tnl::Vector2f> buildBoundaryCenterline(const gmMapManager& map) const;

        // 中心線(閉ループ)を、ROUTE_RIBBON_CHUNK_LENGTHごとにチャンク分割し、
        // 各チャンクに水平/垂直の2枚のリボンを構築してchunks_へ積む。
        void appendChunks(const std::vector<tnl::Vector2f>& centerline);

        // [beginIndex, endIndex]区間の中心線から、リボン(帯)の頂点/インデックスを構築する。
        // 水平/垂直とも、このロジックを共有し、基準オフセット軸だけ変える。
        // 閉ループのため、区間の両端(先頭・末尾)でも前後の点を正しく参照できるよう、
        // centerlineは「最後の点が最初の点と同じ座標」になっている前提で、周回して接線を求める。
        void buildRibbonSegment(
            const std::vector<tnl::Vector2f>& centerline,
            const std::vector<float>& distances,
            size_t beginIndex, size_t endIndex,
            bool useHorizontalOffsetAxis,
            std::vector<VERTEX3D>& outVtx,
            std::vector<WORD>& outIdx) const;

        std::weak_ptr<gmMapManager> map_;
        Shared<dxe::Texture> texture_;

        std::vector<RibbonChunk> chunks_;

        float scrollTime_ = 0.0f;
    };
}
