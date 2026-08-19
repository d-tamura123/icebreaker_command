// gmMapBoundaryVisualizer.cpp
#include "gmMapBoundaryVisualizer.h"
#include "../map/gmMapManager.h"
#include "../gmGameConfig.h"
#include "../util/gmRenderUtil.h"
#include <DxLib.h>
#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>
#include <cmath>

namespace gm {

    gmMapBoundaryVisualizer::gmMapBoundaryVisualizer(const std::shared_ptr<gmMapManager>& map)
        : map_(map)
    {
        loadResources();

        if (auto m = map_.lock()) {
            buildBoundary(*m);
        }
    }

    gmMapBoundaryVisualizer::~gmMapBoundaryVisualizer() = default;

    void gmMapBoundaryVisualizer::loadResources()
    {
        texture_ = dxe::Texture::CreateFromFile(GRAPHICS_FILE_PATH__MAP_BOUNDARY_RIBBON);
    }

    void gmMapBoundaryVisualizer::buildBoundary(const gmMapManager& map)
    {
        chunks_.clear();

        std::vector<tnl::Vector2f> centerline = buildBoundaryCenterline(map);
        appendChunks(centerline);
    }

    // ------------------------------------------------------------
    // マップの四辺(左上原点から時計回り)を、ROUTE_RIBBON_SAMPLE_STEP間隔で
    // サンプリングした閉ループの中心線を作る(直線なので補間曲線は不要)。
    // 最後の点は始点と同じ座標にして、閉じたループにする。
    // 
    // Note: ワールドZ座標は、2D→3D変換の都合(gmMapManager::GetPlayerStartWorld()等参照)により、
    // 0〜-h(マイナス方向)の範囲になる。Xはそのままプラス方向(0〜w)。
    // ------------------------------------------------------------
    std::vector<tnl::Vector2f> gmMapBoundaryVisualizer::buildBoundaryCenterline(const gmMapManager& map) const
    {
        const float w = static_cast<float>(MAP_CHIP_WIDTH) * CELL_SIZE;
        const float h = static_cast<float>(MAP_CHIP_HEIGHT) * CELL_SIZE;

        const tnl::Vector2f corners[5] = {
            { 0.0f, 0.0f },
            { w,    0.0f },
            { w,   -h    },
            { 0.0f,-h    },
            { 0.0f, 0.0f }, // 閉じるため、始点をもう一度
        };

        std::vector<tnl::Vector2f> centerline;
        for (int edge = 0; edge < 4; ++edge) {
            const tnl::Vector2f& a = corners[edge];
            const tnl::Vector2f& b = corners[edge + 1];
            const float edgeLength = (b - a).length();
            const int stepCount = std::max(1, static_cast<int>(edgeLength / ROUTE_RIBBON_SAMPLE_STEP));

            for (int s = 0; s < stepCount; ++s) {
                const float t = static_cast<float>(s) / static_cast<float>(stepCount);
                centerline.push_back(a + (b - a) * t);
            }
        }
        centerline.push_back(corners[4]); // 最後に始点を追加して閉じる

        return centerline;
    }

