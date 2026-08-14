// gmInputManager.h
// 入力管理クラス。
//
// 「レイヤー × アクション × バインディング」のテーブル駆動で入力を管理する。
// キー/ボタンそのものではなく gmAction という意味単位でクエリすることで、
// 呼び出し側(gmPlayerShip等)はキー割り当ての詳細を知らずに済む。
//
// 低レベルAPIは dxe::Input を使用する(tnl::Inputではない)。
// 理由: dxe::Inputはキー/マウス/パッド共通の単一enum(eButton)で扱え、
// common/dxe/dxe.cpp にて tnl::Input と並び既に毎フレーム初期化・更新されているため、
// 採用してもエンジン側の配線コストが増えない。
#pragma once
#include <unordered_map>
#include <map>
#include <vector>
#include <memory>
#include <dxe.h>
#include "gmAction.h"
#include "gmInputCallerId.h"
#include "gmInputLayer.h"

namespace gm {

    class gmInputManager {
    public:
        // 初期化。dxe::Inputのインスタンス生成・デフォルトバインディングの構築を行う。
        // arg1... 開始時点のレイヤー(省略時: Gameplay)
        void initialize(gmInputLayer initialLayer = gmInputLayer::Gameplay);
        void finalize();

        // 毎フレーム呼び出し。consumePress()の消費フラグを、ボタンが離されたタイミングでリセットする。
        // (dxe::Input自体の更新はcommon/dxe/dxe.cpp側で既に毎フレーム行われているため、ここでは行わない)
        void update();

        void setActiveLayer(gmInputLayer layer) { current_layer_ = layer; }
        gmInputLayer getActiveLayer() const { return current_layer_; }

        // ------------------------------------------------------------
        // クエリ系API
        // ------------------------------------------------------------
        // 押した瞬間のみtrue(何度呼んでも状態は変わらない。消費の概念は無い)
        bool isPressed(gmAction action) const;

        // 押している間ずっとtrue
        bool isHeld(gmAction action) const;

        // 離した瞬間のみtrue
        bool isReleased(gmAction action) const;

        // ------------------------------------------------------------
        // 「1回の押下につき1回だけ反応させたい」場合に使う。
        //
        // 過去のプロジェクトで、同じgmActionを複数箇所からconsumePress()した際に
        // 「最初に呼んだ箇所だけtrueを受け取り、以後(ボタンを離すまで)は
        //  他の箇所は常にfalseを受け取る」という不具合があったため、
        // 呼び出し元を識別するcallerIdを引数に取り、消費フラグを
        // {action, callerId}のペアごとに独立させている。
        //
        // arg1... 対象アクション
        // arg2... 呼び出し元ID(gmInputCallerId.h参照。呼び出し箇所ごとに異なる値を渡すこと)
        // ------------------------------------------------------------
        bool consumePress(gmAction action, gmInputCallerId callerId);

    private:
        bool isActionInCurrentLayer(gmAction action) const;
        bool checkPressed(const std::vector<dxe::Input::eButton>& buttons) const;
        bool checkHeld(const std::vector<dxe::Input::eButton>& buttons) const;
        bool checkReleased(const std::vector<dxe::Input::eButton>& buttons) const;

        void initializeDefaultBindings();
        void initializeLayerActions();

        gmInputLayer current_layer_ = gmInputLayer::Gameplay;

        // consumePress()の消費フラグ。{action, callerId}ごとに独立して管理する。
        std::map<std::pair<gmAction, gmInputCallerId>, bool> consumed_flags_;

        struct ActionBinding {
            std::vector<dxe::Input::eButton> buttons;
        };
        std::unordered_map<gmAction, ActionBinding> bindings_;                 // バインディング全リスト
        std::unordered_map<gmInputLayer, std::vector<gmAction>> layerActions_; // レイヤー別のバインディングリスト

        Shared<dxe::Input> input_;
    };

}
