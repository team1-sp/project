// Input sources of the game loop (US-1.3; srs.fr.1.14, 5.5, 5.11).
#include <vector>

#include <gtest/gtest.h>

#include "engine/core/input_actions.hpp"
#include "engine/core/input_source.hpp"

namespace {

using engine::InputAction;
using engine::InputTransition;

} // namespace

TEST(NoInputSource, AnyTick_GivesNoTransitions) {
    engine::NoInputSource source;
    std::vector<InputTransition> transitions;

    source.takeTransitions(0, transitions);
    source.takeTransitions(99, transitions);

    EXPECT_TRUE(transitions.empty());
}

// srs.fr.5.5: the first tick takes the whole queue in the order it was pushed; the next gets none.
TEST(KeyboardQueue, Take_GivesEverythingInOrderOnce) {
    engine::KeyboardQueue queue;
    queue.push({InputAction::MoveUp, true});
    queue.push({InputAction::Interact, true});
    queue.push({InputAction::MoveUp, false});
    std::vector<InputTransition> first;
    std::vector<InputTransition> second;

    queue.takeTransitions(0, first);
    queue.takeTransitions(1, second);

    EXPECT_EQ(first, (std::vector<InputTransition>{{InputAction::MoveUp, true},
                                                   {InputAction::Interact, true},
                                                   {InputAction::MoveUp, false}}));
    EXPECT_TRUE(second.empty());
    EXPECT_TRUE(queue.empty());
}

TEST(KeyboardQueue, Clear_DropsQueuedTransitions) {
    engine::KeyboardQueue queue;
    queue.push({InputAction::MoveLeft, true});

    queue.clear();

    EXPECT_TRUE(queue.empty());
    std::vector<InputTransition> transitions;
    queue.takeTransitions(0, transitions);
    EXPECT_TRUE(transitions.empty());
}
