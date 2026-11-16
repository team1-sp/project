#include "engine/core/input_actions.hpp"

namespace engine {

namespace {

std::size_t indexOf(InputAction action) {
    return static_cast<std::size_t>(action);
}

} // namespace

const char* inputActionName(InputAction action) {
    switch (action) {
    case InputAction::MoveUp:
        return "MoveUp";
    case InputAction::MoveDown:
        return "MoveDown";
    case InputAction::MoveLeft:
        return "MoveLeft";
    case InputAction::MoveRight:
        return "MoveRight";
    case InputAction::Interact:
        return "Interact";
    }
    return "Unknown";
}

void InputActionState::applyTick(std::span<const InputTransition> transitions) {
    pressed_.fill(false);
    released_.fill(false);
    // Held follows the last transition of each action; an action with none keeps its state.
    for (const InputTransition& transition : transitions) {
        const std::size_t i = indexOf(transition.action);
        if (transition.down) {
            pressed_[i] = true;
        } else {
            released_[i] = true;
        }
        held_[i] = transition.down;
    }
}

bool InputActionState::pressed(InputAction action) const {
    return pressed_[indexOf(action)];
}

bool InputActionState::released(InputAction action) const {
    return released_[indexOf(action)];
}

bool InputActionState::held(InputAction action) const {
    return held_[indexOf(action)];
}

} // namespace engine
