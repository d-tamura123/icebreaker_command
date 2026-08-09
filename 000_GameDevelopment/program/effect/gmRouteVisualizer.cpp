// gmRouteVisualizer.cpp
#include "gmRouteVisualizer.h"
#include "../map/gmMapManager.h"
#include "../gmGameConfig.h"
#include "../util/gmRenderUtil.h"
#include "../util/gmRouteCenterlineUtil.h"
#include <DxLib.h>
#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>
#include <cmath>

namespace gm {

    gmRouteVisualizer::gmRouteVisualizer(const std::shared_ptr<gmMapManager>& map)
        : map_(map)
    {
        loadResources();

        if (auto m = map_.lock()) {
            buildAllRoutes(*m);
        }
    }

    gmRouteVisualizer::~gmRouteVisualizer() = default;

    void gmRouteVisualizer::loadResources()
    {
        texture_ = dxe::Texture::CreateFromFile(GRAPHICS_FILE_PATH__ROUTE_RIBBON);
    }

    // ------------------------------------------------------------
    // 読み込めている航路(GetRouteCount()本)ぶん、順にリボンを構築する。
    // ------------------------------------------------------------
    void gmRouteVisualizer::buildAllRoutes(const gmMapManager& map)
    {
        chunks_.clear();

        const size_t routeCount = map.GetRouteCount();
        for (size_t i = 0; i < routeCount; ++i) {
            std::vector<tnl::Vector2f> waypointsWorld = map.GetRouteWorldPoints(i);
            buildRoute(waypointsWorld);
        }
    }

    void gmRouteVisualizer::buildRoute(const std::vector<tnl::Vector2f>& waypointsWorld)
    {
        if (waypointsWorld.size() < 2) {
            return; // S・Gの2点すら無い航路データは異常値として無視する
        }

        std::vector<tnl::Vector2f> centerline = sampleCenterline(waypointsWorld);
        appendChunks(centerline);
    }

    // ------------------------------------------------------------
    // 中心線は遠心的Catmull-Rom補間で求める
    // 
    // (詳細はSampleRouteCenterline()参照)。
    // ウェイポイント列を密なポリラインに変換したものを扱う。
    // ------------------------------------------------------------
    std::vector<tnl::Vector2f> gmRouteVisualizer::sampleCenterline(const std::vector<tnl::Vector2f>& waypointsWorld) const
    {
        return SampleRouteCenterline(waypointsWorld, ROUTE_RIBBON_SAMPLE_STEP);
    }

