// gmInputManager.cpp
#include "gmInputManager.h"
#include <algorithm>

namespace gm {

    void gmInputManager::initialize(gmInputLayer initialLayer)
    {
        current_layer_ = initialLayer;

        // ステージ機構は使わないため、デフォルトステージ(0)固定・パッド未使用で生成する。
        input_ = dxe::Input::Create();

        initializeDefaultBindings();
        initializeLayerActions();
    }

    void gmInputManager::finalize()
    {
        input_.reset();
        bindings_.clear();
        layerActions_.clear();
        consumed_flags_.clear();
    }

    void gmInputManager::update()
    {
        // 消費済み(true)のまま放置されているエントリのうち、
        // 実際にボタンが離されたものだけをfalseへ戻す。
        // (離されるまでは「まだ消費済み」を維持し、押しっぱなし中の再消費を防ぐ)
        for (auto& [key, consumed] : consumed_flags_) {
            if (consumed && isReleased(key.first)) {
                consumed = false;
            }
        }
    }

    bool gmInputManager::isActionInCurrentLayer(gmAction action) const
    {
        auto itLayer = layerActions_.find(current_layer_);
        if (itLayer == layerActions_.end()) return false;

        const auto& actions = itLayer->second;
        return std::find(actions.begin(), actions.end(), action) != actions.end();
    }

    bool gmInputManager::isPressed(gmAction action) const
    {
        if (!isActionInCurrentLayer(action)) return false;

        auto it = bindings_.find(action);
        if (it == bindings_.end()) return false;

        return input_ && checkPressed(it->second.buttons);
    }

    bool gmInputManager::isHeld(gmAction action) const
    {
        if (!isActionInCurrentLayer(action)) return false;

        auto it = bindings_.find(action);
        if (it == bindings_.end()) return false;

        return input_ && checkHeld(it->second.buttons);
    }

    bool gmInputManager::isReleased(gmAction action) const
    {
        if (!isActionInCurrentLayer(action)) return false;

        auto it = bindings_.find(action);
        if (it == bindings_.end()) return false;

        return input_ && checkReleased(it->second.buttons);
    }

    bool gmInputManager::consumePress(gmAction action, gmInputCallerId callerId)
    {
        if (!isActionInCurrentLayer(action)) return false;

        const auto key = std::make_pair(action, callerId);

        // 既にこの{action, callerId}が消費済みなら、ボタンが押されたままでも反応しない
        if (consumed_flags_[key]) return false;

        if (isPressed(action)) {
            consumed_flags_[key] = true;
            return true;
        }
        return false;
    }

    bool gmInputManager::checkPressed(const std::vector<dxe::Input::eButton>& buttons) const
    {
        for (const auto& btn : buttons) {
            if (input_->pressed(btn)) return true;
        }
        return false;
    }

    bool gmInputManager::checkHeld(const std::vector<dxe::Input::eButton>& buttons) const
    {
        for (const auto& btn : buttons) {
            if (input_->keep(btn)) return true;
        }
        return false;
    }

    bool gmInputManager::checkReleased(const std::vector<dxe::Input::eButton>& buttons) const
    {
        for (const auto& btn : buttons) {
            if (input_->released(btn)) return true;
        }
        return false;
    }

    // ------------------------------------------------------------
    // デフォルトのキー割り当て。
    // spec(hud_spec)にて確定したgmAction一覧に対応する。
    // ------------------------------------------------------------
    void gmInputManager::initializeDefaultBindings()
    {
        using eButton = dxe::Input::eButton;

        bindings_[gmAction::Ship_SpeedUp]         = { { eButton::KB_W } };
        bindings_[gmAction::Ship_SpeedDown]       = { { eButton::KB_S } };
        bindings_[gmAction::Ship_RudderLeftHold]  = { { eButton::KB_A } };
        bindings_[gmAction::Ship_RudderRightHold] = { { eButton::KB_D } };
        bindings_[gmAction::Ship_RudderLeftStep]  = { { eButton::KB_Q } };
        bindings_[gmAction::Ship_RudderRightStep] = { { eButton::KB_E } };
        bindings_[gmAction::Ship_Recovery]        = { { eButton::KB_5 } };

        bindings_[gmAction::Weapon_Switch1]       = { { eButton::KB_1 } };
        bindings_[gmAction::Weapon_Switch2]       = { { eButton::KB_2 } };
        bindings_[gmAction::Weapon_Switch3]       = { { eButton::KB_3 } };
        bindings_[gmAction::Weapon_Fire]          = { { eButton::MOUSE_LEFT } };

        bindings_[gmAction::System_TogglePauseMenu] = { { eButton::KB_ESCAPE } };

    }

    // ------------------------------------------------------------
    // レイヤー別の有効アクションリスト。
    // ------------------------------------------------------------
    void gmInputManager::initializeLayerActions()
    {
        layerActions_[gmInputLayer::Gameplay] = {
            gmAction::Ship_SpeedUp,
            gmAction::Ship_SpeedDown,
            gmAction::Ship_RudderLeftHold,
            gmAction::Ship_RudderRightHold,
            gmAction::Ship_RudderLeftStep,
            gmAction::Ship_RudderRightStep,
            gmAction::Ship_Recovery,
            gmAction::Weapon_Switch1,
            gmAction::Weapon_Switch2,
            gmAction::Weapon_Switch3,
            gmAction::Weapon_Fire,
            gmAction::System_TogglePauseMenu,
        };

        layerActions_[gmInputLayer::Menu] = {
            gmAction::System_TogglePauseMenu, // 閉じる操作(Escキー)のみMenuレイヤーでも有効にする
        };
    }

}
