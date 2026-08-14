// gmCursorUtil.cpp
#include "gmCursorUtil.h"
#include <Windows.h> // ClipCursor()
#include <DxLib.h>   // SetMousePoint()

namespace gm {

    void gmCursorUtil::UnlockCursorFromWindow()
    {
        // 引数nullptrで、カーソルの移動範囲の制限を解除する(dxe::LockCursorToWindow()の逆操作)
        ClipCursor(nullptr);
    }

    void gmCursorUtil::SetMousePoint(int x, int y)
    {
        ::SetMousePoint(x, y);
    }

    void gmCursorUtil::GetMousePoint(int& x, int& y)
    {
        ::GetMousePoint(&x, &y);
    }
}
