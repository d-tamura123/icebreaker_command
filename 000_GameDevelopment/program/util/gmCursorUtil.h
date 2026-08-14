// gmCursorUtil.h
// マウスカーソルの制御のうち、dxe側に用意されていない2機能をここで補う。
//
// 経緯: 当初はdxe(common/dxe/dxe.cpp)に直接UnlockCursorFromWindow()/SetMousePoint()を
// 追加していたが、common/以下は担当外の成果物のため変更不可(AGENTS.md参照)と分かったため、
// game側(program/util/)に切り出した。
//
// dxe::LockCursorToWindow() / dxe::SetVisibleMousePointer() は既存のdxe機能(変更なし)を
// そのまま利用し、ここで補うのは以下の2つだけ:
//   - UnlockCursorFromWindow() : LockCursorToWindow()で固定したカーソルの制限を解除する
//                                (Win32のClipCursor(nullptr)相当。dxe側にLockはあるがUnlockが無いため)
//   - SetMousePoint()          : カーソル座標を指定位置へ強制的にセットする
//                                (DxLibのSetMousePoint()のラッパー。dxe側に無いため)
#pragma once

namespace gm {

    class gmCursorUtil {
    public:
        // dxe::LockCursorToWindow()で固定したマウスカーソルの制限を解除する。
        static void UnlockCursorFromWindow();

        // マウスカーソルの座標を指定位置へ強制的にセットする。
        // 座標系は dxe::Input の GetMousePoint と同じ(ウィンドウのクライアント領域基準)。
        static void SetMousePoint(int x, int y);

        // マウスカーソルの現在座標を取得する(DxLibのGetMousePoint()のラッパー)。
        // dxe::Inputは自身のSetMousePoint()呼び出しを検知できず、次フレームの移動量計算に
        // ワープ分を誤って混入させてしまう(詳細はgmPlayerCameraController参照)ため、
        // カーソルを毎フレーム中央へ戻す用途ではdxe::Inputの移動量を使わず、
        // この関数で自前に前フレームとの差分を取る必要がある。
        static void GetMousePoint(int& x, int& y);
    };

}
