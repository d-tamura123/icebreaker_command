// gmUIStrings.h
// UI文言の一元管理
// 
// ToDo:
// 将来本格的な多言語対応をする際に「この1ファイルの中身を、言語ごとの文字列テーブルからの
// 検索に差し替えるだけ」で済むようにしておくための準備。
#pragma once

namespace gm {
    namespace UIStrings {

        // ---- ポーズメニュー(gmPauseMenuUI) ----
        constexpr const char* PAUSE_MENU_TITLE          = "PAUSED";
        constexpr const char* PAUSE_MENU_RESUME         = "Resume";
        constexpr const char* PAUSE_MENU_RESTART        = "Restart";
        constexpr const char* PAUSE_MENU_RETURN_TO_START = "Return to Start";
        constexpr const char* PAUSE_MENU_RETURN_TO_TITLE = "Return to Title";

        // ポーズメニューの各項目に対応する確認ダイアログの本文(26文字程度/行を目安に収めること)
        constexpr const char* CONFIRM_RESTART           = "Restart? Progress lost.";
        constexpr const char* CONFIRM_RETURN_TO_START   = "Return to start point?";
        constexpr const char* CONFIRM_RETURN_TO_TITLE   = "Return to title screen?";

        // ---- 汎用確認ダイアログ(gmConfirmDialogUI)のボタンラベル ----
        constexpr const char* CONFIRM_DIALOG_YES = "YES";
        constexpr const char* CONFIRM_DIALOG_NO  = "NO";

    }
}
