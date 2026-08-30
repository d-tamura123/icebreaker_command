// gmSoundManager.cpp
#include "gmSoundManager.h"
#include "../gmGameConfig.h"
#include <DxLib.h>
#undef min              // std::max, std::minのマクロ競合解消
#undef max
#include <algorithm>

namespace gm {

    void gmSoundManager::finalize()
    {
        for (auto& [_, handle] : seHandles_) {
            DeleteSoundMem(handle);
        }
        for (auto& [_, handle] : bgmHandles_) {
            DeleteSoundMem(handle);
        }

        seHandles_.clear();
        bgmHandles_.clear();
        activeSeEndTimes_.clear();
    }

    void gmSoundManager::loadSe(const std::string& name, const std::string& filepath)
    {
        if (seHandles_.find(name) != seHandles_.end()) return; // 既に読み込み済み(使い回し)

        const int handle = LoadSoundMem(filepath.c_str());
        if (handle != -1) {
            seHandles_[name] = handle;
        }
    }

    void gmSoundManager::loadBgm(const std::string& name, const std::string& filepath)
    {
        if (bgmHandles_.find(name) != bgmHandles_.end()) return; // 既に読み込み済み(使い回し)

        const int handle = LoadSoundMem(filepath.c_str());
        if (handle != -1) {
            bgmHandles_[name] = handle;
        }
    }

    void gmSoundManager::setListenerTransform(const tnl::Vector3& pos, const tnl::Vector3& rightAxis)
    {
        listenerPos_ = pos;
        listenerRightAxis_ = rightAxis;
    }

    // ------------------------------------------------------------
    // 距離からパン値を計算する(-SOUND_PAN_MAX_VALUE〜+SOUND_PAN_MAX_VALUE)。
    //
    // リスナーの右方向ベクトルへ、音源方向(正規化済み)を内積で投影した値(-1〜+1。
    // 真横なら±1に近く、真正面・真後ろなら0に近い)に、距離由来の強さの倍率を掛ける。
    // SOUND_PAN_DEADZONE_DIST未満は常に0(中央)にし、近距離での耳障りな左右の揺れを防ぐ。
    // ------------------------------------------------------------
    int gmSoundManager::calcPan(const tnl::Vector3& worldPos) const
    {
        tnl::Vector3 toSource = worldPos - listenerPos_;
        toSource.y = 0.0f; // 水平面(XZ)のみで判定する(高さは無視)

        const float dist = toSource.length();
        if (dist < SOUND_PAN_DEADZONE_DIST || dist < 1e-5f) {
            return 0;
        }

        const tnl::Vector3 dir = toSource * (1.0f / dist);

        tnl::Vector3 rightAxis = listenerRightAxis_;
        rightAxis.y = 0.0f;
        const float rightLen = rightAxis.length();
        if (rightLen < 1e-5f) {
            return 0; // リスナーの右方向が求まらない(異常値)場合は中央扱い
        }
        rightAxis = rightAxis * (1.0f / rightLen);

        const float lateral = tnl::Vector3::Dot(dir, rightAxis); // -1(左)〜+1(右)

        const float distanceScale = std::clamp(
            (dist - SOUND_PAN_DEADZONE_DIST) / (SOUND_PAN_MAX_DIST - SOUND_PAN_DEADZONE_DIST),
            0.0f, 1.0f);

        return static_cast<int>(lateral * distanceScale * SOUND_PAN_MAX_VALUE);
    }

    // ------------------------------------------------------------
    // 距離から音量を計算する(SOUND_VOLUME_MIN〜SOUND_VOLUME_MAX)。
    // ------------------------------------------------------------
    int gmSoundManager::calcVolumeByDistance(const tnl::Vector3& worldPos) const
    {
        const float dist = (worldPos - listenerPos_).length();
        const float t = std::clamp(dist / SOUND_VOLUME_MAX_DIST, 0.0f, 1.0f);
        return SOUND_VOLUME_MAX - static_cast<int>((SOUND_VOLUME_MAX - SOUND_VOLUME_MIN) * t);
    }

    // ------------------------------------------------------------
    // 同時発音数の制限。SE名ごとに「終了予定時刻(elapsedTime_基準)」のリストを持ち、
    // 期限切れを間引いた上で、残数がSOUND_MAX_CONCURRENT_SAME_SE未満なら再生を許可する。
    //
    // Note: DxLibは同一ハンドルへの複数回のPlaySoundMem()呼び出しでも重複再生できる仕様
    // (CheckSoundMem()は「そのハンドルが1つでも再生中か」の真偽値のみを返し、重複再生数までは
    // 分からない)ため、実際に鳴らした回数を自前でカウントする方式にしている。
    // ------------------------------------------------------------
    bool gmSoundManager::tryReserveConcurrentSlot(const std::string& name, int handle)
    {
        auto& endTimes = activeSeEndTimes_[name];

        endTimes.erase(
            std::remove_if(endTimes.begin(), endTimes.end(),
                [this](float endTime) { return endTime <= elapsedTime_; }),
            endTimes.end());

        if (static_cast<int>(endTimes.size()) >= SOUND_MAX_CONCURRENT_SAME_SE) {
            return false;
        }

        const int totalTimeMs = GetSoundTotalTime(handle);
        const float durationSec = (totalTimeMs > 0) ? (totalTimeMs / 1000.0f) : 1.0f; // 取得失敗時は1秒を仮定
        endTimes.push_back(elapsedTime_ + durationSec);

        return true;
    }

