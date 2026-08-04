// gmFlameThrowerAttack.h
#pragma once
#include "gmObjectBase.h"
#include "../effect/gmSpriteAnimInstance.h"
#include <vector>
#include <memory>

namespace gm {

    class gmSpriteAnimRegistry;

    // ------------------------------------------------------------
    // 火炎放射攻撃: 船の片側面(発射地点は発動時に固定)から、
    // クリック方向を中心に扇状(fanAngleDeg)へ一定時間(duration)
    // 燃え続ける近接攻撃。gmProjectileBaseの「発射→放物線→着弾」という
    // 概念を一切使わないため、gmObjectBaseを直接継承する
    // (gmProjectileBase.hのコメントで触れられていた想定攻撃そのもの)。
    //
    // 見た目は、扇の中の1本1本の「ライン」ごとに、tktk_Fire_10のビルボード1枚を
    // 起点から先端まで、細長く伸ばして(setForwardLength)配置することで表現する。
    // 当たり判定は、扇形コライダーが存在しないため、ライン1本につき
    // カプセルコライダー1本を対応させて近似する(見た目の構成とほぼ1対1)。
    //
    // 発動時の狙い(発射位置・中心角度)はコンストラクタで一度だけ確定し、
    // 以後は追従しない(発動中にマウスを動かしても扇は回らない)。
    // ------------------------------------------------------------
    class gmFlameThrowerAttack : public gmObjectBase {
    public:
        // arg1... 識別ID
        // arg2... 発射位置(ワールド座標。船の側面ハードポイントの想定位置を渡す)
        // arg3... 扇の中心方向(正規化済み、XZ平面、Y成分は無視される)
        // arg4... スプライトアニメーションのメタデータ/テクスチャを引くためのレジストリ
        // arg5... 扇の開き角(度)
        // arg6... 扇の中に配置するライン(=カプセル1本+ビルボード1枚)の本数
        // arg7... 射程(ワールド単位)
        // arg8... 発動してから自動的に終了するまでの時間(秒)
        gmFlameThrowerAttack(
            const std::string& id,
            const tnl::Vector3& originPos,
            const tnl::Vector3& centerDir,
            const std::shared_ptr<gmSpriteAnimRegistry>& spriteRegistry,
            float fanAngleDeg = DEFAULT_FAN_ANGLE_DEG,
            int lineCount = DEFAULT_LINE_COUNT,
            float range = DEFAULT_RANGE,
            float duration = DEFAULT_DURATION
        );

        void update(float deltaTime) override;
        void render(const Shared<dxe::Camera>& camera) override;

        void onCollisionEnter(gmObjectBase* other) override;

        // 発動時間が尽きる前に強制的に終了させたい場合に呼ぶ
        // (アウトロ再生後、全ラインの再生が終わり次第kill()される)
        void requestStop();

    private:
        std::vector<gmSpriteAnimInstance> lineVisuals_; // ライン1本につき1枚(細長い板ポリ)

        float elapsedTime_   = 0.0f;                // 発動からの経過時間(秒)
        float duration_      = DEFAULT_DURATION;
        bool  stopRequested_ = false;               // 全ラインへrequestStop()を送信済みかどうか

        // ---- 調整用パラメータ ----
        // TODO:
        // 将来の武器強化要素(射程/開き角/本数の強化等)で、
        // コンストラクタ引数から上書きする想定。
        static constexpr float DEFAULT_FAN_ANGLE_DEG = 45.0f;   // 扇の開き角(度)
        static constexpr int   DEFAULT_LINE_COUNT    = 5;       // 扇の中のライン本数(奇数だと中心ラインがクリック方向とちょうど重なる)
        static constexpr float DEFAULT_RANGE         = 160.0f;  // 射程(world単位)
        static constexpr float DEFAULT_DURATION      = 1.2f;    // 発動から自動終了までの秒数

        static constexpr float LINE_VISUAL_WIDTH    = 45.0f;   // ラインの太さ(見た目、world単位)
        static constexpr float LINE_COLLIDER_RADIUS = 24.0f;   // 当たり判定カプセルの半径(=ラインの太さ)

        // tktk_Fire_10は「玉(頭)が育っていく」アニメのため、コマによって玉側(頭側)の
        // 余白(透明部分)の割合が大きく異なる(実測: 1コマ目で約77%、育ちきったコマで約4%)。
        // ループ再生に使うコマ範囲によらず、ある程度余白を引いておかないと
        // 「炎が起点(船)まで届いて見えない」問題が起きるため、暫定値としてここで調整する。
        // NOTE: 根本的には、CSV側でループに使うコマ範囲を「玉が育ちきった後」に絞るのが本筋。
        //       それと合わせて、この値も実機で見ながら微調整してほしい。
        static constexpr float VISUAL_HEAD_UV_INSET = 0.32f;   // 頭側(起点側)のUVを削る割合
        static constexpr float VISUAL_TAIL_UV_INSET = 0.00f;   // 尾側(先端側)のUVを削る割合

        static constexpr const char* VFX_CLIP_NAME = "tktk_Fire_10"; // 見た目に使うクリップ名
        // NOTE:
        // 持続ループ再生させるには、tktk_sprite_metadata.csv側でこのクリップに
        // loopStart/loopEndを設定しておく必要がある(未設定の場合、gmSpriteAnimInstance側の
        // 仕様により「イントロ分(frameCount)を1回再生したら自動終了」という
        // ワンショット相当の挙動にフォールバックする。見た目のループが未対応でも
        // ゲームは壊れないが、durationいっぱいまで燃え続けない可能性がある)。

        // tktk_Fire_10は「頭(玉)が左・尾が右」の横向き素材。
        // 火炎放射では、玉(頭)を火元側(起点/船側)、尾を放射先(外側)に
        // 見立てたいため、gmProjectileの弾のような「頭が進行方向(外側)を
        // 向く」向きとは逆にする(=true)。
        static constexpr bool REVERSE_TAIL_DIRECTION = true;

        // カプセルコライダーの向き(localRotation)を「ローカルY軸→ラインの方向」で
        // 組み立てているが、エンジン側の回転の向き(ハンドネス)によっては
        // 実機で狙った向きと逆になる可能性があるため、その場合はtrueにして
        // 回転方向を反転する(gmKyleColliderGizmoでの目視確認を想定)
        static constexpr bool CAPSULE_AXIS_FLIP = false;
    };
}