    // ------------------------------------------------------------
    // 中心線1本ぶんを、ROUTE_RIBBON_CHUNK_LENGTHごとにチャンク分割し、
    // 各チャンクに水平/垂直/影の3枚のリボンを構築してchunks_へ積む。
    // 隣接チャンクは境界の点を共有するため、チャンクの継ぎ目に隙間はできない。
    // ------------------------------------------------------------
    void gmRouteVisualizer::appendChunks(const std::vector<tnl::Vector2f>& centerline)
    {
        const size_t pointCount = centerline.size();
        if (pointCount < 2) {
            return;
        }

        // 累積距離(フェード・UV計算に使う)
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

            // 空中・水平リボン(接線から求めた水平方向を基準オフセット軸にする)
            buildRibbonSegment(centerline, distances, chunkBegin, chunkEnd,
                ROUTE_RIBBON_ALTITUDE, /*useHorizontalOffsetAxis=*/true,
                ROUTE_RIBBON_WIDTH, 255, 255,
                chunk.horizontalVtx, chunk.horizontalIdx);

            // 空中・垂直リボン(ワールド上方向固定を基準オフセット軸にする。水平リボンと十字に交差)
            buildRibbonSegment(centerline, distances, chunkBegin, chunkEnd,
                ROUTE_RIBBON_ALTITUDE, /*useHorizontalOffsetAxis=*/false,
                ROUTE_RIBBON_WIDTH, 255, 255,
                chunk.verticalVtx, chunk.verticalIdx);

            // 海面・影リボン(水平のみ、控えめな太さ・明るさ)
            buildRibbonSegment(centerline, distances, chunkBegin, chunkEnd,
                ROUTE_SHADOW_RIBBON_HEIGHT_OFFSET, /*useHorizontalOffsetAxis=*/true,
                ROUTE_RIBBON_WIDTH * ROUTE_SHADOW_RIBBON_WIDTH_SCALE,
                ROUTE_SHADOW_RIBBON_COLOR_SCALE, ROUTE_SHADOW_RIBBON_ALPHA_SCALE,
                chunk.shadowVtx, chunk.shadowIdx);

            const tnl::Vector2f& midPoint = centerline[(chunkBegin + chunkEnd) / 2];
            chunk.cullCenter = tnl::Vector3(midPoint.x, ROUTE_RIBBON_ALTITUDE, midPoint.y);

            chunks_.push_back(std::move(chunk));

            chunkBegin = chunkEnd; // 次のチャンクは、この境界点を共有して始める(継ぎ目の隙間防止)
        }
    }

    // ------------------------------------------------------------
    // [beginIndex, endIndex]区間の中心線から、リボン(帯)の頂点/インデックスを構築する。
    // 水平/垂直/影の3種とも、このロジックを共有し、パラメータだけ変える。
    // ------------------------------------------------------------
    void gmRouteVisualizer::buildRibbonSegment(
        const std::vector<tnl::Vector2f>& centerline,
        const std::vector<float>& distances,
        size_t beginIndex, size_t endIndex,
        float heightOffset,
        bool useHorizontalOffsetAxis,
        float width,
        uint8_t colorScale,
        uint8_t alphaScale,
        std::vector<VERTEX3D>& outVtx,
        std::vector<WORD>& outIdx) const
    {
        outVtx.clear();
        outIdx.clear();

        const float halfWidth = width * 0.5f;
        const float totalLength = distances.back();
        const size_t pointCount = centerline.size();

        for (size_t i = beginIndex; i <= endIndex; ++i) {
            // 接線(前後の点から求める。区間の両端は片側だけで代用)
            tnl::Vector2f tangent2D;
            if (i == 0) {
                tangent2D = centerline[1] - centerline[0];
            }
            else if (i == pointCount - 1) {
                tangent2D = centerline[i] - centerline[i - 1];
            }
            else {
                tangent2D = centerline[i + 1] - centerline[i - 1];
            }
            tangent2D.normalize();

            const tnl::Vector3 center(centerline[i].x, heightOffset, centerline[i].y);

            // 基準オフセット軸:
            //   水平リボン: 接線を水平面内で90度回転させた「右方向」
            //   垂直リボン: ワールド上方向(0,1,0)固定(水平な接線とは常に直交するため、
            //              回転計算は不要でそのまま使える)
            tnl::Vector3 offsetAxis;
            if (useHorizontalOffsetAxis) {
                const tnl::Vector3 tangent3D(tangent2D.x, 0.0f, tangent2D.y);
                offsetAxis = tnl::Vector3::Normalize(tnl::Vector3::Cross(tnl::Vector3::up(), tangent3D));
            }
            else {
                offsetAxis = tnl::Vector3::up();
            }

            // 始点・終点付近のアルファフェード
            const float distFromStart = distances[i];
            const float distFromEnd = totalLength - distances[i];
            float fade = 1.0f;
            if (distFromStart < ROUTE_RIBBON_FADE_LENGTH) {
                fade = distFromStart / ROUTE_RIBBON_FADE_LENGTH;
            }
            if (distFromEnd < ROUTE_RIBBON_FADE_LENGTH) {
                fade = std::min(fade, distFromEnd / ROUTE_RIBBON_FADE_LENGTH);
            }
            fade = std::clamp(fade, 0.0f, 1.0f);

            const uint8_t finalAlpha = static_cast<uint8_t>(
                std::clamp(fade * static_cast<float>(alphaScale), 0.0f, 255.0f));
            COLOR_U8 color = GetColorU8(colorScale, colorScale, colorScale, finalAlpha);

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
    // UVスクロールぶんだけ時間を進める。空中の水平/垂直リボンだけスクロールさせ、
    // 海面の影リボンは位置確認用の静かな基準線として、あえて動かさない。
    // ------------------------------------------------------------
    void gmRouteVisualizer::update(float deltaTime)
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
    // 視界内(RENDER_DISTANCE_SQ以内)のチャンクだけ、3枚のリボンを描画する。
    // ------------------------------------------------------------
    void gmRouteVisualizer::render(const Shared<dxe::Camera>& camera)
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
            if (!chunk.shadowVtx.empty()) {
                DrawPolygonIndexed3D(chunk.shadowVtx.data(), static_cast<int>(chunk.shadowVtx.size()),
                    chunk.shadowIdx.data(), static_cast<int>(chunk.shadowIdx.size() / 3), textureHandle, TRUE);
            }
        }

        SetUseLighting(TRUE);
        SetWriteZBuffer3D(TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        SetUseZBuffer3D(TRUE);
        SetUseBackCulling(TRUE);
    }
}
