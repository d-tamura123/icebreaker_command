#include "gmIsland.h"
#include "../gmGameConfig.h"

namespace gm {

    void gmIsland::render(const Shared<dxe::Camera>& camera)
    {
        // カメラ位置
        tnl::Vector3 camPos = camera->getPosition();

        // 島の中心位置（gmMeshBase の position_）
        tnl::Vector3 islandPos = this->position_;

        // 距離
        tnl::Vector3 diff = islandPos - camPos;
        
        // 同じベクトル同士の内積を求めることで、長さの2乗を取得
        float distSq = tnl::Vector3::Dot(diff, diff);
        
        // 島の中心から最も遠い角までの概算半径を考慮する
        // (width と depth から簡易的な判定用半径の2乗を計算)
        float islandRadiusSq = (baseWidth_ * baseWidth_ + baseDepth_ * baseDepth_) * 0.25f;
        
        // 描画限界距離に、島のサイズ分の猶予を持たせる
        // (A < B - C  等価公式： 距離 > 限界値 + 半径)
        float maxCheckDist = RENDER_DISTANCE + std::sqrt(islandRadiusSq);

        // 描画距離外なら描画しない
        if (distSq > (maxCheckDist * maxCheckDist)) {
            return;
        }

        // さらにカメラの裏側にある島を除外
        // カメラ前方方向との角度判定
        // camera->forward() が指す方向と diff の角度のコサインを計算する
        tnl::Vector3 camForward = camera->forward();
        float forwardLenSq = tnl::Vector3::Dot(camForward, camForward);
        if (forwardLenSq > 0.0f && distSq > 0.0f) {
            float diffLen = std::sqrt(distSq);
            float forwardLen = std::sqrt(forwardLenSq);
            float cosAngle = tnl::Vector3::Dot(diff, camForward) / (diffLen * forwardLen);
            // cosAngle が負（おおよそ < -0.2）ならカメラの後方寄り -> スキップ
            // しきい値は必要に応じて調整（-0.2 はやや寛容な判定）
            if (cosAngle < -0.2f) {
                return;
            }
        }

        // ★基底クラスの描画を呼ぶ（回転・スケール・テクスチャ設定など）
        gmMeshBase::render(camera);
    }
}
