// gmRouteVisualizer.h
#pragma once
#include <memory>
#include <vector>
#include <dxe.h>
#include <DxLib.h>

namespace gm {

    class gmMapManager;

    // ------------------------------------------------------------
    // NPC交易船の航路(gmMapManager::RouteInfo)を、ゲーム画面上にリボンメッシュとして
    // 描画するクラス。判定処理には一切関与しない、見た目だけのクラス。
    //
    // 【構成(1航路あたり3枚のリボン)】
    //   ・空中・水平リボン: 海面(y=0)からROUTE_RIBBON_ALTITUDE離れた高さに、
    //     モノレールのレールのように一定高度で浮かせる(波の高さは追わない)
    //   ・空中・垂直リボン: 水平リボンと同じ中心線上に、90度立てて交差させる
    //     (カメラが水面すれすれの低い角度になっても、どちらか一方は見える幅を保つため)
    //   ・海面・影リボン: 水平方向のみ、上記2枚より細く・暗く・薄く、海面ギリギリの高さに
    //     配置する(位置確認用。空中の2枚だけだと、実際にどこの海面上を航路が通っているか
    //     把握しづらいための補助)
    //
    // 3枚とも同じ生成ロジック(中心線+基準オフセット軸)を使い回し、基準ベクトルと
    // 太さ・明るさのパラメータだけを変えて作る。
    //
    // 【ジオメトリの生成手順】
    //   1. gmMapManager::GetRouteWorldPoints()でウェイポイントを取得
    //   2. 遠心的(centripetal)Catmull-Rom補間で区間ごとのベジェ制御点を求める
    //      (IC_ExcelMapTool側 modRouteExporter.bas の遠心的Catmull-Rom補間と同じ式。
    //       手入力の点列は間隔が不揃いになりやすく、一様パラメータ化だとループ状に
    //       暴れることがあるため、Excel側プレビューと同じ方式で統一している)
    //   3. 各区間を一定間隔(ROUTE_RIBBON_SAMPLE_STEP)でサンプリングし、密なポリラインにする
    //   4. 各サンプル点で、中心から基準オフセット軸方向に半幅ぶん離れた2頂点を作る
    //   5. 隣接するサンプル点同士を四角形(三角形2枚)でつなぐ
    //   6. 始点(S)・終点(G)付近はアルファをフェードさせる
    //   7. 一定の長さ(ROUTE_RIBBON_CHUNK_LENGTH)ごとにチャンク分割し、
    //      各チャンクの中心点をカリング判定に使う(gmIceberg::render()と同じ距離カリング)
    //
    // これらは全て起動時(construct時)に1回だけ行い、以後は生成済みの頂点/インデックス配列を
    // そのまま描画に使い回す(航路データは実行中に変化しないため)。
    // ------------------------------------------------------------
    class gmRouteVisualizer {
    public:
        // arg1... 航路データを保持するgmMapManager(routesを読み取るだけで、所有はしない)
        explicit gmRouteVisualizer(const std::shared_ptr<gmMapManager>& map);
        ~gmRouteVisualizer();

        // UVスクロール用の時間だけ進める(ジオメトリの再生成は行わない)
        void update(float deltaTime);

        void render(const Shared<dxe::Camera>& camera);

    private:
        // 1チャンクぶんの描画データ(空中・水平/垂直、海面・影の3枚ぶんをまとめて持つ)
        struct RibbonChunk {
            std::vector<VERTEX3D> horizontalVtx;
            std::vector<WORD>     horizontalIdx;

            std::vector<VERTEX3D> verticalVtx;
            std::vector<WORD>     verticalIdx;

            std::vector<VERTEX3D> shadowVtx;
            std::vector<WORD>     shadowIdx;

            tnl::Vector3 cullCenter{ 0.0f, 0.0f, 0.0f }; // カリング判定用のチャンク中心(水平面上の位置)
        };

        void loadResources();
        void buildAllRoutes(const gmMapManager& map);
        void buildRoute(const std::vector<tnl::Vector2f>& waypointsWorld);

        // 遠心的Catmull-Rom補間で、waypointsWorldを密なポリライン(中心線)へ変換する
        std::vector<tnl::Vector2f> sampleCenterline(const std::vector<tnl::Vector2f>& waypointsWorld) const;

        // 中心線1本ぶんから、チャンク分割済みのリボン(水平/垂直/影)をchunks_へ積む
        void appendChunks(const std::vector<tnl::Vector2f>& centerline);

        // 1チャンクぶんの区間([beginIndex, endIndex])から、指定した基準オフセット軸・太さ・
        // 明るさでリボンの頂点/インデックスを構築する(水平/垂直/影、共通のロジック)。
        // arg1... 中心線全体(centerline)
        // arg2... 累積距離(distances、centerlineと同じ添字で対応する、フェード・UV計算に使う)
        // arg3... 区間の開始/終了インデックス(centerlineの添字)
        // arg4... リボンのY座標オフセット(海面y=0からの高さ)
        // arg5... true: 基準オフセット軸を「中心線の接線から求めた水平右方向」にする(水平リボン用)
        //         false: 基準オフセット軸を「ワールド上方向(0,1,0)固定」にする(垂直リボン用)
        // arg6... リボンの全幅(world単位)
        // arg7... 頂点色に掛ける明度スケール(0〜255)
        // arg8... 頂点色に掛けるアルファスケール(0〜255。フェード計算後の値にさらに掛かる)
        // arg9... [出力] 頂点配列
        // arg10.. [出力] インデックス配列
        void buildRibbonSegment(
            const std::vector<tnl::Vector2f>& centerline,
            const std::vector<float>& distances,
            size_t beginIndex, size_t endIndex,
            float heightOffset,
            bool useHorizontalOffsetAxis,
            float width,
            uint8_t colorScale,
            uint8_t alphaScale,
            std::vector<VERTEX3D>& outVtx,
            std::vector<WORD>& outIdx) const;

        std::weak_ptr<gmMapManager> map_;
        Shared<dxe::Texture> texture_;

        std::vector<RibbonChunk> chunks_;

        float scrollTime_ = 0.0f;
    };
}
