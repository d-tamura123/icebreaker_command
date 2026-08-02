// gmSpriteAnimRegistry.cpp
#include "gmSpriteAnimRegistry.h"
#include <fstream>
#include <sstream>
#include <vector>

namespace gm {

    namespace {
        // カンマ区切りの1行を分割する(このCSVはフィールド内にカンマを含まない前提の単純な実装)
        std::vector<std::string> SplitCSVLine(const std::string& line)
        {
            std::vector<std::string> result;
            std::stringstream ss(line);
            std::string cell;
            while (std::getline(ss, cell, ',')) {
                // 行末の'\r'(CRLF由来)が混入することがあるので取り除く
                if (!cell.empty() && cell.back() == '\r') {
                    cell.pop_back();
                }
                result.push_back(cell);
            }
            return result;
        }
    }

    // ------------------------------------------------------------
    // CSVを1行ずつ読み込み、クリップ名をキーにしたメタデータの辞書(clips_)を作る。
    //   手順1: ファイルを開く
    //   手順2: 1行目(ヘッダ行)を読み飛ばす
    //   手順3: 2行目以降を1行ずつパースし、clips_へ登録する
    // ------------------------------------------------------------
    bool gmSpriteAnimRegistry::loadFromCSV(const std::string& csvPath)
    {
        // ---- 手順1: ファイルを開く ----
        std::ifstream file(csvPath, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }

        std::string line;

        // ---- 手順2: 1行目はヘッダなので読み飛ばす ----
        // (CSVの先頭にUTF-8のBOMが付いていても、ヘッダ行ごと使わないので影響しない)
        if (!std::getline(file, line)) {
            return false;
        }

        // ---- 手順3: 2行目以降を1行ずつパースする ----
        // 列の並び順: file, cell_size, cols, rows, grid_total, frame_count, fps, source
        while (std::getline(file, line)) {
            if (line.empty()) {
                continue;
            }

            std::vector<std::string> columns = SplitCSVLine(line);
            if (columns.size() < 7) {
                continue; // 列数が足りない壊れた行はスキップする
            }

            gmSpriteAnimClip clip;
            clip.name = columns[0];
            clip.cellSize = std::stoi(columns[1]);
            clip.cols = std::stoi(columns[2]);
            clip.rows = std::stoi(columns[3]);
            // columns[4](grid_total)は末尾の空白コマを含む総コマ数なので使わない
            clip.frameCount = std::stoi(columns[5]);
            clip.fps = std::stof(columns[6]);

            clips_[clip.name] = clip;
        }

        return true;
    }

    // ------------------------------------------------------------
    // クリップ名から、読み込み済みメタデータを引く。
    // ------------------------------------------------------------
    const gmSpriteAnimClip* gmSpriteAnimRegistry::getClip(const std::string& name) const
    {
        auto it = clips_.find(name);
        return (it != clips_.end()) ? &it->second : nullptr;
    }

    // ------------------------------------------------------------
    // クリップ名から、テクスチャを引く。キャッシュに無ければこのタイミングで
    // 初めて実ファイル(imageDir + name + ".png")を読み込み、キャッシュへ登録する。
    // ------------------------------------------------------------
    Shared<dxe::Texture> gmSpriteAnimRegistry::getTexture(const std::string& name, const std::string& imageDir)
    {
        auto it = textureCache_.find(name);
        if (it != textureCache_.end()) {
            return it->second;
        }

        const std::string path = imageDir + name + ".png";
        Shared<dxe::Texture> tex = dxe::Texture::CreateFromFile(path);
        textureCache_[name] = tex;
        return tex;
    }
}
