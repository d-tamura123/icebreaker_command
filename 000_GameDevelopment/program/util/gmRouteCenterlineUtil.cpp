// gmRouteCenterlineUtil.cpp
#include "gmRouteCenterlineUtil.h"
#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>
#include <cmath>

namespace gm {

    namespace {
        // 2点間の距離を、遠心的パラメータ化用の指数(0.5)で累乗して返す。
        // 距離0(区間の外側で前後の点を自分自身で代用する場合に発生しうる)による
        // ゼロ除算を避けるため、極小値でクランプしてから累乗する。
        float SafeDistancePow(const tnl::Vector2f& a, const tnl::Vector2f& b)
        {
            float distance = (b - a).length();
            if (distance < 0.0001f) {
                distance = 0.0001f;
            }
            return std::sqrt(distance);
        }

        // 3次ベジェ(頂点p1→p2、制御点c1,c2)をtで評価する
        tnl::Vector2f EvalCubicBezier(const tnl::Vector2f& p1, const tnl::Vector2f& c1,
            const tnl::Vector2f& c2, const tnl::Vector2f& p2, float t)
        {
            const float u = 1.0f - t;
            return p1 * (u * u * u)
                + c1 * (3.0f * u * u * t)
                + c2 * (3.0f * u * t * t)
                + p2 * (t * t * t);
        }
    }

    std::vector<tnl::Vector2f> SampleRouteCenterline(
        const std::vector<tnl::Vector2f>& waypointsWorld,
        float sampleStep)
    {
        std::vector<tnl::Vector2f> result;
        if (waypointsWorld.empty()) {
            return result;
        }

        result.push_back(waypointsWorld.front());

        const size_t n = waypointsWorld.size();
        for (size_t seg = 0; seg + 1 < n; ++seg) {
            const tnl::Vector2f& p1 = waypointsWorld[seg];
            const tnl::Vector2f& p2 = waypointsWorld[seg + 1];
            const tnl::Vector2f& p0 = (seg >= 1) ? waypointsWorld[seg - 1] : p1;        // 区間の外側は自分自身で代用
            const tnl::Vector2f& p3 = (seg + 2 < n) ? waypointsWorld[seg + 2] : p2;     // 同上

            const float t0 = 0.0f;
            const float t1 = t0 + SafeDistancePow(p0, p1);
            const float t2 = t1 + SafeDistancePow(p1, p2);
            const float t3 = t2 + SafeDistancePow(p2, p3);

            // P1・P2それぞれの接線ベクトル(Barry-Goldman式)
            const tnl::Vector2f m1 = (p1 - p0) * ((t2 - t1) / (t1 - t0))
                - (p2 - p0) * ((t2 - t1) / (t2 - t0))
                + (p2 - p1);
            const tnl::Vector2f m2 = (p2 - p1)
                - (p3 - p1) * ((t2 - t1) / (t3 - t1))
                + (p3 - p2) * ((t2 - t1) / (t3 - t2));

            const tnl::Vector2f c1 = p1 + m1 / 3.0f;
            const tnl::Vector2f c2 = p2 - m2 / 3.0f;

            // 区間の近似長(制御多角形の長さ)からサンプル数を決める
            const float approxLength = (c1 - p1).length() + (c2 - c1).length() + (p2 - c2).length();
            const int sampleCount = std::max(1, static_cast<int>(std::round(approxLength / sampleStep)));

            for (int s = 1; s <= sampleCount; ++s) {
                const float t = static_cast<float>(s) / static_cast<float>(sampleCount);
                result.push_back(EvalCubicBezier(p1, c1, c2, p2, t));
            }
        }

        return result;
    }

}
