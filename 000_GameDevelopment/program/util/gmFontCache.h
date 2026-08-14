// gmFontCache.h
// dxe::FontTextResouce(フォントリソース)を一元管理し、同じ(フォント名, サイズ)の組み合わせに
// ついては使い回すためのキャッシュ。
//
// このクラスの GetOrCreate() を経由して取得することで、同じ(フォント名, サイズ)の要求には
// 常に同じ Shared<dxe::FontTextResouce> を返す(参照カウントが増えるだけで再生成はしない)。
// フォントファイルの登録(tnl::AddFont())も、ファイル単位で初回の1回だけ行う。
#pragma once
#include <dxe.h>
#include <string>
#include <map>

namespace gm {

    class gmFontCache {
    public:
        // arg1... フォントサイズ
        // arg2... DxLibへ登録する際のフォント名(ResourceConstantHedder.hのFONT_NAME_*を渡す)
        // arg3... フォントファイルパス(ResourceConstantHedder.hのFILE_PATH_*を渡す。ファイル単位で初回のみ登録される)
        static Shared<dxe::FontTextResouce> GetOrCreate(
            int32_t fontSize,
            const std::string& fontName,
            const std::string& filePath);

    private:
        struct Key {
            int32_t fontSize;
            std::string fontName;
            bool operator<(const Key& other) const {
                if (fontSize != other.fontSize) return fontSize < other.fontSize;
                return fontName < other.fontName;
            }
        };

        static std::map<Key, Shared<dxe::FontTextResouce>> resourceCache_;
        static std::map<std::string, bool> registeredFontFiles_; // tnl::AddFont()済みかどうか(ファイルパス単位)
    };

}
