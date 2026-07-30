#include "gmMeshBoundsUtil.h"
#include <DxLib.h>
#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>
#include <cfloat>

namespace gm {

    bool ComputeMeshBounds(int mvHandle, tnl::Vector3& outCenter, tnl::Vector3& outSize)
    {
        const int meshNum = MV1GetMeshNum(mvHandle);
        if (meshNum <= 0) {
            return false;
        }

        tnl::Vector3 minPos(FLT_MAX, FLT_MAX, FLT_MAX);
        tnl::Vector3 maxPos(-FLT_MAX, -FLT_MAX, -FLT_MAX);

        // 全サブメッシュ分のMin/Maxを合算する
        for (int i = 0; i < meshNum; ++i) {
            DxLib::VECTOR mn = MV1GetMeshMinPosition(mvHandle, i);
            DxLib::VECTOR mx = MV1GetMeshMaxPosition(mvHandle, i);

            minPos.x = std::min(minPos.x, mn.x);
            minPos.y = std::min(minPos.y, mn.y);
            minPos.z = std::min(minPos.z, mn.z);

            maxPos.x = std::max(maxPos.x, mx.x);
            maxPos.y = std::max(maxPos.y, mx.y);
            maxPos.z = std::max(maxPos.z, mx.z);
        }

        outCenter = (minPos + maxPos) * 0.5f;
        outSize = maxPos - minPos;
        return true;
    }

}