    // ------------------------------------------------------------
    // 中心線(閉ループ)を、ROUTE_RIBBON_CHUNK_LENGTHごとにチャンク分割し、
    // 各チャンクに水平/垂直の2枚のリボンを構築してchunks_へ積む。
    // 隣接チャンクは境界の点を共有するため、チャンクの継ぎ目に隙間はできない。
    // ------------------------------------------------------------
    void gmMapBoundaryVisualizer::appendChunks(const std::vector<tnl::Vector2f>& centerline)
    {
        const size_t pointCount = centerline.size();
        if (pointCount < 2) {
            return;
        }

        // 累積距離(UV計算に使う)
        std::vector<float> distances(pointCount);
        distances[0] = 0.0f;
        for (size_t i = 1; i < pointCount; ++i) {
            distances[i] = distances[i - 1] + (centerline[i] - centerline[i - 1]).length();
        }

        size_t chunkBegin = 0;
        for (size_t i = 1; i < pointCount; ++i) {
            const bool isLastPoint = (i == pointCount - 1);
            const float lengthSinceChunkBegin = distances[i] - distances[chunkBegin];

            if (lengthSinceChunkBegin < ROUTE_RIBBON_CHUNK_LENGTH && !isLastPoint) {
                continue; // まだチャンクを閉じる長さに達していない
            }

            const size_t chunkEnd = i;

            RibbonChunk chunk;

            // 水平リボン(接線から求めた水平方向を基準オフセット軸にする)
            buildRibbonSegment(centerline, distances, chunkBegin, chunkEnd,
                /*useHorizontalOffsetAxis=*/true,
                chunk.horizontalVtx, chunk.horizontalIdx);

            // 垂直リボン(ワールド上方向固定を基準オフセット軸にする。水平リボンと十字に交差)
            buildRibbonSegment(centerline, distances, chunkBegin, chunkEnd,
                /*useHorizontalOffsetAxis=*/false,
                chunk.verticalVtx, chunk.verticalIdx);

            const tnl::Vector2f& midPoint = centerline[(chunkBegin + chunkEnd) / 2];
            chunk.cullCenter = tnl::Vector3(midPoint.x, ROUTE_RIBBON_ALTITUDE, midPoint.y);

            chunks_.push_back(std::move(chunk));

            chunkBegin = chunkEnd; // 次のチャンクは、この境界点を共有して始める(継ぎ目の隙間防止)
        }
    }

    // ------------------------------------------------------------
    // [beginIndex, endIndex]区間の中心線から、リボン(帯)の頂点/インデックスを構築する。
    // gmRouteVisualizerと異なり閉ループのため、始点・終点のアルファフェードは行わない
    // (常に不透明)。また、区間の両端でも周回して前後の点を参照し、接線を正しく求める
    // (centerlineの最後の点は最初の点と同じ座標になっている前提)。
    // ------------------------------------------------------------
    void gmMapBoundaryVisualizer::buildRibbonSegment(
        const std::vector<tnl::Vector2f>& centerline,
        const std::vector<float>& distances,
        size_t beginIndex, size_t endIndex,
        bool useHorizontalOffsetAxis,
        std::vector<VERTEX3D>& outVtx,
        std::vector<WORD>& outIdx) const
    {
        outVtx.clear();
        outIdx.clear();

        const float halfWidth = ROUTE_RIBBON_WIDTH * 0.5f;
        const size_t pointCount = centerline.size();

        const COLOR_U8 color = GetColorU8(255, 255, 255, 255); // 閉ループのためフェード無し、常に不透明

        for (size_t i = beginIndex; i <= endIndex; ++i) {
            // 接線(閉ループのため、両端でも周回して前後の点を参照する)
            const tnl::Vector2f& prevPoint = (i == 0) ? centerline[pointCount - 2] : centerline[i - 1];
            const tnl::Vector2f& nextPoint = (i == pointCount - 1) ? centerline[1] : centerline[i + 1];
            tnl::Vector2f tangent2D = nextPoint - prevPoint;
            tangent2D.normalize();

            const tnl::Vector3 center(centerline[i].x, ROUTE_RIBBON_ALTITUDE, centerline[i].y);

            // 基準オフセット軸:
            //   水平リボン: 接線を水平面内で90度回転させた「右方向」
            //   垂直リボン: ワールド上方向(0,1,0)固定
            tnl::Vector3 offsetAxis;
            if (useHorizontalOffsetAxis) {
                const tnl::Vector3 tangent3D(tangent2D.x, 0.0f, tangent2D.y);
                offsetAxis = tnl::Vector3::Normalize(tnl::Vector3::Cross(tnl::Vector3::up(), tangent3D));
            }
            else {
                offsetAxis = tnl::Vector3::up();
            }

            const float u = distances[i] / ROUTE_RIBBON_UV_REPEAT_LENGTH;

            VERTEX3D left{};
            left.pos = VGet(center.x - offsetAxis.x * halfWidth, center.y - offsetAxis.y * halfWidth, center.z - offsetAxis.z * halfWidth);
            left.dif = color;
            left.spc = GetColorU8(0, 0, 0, 0);
            left.norm = VGet(0.0f, 1.0f, 0.0f);
            left.u = u;
            left.v = 0.0f;

            VERTEX3D right{};
            right.pos = VGet(center.x + offsetAxis.x * halfWidth, center.y + offsetAxis.y * halfWidth, center.z + offsetAxis.z * halfWidth);
            right.dif = color;
            right.spc = GetColorU8(0, 0, 0, 0);
            right.norm = VGet(0.0f, 1.0f, 0.0f);
            right.u = u;
            right.v = 1.0f;

            outVtx.push_back(left);
            outVtx.push_back(right);
        }

        // 隣接するサンプル点同士を四角形(三角形2枚)でつなぐ
        const size_t sampleCountInChunk = endIndex - beginIndex + 1;
        for (size_t i = 0; i + 1 < sampleCountInChunk; ++i) {
            const WORD base = static_cast<WORD>(i * 2);
            outIdx.push_back(base + 0);
            outIdx.push_back(base + 1);
            outIdx.push_back(base + 2);
            outIdx.push_back(base + 1);
            outIdx.push_back(base + 3);
            outIdx.push_back(base + 2);
        }
    }

