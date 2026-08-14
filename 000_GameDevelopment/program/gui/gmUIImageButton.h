// gmUIImageButton.h
// 汎用の画像ボタン(通常/ホバー/押下の3状態)。
//
// クリック時の処理は呼び出し側から渡されたコールバックに委譲する
// 「見た目(3枚の画像)とクリック時の処理だけが違うボタン」であれば汎用的に使い回す。
//
// ホバー/クリック判定は、setInputEnabled(true)の間だけ行う。
// このゲームは通常時カーソルを非表示+画面中央に固定しているため(gmPlayerCameraController参照)、
// カーソルモード(Alt押下中)でない間は「カーソルがボタン上にあるか」という判定自体に意味が無いため、
// 呼び出し側(gmTopBarUI等)からカーソルモードの状態をそのまま渡してもらう想定。
#pragma once
#include "gmUIObjectBase.h"
#include <dxe.h>
#include <functional>
#include <string>

namespace gm {

    class gmUIImageButton : public gmUIObjectBase {
    public:
        // arg1... 左上座標
        // arg2... 一辺のサイズ(正方形前提)
        // arg3... 通常時の画像パス
        // arg4... ホバー時の画像パス
        // arg5... 押下時の画像パス
        // arg6... クリック確定時(ボタン上で押して、ボタン上で離した瞬間)に呼ばれるコールバック
        gmUIImageButton(
            const tnl::Vector2f& pos,
            float size,
            const std::string& normalImagePath,
            const std::string& hoverImagePath,
            const std::string& pressedImagePath,
            std::function<void()> onClick);

        void setInputEnabled(bool enabled) { inputEnabled_ = enabled; }

        void update(float dt) override;
        void draw() override;

    private:
        enum class eState { Normal, Hover, Pressed };

        float size_ = 0.0f;
        int hNormal_ = -1;
        int hHover_ = -1;
        int hPressed_ = -1;

        eState state_ = eState::Normal;
        bool inputEnabled_ = false;
        bool wasPressedWhileHovering_ = false; // クリック確定判定(押下→ホバー中に離す)用

        std::function<void()> onClick_;
        Shared<dxe::Input> input_; // マウス左ボタンの状態専用(gmInputManagerのgmActionは経由しない。理由はヘッダ冒頭コメント参照)
    };

}
