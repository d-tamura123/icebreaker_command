// gmRouteCenterlineUtil.h
#pragma once
#include <vector>
#include <dxe.h>

namespace gm {

    // ------------------------------------------------------------
    // 航路のウェイポイント列(Excel側S/1/2/.../Gに対応する、間隔が不揃いな点列)を、
    // 遠心的(centripetal)Catmull-Rom補間で密なポリライン(中心線)に変換する共通関数。
    //
    // IC_ExcelMapTool側 modRouteExporter.bas の DrawRouteCurve()、および
    // gmRouteVisualizer(見た目のリボン)で使っているのと全く同じ式のC++実装。
    // gmTradeShip(実際の航行)もこれと同じ中心線を辿ることで、
    // 「見た目の航路」と「実際に船が通る経路」が一致するようにしている
    // (もし別々の計算式を使うと、見た目では島から十分離れているはずの航路を
    //  実際の船が違う軌道で通ってしまい、Excel側で検証した島クリアランスが
    //  無意味になってしまう)。
    //
    // arg1... 航路のウェイポイント(ワールド座標、S→Gの順)
    // arg2... 中心線をサンプリングする間隔(world単位)。小さいほど滑らかだが点数が増える
    // ret.... 密なポリライン(ワールド座標、S→Gの順)
    // ------------------------------------------------------------
    std::vector<tnl::Vector2f> SampleRouteCenterline(
        const std::vector<tnl::Vector2f>& waypointsWorld,
        float sampleStep);

}