    void gmSoundManager::playUiSe(const std::string& name)
    {
        auto it = seHandles_.find(name);
        if (it == seHandles_.end()) return;

        if (!tryReserveConcurrentSlot(name, it->second)) return;

        ChangeVolumeSoundMem(static_cast<int>(SOUND_VOLUME_MAX * masterSeVolumeScale_), it->second);
        ChangePanSoundMem(0, it->second); // UI音はパン・距離減衰の対象外。常に中央
        PlaySoundMem(it->second, DX_PLAYTYPE_BACK);
    }

    void gmSoundManager::playPositionalSe(const std::string& name, const tnl::Vector3& worldPos)
    {
        auto it = seHandles_.find(name);
        if (it == seHandles_.end()) return;

        if (!tryReserveConcurrentSlot(name, it->second)) return;

        const int pan = calcPan(worldPos);
        const int volume = static_cast<int>(calcVolumeByDistance(worldPos) * masterSeVolumeScale_);

        ChangeVolumeSoundMem(volume, it->second);
        ChangePanSoundMem(pan, it->second);
        PlaySoundMem(it->second, DX_PLAYTYPE_BACK);
    }

    void gmSoundManager::playBgm(const std::string& name, bool loop)
    {
        auto it = bgmHandles_.find(name);
        if (it == bgmHandles_.end()) return;

        currentBgmHandle_ = it->second;
        bgmVolume_ = SOUND_VOLUME_MAX;
        ChangeVolumeSoundMem(static_cast<int>(bgmVolume_ * masterBgmVolumeScale_), currentBgmHandle_);
        PlaySoundMem(currentBgmHandle_, loop ? DX_PLAYTYPE_LOOP : DX_PLAYTYPE_BACK);
    }

    void gmSoundManager::stopBgm()
    {
        if (currentBgmHandle_ != -1) {
            StopSoundMem(currentBgmHandle_);
            currentBgmHandle_ = -1;
        }
    }

    void gmSoundManager::fadeOutBgm(float duration)
    {
        if (currentBgmHandle_ == -1) return;
        isFadingOut_ = true;
        fadeDuration_ = duration;
        fadeTimer_ = 0.0f;
    }

    void gmSoundManager::fadeInBgm(const std::string& name, float duration, bool loop)
    {
        auto it = bgmHandles_.find(name);
        if (it == bgmHandles_.end()) return;

        targetBgmHandle_ = it->second;
        isFadingIn_ = true;
        fadeDuration_ = duration;
        fadeTimer_ = 0.0f;
        bgmVolume_ = 0;
        ChangeVolumeSoundMem(0, targetBgmHandle_);
        PlaySoundMem(targetBgmHandle_, loop ? DX_PLAYTYPE_LOOP : DX_PLAYTYPE_BACK);
    }

    void gmSoundManager::update(float deltaTime)
    {
        elapsedTime_ += deltaTime;

        // --- BGMフェードアウト処理 ---
        if (isFadingOut_ && currentBgmHandle_ != -1) {
            fadeTimer_ += deltaTime;

            float t = fadeTimer_ / fadeDuration_;
            t = std::clamp(t, 0.0f, 1.0f);

            bgmVolume_ = static_cast<int>(SOUND_VOLUME_MAX * (1.0f - t));
            ChangeVolumeSoundMem(static_cast<int>(bgmVolume_ * masterBgmVolumeScale_), currentBgmHandle_);

            if (t >= 1.0f) {
                stopBgm();
                isFadingOut_ = false;
            }
        }

        // --- BGMフェードイン処理 ---
        if (isFadingIn_ && targetBgmHandle_ != -1) {
            fadeTimer_ += deltaTime;

            float t = fadeTimer_ / fadeDuration_;
            t = std::clamp(t, 0.0f, 1.0f);

            bgmVolume_ = static_cast<int>(SOUND_VOLUME_MAX * t);
            ChangeVolumeSoundMem(static_cast<int>(bgmVolume_ * masterBgmVolumeScale_), targetBgmHandle_);

            if (t >= 1.0f) {
                currentBgmHandle_ = targetBgmHandle_;
                targetBgmHandle_ = -1;
                isFadingIn_ = false;
            }
        }
    }
}
