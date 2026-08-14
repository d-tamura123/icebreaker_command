// gmFontCache.cpp
#include "gmFontCache.h"
#include <DxLib.h>

namespace gm {

    std::map<gmFontCache::Key, Shared<dxe::FontTextResouce>> gmFontCache::resourceCache_;
    std::map<std::string, bool> gmFontCache::registeredFontFiles_;

    Shared<dxe::FontTextResouce> gmFontCache::GetOrCreate(
        int32_t fontSize,
        const std::string& fontName,
        const std::string& filePath)
    {
        const Key key{ fontSize, fontName };

        auto it = resourceCache_.find(key);
        if (it != resourceCache_.end()) {
            return it->second; // 既存のリソースをそのまま共有して返す(再生成しない)
        }

        // フォントファイルの登録は、ファイル単位で初回の1回だけ行う
        // (同じファイルから複数サイズのリソースを作る場合、2回目以降はスキップされる)
        if (!registeredFontFiles_[filePath]) {
            tnl::AddFont(filePath);
            registeredFontFiles_[filePath] = true;
        }

        Shared<dxe::FontTextResouce> resource = dxe::FontTextResouce::Create(
            fontSize, fontName, DX_FONTTYPE_ANTIALIASING_EDGE_4X4, -1, 1);

        resourceCache_[key] = resource;
        return resource;
    }

}
