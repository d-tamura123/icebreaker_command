// gmSpriteAnimRegistry.h
#pragma once
#include <dxe.h>
#include <string>
#include <unordered_map>
#include <memory>
#include "gmSpriteAnimClip.h"

namespace gm {

    // ------------------------------------------------------------
    // スプライトアニメーションのメタデータ(CSV)とテクスチャ(PNG)の読み込み。
    //
    // ・メタデータは起動時に loadFromCSV() で1回だけ読み込み、以降はメモリ上の辞書を引くだけ。
    // ・テクスチャは実際に要求されたタイミングで初めて読み込み、以降はキャッシュを返す
    //   (234種類全部を最初から読み込むと無駄が多いため)。
    // ------------------------------------------------------------
    class gmSpriteAnimRegistry {
    public:
        // 起動時に1回だけ呼ぶ。CSVを読み込みメタデータをキャッシュする。
        // ret.... 読み込みに成功したらtrue
        bool loadFromCSV(const std::string& csvPath);

        // 名前からメタデータを引く(見つからなければnullptr)
        const gmSpriteAnimClip* getClip(const std::string& name) const;

        // 名前からテクスチャを引く。初回だけ実ファイルを読み込み、以降はキャッシュを返す。
        // arg2... 画像が置かれているディレクトリ(末尾に'/'を含むこと。gmGameConfig::VFX_EFFECT_GRAPHICS_DIR等)
        Shared<dxe::Texture> getTexture(const std::string& name, const std::string& imageDir);

    private:
        std::unordered_map<std::string, gmSpriteAnimClip>       clips_;         // クリップ名 → メタデータ
        std::unordered_map<std::string, Shared<dxe::Texture>>   textureCache_;  // クリップ名 → 読み込み済みテクスチャ
    };
}