    // ------------------------------------------------------------
    // UVスクロールぶんだけ時間を進める(航路と同じROUTE_RIBBON_SCROLL_SPEEDを流用)。
    // ------------------------------------------------------------
    void gmMapBoundaryVisualizer::update(float deltaTime)
    {
        if (ROUTE_RIBBON_SCROLL_SPEED == 0.0f) {
            return;
        }

        scrollTime_ += deltaTime;
        const float deltaU = ROUTE_RIBBON_SCROLL_SPEED * deltaTime;

        for (auto& chunk : chunks_) {
            for (auto& v : chunk.horizontalVtx) { v.u += deltaU; }
            for (auto& v : chunk.verticalVtx) { v.u += deltaU; }
        }
    }

    // ------------------------------------------------------------
    // 視界内(RENDER_DISTANCE_SQ以内)のチャンクだけ、2枚のリボンを描画する。
    // ------------------------------------------------------------
    void gmMapBoundaryVisualizer::render(const Shared<dxe::Camera>& camera)
    {
        if (!texture_) {
            return;
        }

        const tnl::Vector3 camPos = camera->getPosition();

        gm::ApplyCamera3D(camera);

        SetUseLighting(FALSE);
        SetWriteZBuffer3D(FALSE);                   // 半透明合成のため、Zバッファへの書き込みはしない
        SetUseZBuffer3D(TRUE);                      // 奥行きの前後関係は考慮する(他オブジェクトに隠れてよい)
        SetUseBackCulling(FALSE);                   // 十字クロス・低い視点からも見えるよう、両面描画する
        SetDrawBlendMode(DX_BLENDMODE_ADD, 255);    // 発光表現のため加算合成

        const int textureHandle = texture_->getDxLibGraphHandle();

        for (const auto& chunk : chunks_) {
            const float dx = chunk.cullCenter.x - camPos.x;
            const float dz = chunk.cullCenter.z - camPos.z;
            if ((dx * dx + dz * dz) > RENDER_DISTANCE_SQ) {
                continue;
            }

            if (!chunk.horizontalVtx.empty()) {
                DrawPolygonIndexed3D(chunk.horizontalVtx.data(), static_cast<int>(chunk.horizontalVtx.size()),
                    chunk.horizontalIdx.data(), static_cast<int>(chunk.horizontalIdx.size() / 3), textureHandle, TRUE);
            }
            if (!chunk.verticalVtx.empty()) {
                DrawPolygonIndexed3D(chunk.verticalVtx.data(), static_cast<int>(chunk.verticalVtx.size()),
                    chunk.verticalIdx.data(), static_cast<int>(chunk.verticalIdx.size() / 3), textureHandle, TRUE);
            }
        }

        SetUseLighting(TRUE);
        SetWriteZBuffer3D(TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        SetUseZBuffer3D(TRUE);
        SetUseBackCulling(TRUE);
    }
}
