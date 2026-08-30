// gmSoundManager.h
#pragma once
#include <dxe.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace gm {

    // ------------------------------------------------------------
    // サウンドマネージャ。
    // シーンスコープ(シーン開始時にロード、シーン終了時に破棄)で使う想定。
    //
    // 3D空間の位置表現は以下の簡易方式でおこなう:
    //   ・左右: パン(DxLibのChangePanSoundMem)
    //   ・距離: 音量減衰のみ
    // かつ、いずれも「発火した瞬間に1回だけ計算し、以後は再生中でも更新しない」仕様
    // (再生中もリアルタイムに追従させると、カメラを振った時に耳障りになりうるため。
    //  詳細な閾値・パン最大値等はgmGameConfig.hのSOUND_*参照)。
    // ------------------------------------------------------------
    class gmSoundManager {
    public:
        void finalize(); // 保持している全ハンドルを解放する

        // ---- リソース読み込み ----
        // 同じnameで既に読み込み済みの場合は何もしない(重複ロード防止・使い回し)。
        void loadSe(const std::string& name, const std::string& filepath);
        void loadBgm(const std::string& name, const std::string& filepath);

        // ---- リスナー(=カメラ)の状態。毎フレーム更新する ----
        // arg1... リスナー座標(距離減衰の基準)
        // arg2... リスナーの右方向ベクトル(パンの基準。dxe::Camera::right()をそのまま渡す想定)
        void setListenerTransform(const tnl::Vector3& pos, const tnl::Vector3& rightAxis);

        // ---- 再生 ----
        // UI操作音(クリック音等)。パン・距離減衰の対象外、常に中央・フルボリュームで鳴らす。
        void playUiSe(const std::string& name);

        // 3D空間に位置を持つ単発SE。発火した瞬間のパン・音量を1回だけ計算して鳴らす。
        void playPositionalSe(const std::string& name, const tnl::Vector3& worldPos);

        void playBgm(const std::string& name, bool loop);
        void stopBgm();
        void fadeOutBgm(float duration);
        void fadeInBgm(const std::string& name, float duration, bool loop);

        // ---- マスターボリューム ----
        // 0.0〜1.0の係数。設定画面は無いが、将来足す際にここへ繋げられるよう先に用意しておく。
        void setMasterSeVolumeScale(float scale) { masterSeVolumeScale_ = scale; }
        void setMasterBgmVolumeScale(float scale) { masterBgmVolumeScale_ = scale; }

        // BGMフェード・同時発音数トラッキングの経過時間更新。毎フレーム呼ぶこと。
        void update(float deltaTime);

    private:
        // 距離からパン値(-SOUND_PAN_MAX_VALUE〜+SOUND_PAN_MAX_VALUE)を計算する。
        // arg1... 音源のワールド座標
        int calcPan(const tnl::Vector3& worldPos) const;

        // 距離から音量(SOUND_VOLUME_MIN〜SOUND_VOLUME_MAX)を計算する。
        int calcVolumeByDistance(const tnl::Vector3& worldPos) const;

        // 同時発音数の制限。SOUND_MAX_CONCURRENT_SAME_SE未満なら再生してtrueを返し、
        // 内部でその再生ぶんの終了予定時刻を記録する。上限に達していればfalseを返す(再生しない)。
        bool tryReserveConcurrentSlot(const std::string& name, int handle);

        std::unordered_map<std::string, int> seHandles_;
        std::unordered_map<std::string, int> bgmHandles_;

        tnl::Vector3 listenerPos_{ 0.0f, 0.0f, 0.0f };
        tnl::Vector3 listenerRightAxis_{ 1.0f, 0.0f, 0.0f };

        float masterSeVolumeScale_ = 1.0f;
        float masterBgmVolumeScale_ = 1.0f;

        // 同時発音数トラッキング用。SE名ごとに「終了予定時刻(elapsedTime_基準)」のリストを持つ。
        std::unordered_map<std::string, std::vector<float>> activeSeEndTimes_;
        float elapsedTime_ = 0.0f;

        // ---- BGMフェード ----
        int currentBgmHandle_ = -1;
        int targetBgmHandle_ = -1;
        bool isFadingOut_ = false;
        bool isFadingIn_ = false;
        float fadeTimer_ = 0.0f;
        float fadeDuration_ = 0.0f;
        int bgmVolume_ = 255; // マスター係数を掛ける前の基準値(0〜255)
    };
}
