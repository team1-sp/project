// Input action state of a tick (US-1.3; srs.fr.1.14, 5.6, 5.7).
#include <vector>

#include <gtest/gtest.h>

#include "engine/core/input_actions.hpp"

namespace {

using engine::InputAction;
using engine::InputActionState;
using engine::InputTransition;

constexpr InputTransition down(InputAction action) {
    return {action, true};
}

constexpr InputTransition up(InputAction action) {
    return {action, false};
}

} // namespace

TEST(InputActionState, New_NothingPressedReleasedOrHeld) {
    const InputActionState state;

    for (const InputAction action :
         {InputAction::MoveUp, InputAction::MoveDown, InputAction::MoveLeft, InputAction::MoveRight,
          InputAction::Interact}) {
        EXPECT_FALSE(state.pressed(action));
        EXPECT_FALSE(state.released(action));
        EXPECT_FALSE(state.held(action));
    }
}

// srs.fr.5.6 example: down, up, down leaves the action pressed, released and held.
TEST(InputActionState, DownUpDown_PressedReleasedAndHeld) {
    InputActionState state;
    const std::vector<InputTransition> transitions = {
        down(InputAction::Interact), up(InputAction::Interact), down(InputAction::Interact)};

    state.applyTick(transitions);

    EXPECT_TRUE(state.pressed(InputAction::Interact));
    EXPECT_TRUE(state.released(InputAction::Interact));
    EXPECT_TRUE(state.held(InputAction::Interact));
}

// srs.fr.5.6 example: down, up leaves it pressed and released but not held.
TEST(InputActionState, DownUp_PressedAndReleasedButNotHeld) {
    InputActionState state;
    const std::vector<InputTransition> transitions = {down(InputAction::Interact),
                                                      up(InputAction::Interact)};

    state.applyTick(transitions);

    EXPECT_TRUE(state.pressed(InputAction::Interact));
    EXPECT_TRUE(state.released(InputAction::Interact));
    EXPECT_FALSE(state.held(InputAction::Interact));
}

// srs.fr.5.7: pressed lasts one tick; held lasts until an up transition.
TEST(InputActionState, TickWithoutTransitions_KeepsHeldAndClearsPressed) {
    InputActionState state;
    const std::vector<InputTransition> press = {down(InputAction::MoveRight)};
    state.applyTick(press);

    state.applyTick({});

    EXPECT_FALSE(state.pressed(InputAction::MoveRight));
    EXPECT_TRUE(state.held(InputAction::MoveRight));
}

TEST(InputActionState, UpAfterHeld_ReleasedForOneTickThenNothing) {
    InputActionState state;
    const std::vector<InputTransition> press = {down(InputAction::MoveLeft)};
    const std::vector<InputTransition> release = {up(InputAction::MoveLeft)};
    state.applyTick(press);

    state.applyTick(release);
    EXPECT_TRUE(state.released(InputAction::MoveLeft));
    EXPECT_FALSE(state.held(InputAction::MoveLeft));

    state.applyTick({});
    EXPECT_FALSE(state.released(InputAction::MoveLeft));
}

TEST(InputActionState, TransitionsOfOneAction_LeaveOtherActionsAlone) {
    InputActionState state;
    const std::vector<InputTransition> first = {down(InputAction::MoveUp)};
    const std::vector<InputTransition> second = {down(InputAction::Interact)};
    state.applyTick(first);

    state.applyTick(second);

    EXPECT_TRUE(state.held(InputAction::MoveUp));
    EXPECT_FALSE(state.pressed(InputAction::MoveUp));
    EXPECT_TRUE(state.pressed(InputAction::Interact));
}

TEST(InputActionName, EveryAction_HasItsName) {
    EXPECT_STREQ(engine::inputActionName(InputAction::MoveUp), "MoveUp");
    EXPECT_STREQ(engine::inputActionName(InputAction::MoveDown), "MoveDown");
    EXPECT_STREQ(engine::inputActionName(InputAction::MoveLeft), "MoveLeft");
    EXPECT_STREQ(engine::inputActionName(InputAction::MoveRight), "MoveRight");
    EXPECT_STREQ(engine::inputActionName(InputAction::Interact), "Interact");
}
