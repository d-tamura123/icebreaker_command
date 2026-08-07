// gmRouteLoader.cpp
#include "gmRouteLoader.h"
#include <DxLib.h>
#undef min              // std::sort等でのマクロ競合解消
#undef max
#include <algorithm>

namespace gm
{
    // ------------------------------------------------------------
    // route_*.bin(256×256×1byteの生バイナリ)を読み込み、0以外のマスを
    // 「エンコードされた順序値」から本来の順序へ戻した上で、始点(S)→終点(G)の
    // 順に並べ替えたセル座標のリストとして返す。
    //
    // 処理は大きく3段階に分かれる。
    //   手順1: 256×256の生バイナリを、一旦マス目のままメモリに読み込む
    //   手順2: 0以外のマスを「(本来の順序, セル座標)」のペアとして収集する
    //   手順3: 順序が小さい順に並べ替えて、出力先へ格納する
    // ------------------------------------------------------------
    bool gmRouteLoader::Load(const char* filePath, std::vector<tnl::Vector2i>& outWaypointCellsInOrder)
    {
        outWaypointCellsInOrder.clear();

        int fh = FileRead_open(filePath);
        if (fh == 0)
        {
            return false; // ファイルが存在しない = このインデックスの航路は無い(呼び出し元はここで走査を打ち切る想定)
        }

        // ---- 手順1: 256×256の生バイナリを、一旦マス目のままメモリに読み込む ----
        const int ROUTE_CELL_COUNT = MAP_CHIP_WIDTH * MAP_CHIP_HEIGHT;
        std::vector<uint8_t> rawCells(ROUTE_CELL_COUNT);

        // DXLib の FileRead_read は memcpy と同じ挙動(gmMapLoaderと同様)
        FileRead_read(rawCells.data(), ROUTE_CELL_COUNT, fh);
        FileRead_close(fh);

        // ---- 手順2: 0以外のマスを「(本来の順序, セル座標)」のペアとして収集する ----
        // ファイル上の値は「本来の順序+1」でエンコードされているため、
        // ここで-1して本来の順序(S=0, 経由点=1,2,3,..., G=連番の最大値+1)に戻す。
        struct OrderedCell
        {
            int order;             // ウェイポイントとしての本来の順序(S=0起点)
            tnl::Vector2i cell;    // マップ上のセル座標
        };

        std::vector<OrderedCell> orderedCells;
        orderedCells.reserve(16); // 1航路あたりのウェイポイント数は、多くても数十程度の想定

        for (int y = 0; y < MAP_CHIP_HEIGHT; ++y)
        {
            for (int x = 0; x < MAP_CHIP_WIDTH; ++x)
            {
                const uint8_t encodedValue = rawCells[y * MAP_CHIP_WIDTH + x];
                if (encodedValue == 0)
                {
                    continue; // 未使用マス(航路データなし)
                }

                // エンコード時に足した+1のオフセットを戻し、本来の順序に直す
                const int originalOrder = static_cast<int>(encodedValue) - 1;
                orderedCells.push_back({ originalOrder, tnl::Vector2i(x, y) });
            }
        }

        // ---- 手順3: 順序が小さい順に並べ替えて、出力先へ格納する ----
        // Excelマクロ側(modRouteExporter.bas)で始点/終点の重複・経由点の欠番/重複は
        // エクスポート時点で検証済みという前提のため、ここでは並べ替えのみ行い、
        // 整合性の再検証はしない(壊れたファイルが直接置かれた場合の防御までは行わない)。
        std::sort(orderedCells.begin(), orderedCells.end(),
            [](const OrderedCell& a, const OrderedCell& b) { return a.order < b.order; });

        outWaypointCellsInOrder.reserve(orderedCells.size());
        for (const auto& orderedCell : orderedCells)
        {
            outWaypointCellsInOrder.push_back(orderedCell.cell);
        }

        return true;
    }
}
