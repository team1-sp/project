#pragma once

#include <vector>

#include "engine/core/input_actions.hpp"
#include "engine/core/tick.hpp"

namespace engine {

// Where a tick's input actions come from (srs.fr.5.11). Each tick takes its input from exactly
// one source: None, the keyboard (normal mode only), an input recording or the Playtest Agent.
// The game loop asks the run's source once per tick, in the input application phase, and builds
// the tick's InputActionState from the answer (srs.fr.1.14). Recording playback (US-5.4) and the
// agent (srs.fr.5.3, 9.51) implement this interface; KeyboardQueue and NoInputSource are here.
class IInputSource {
public:
    virtual ~IInputSource() = default;

    // Appends to `transitions`, in the order they occurred, the transitions tick `tick`
    // consumes. Each transition goes to exactly one tick. An implementation must give the same
    // answer for the same tick in normal and headless mode and must not read real time (R-6).
    virtual void takeTransitions(Tick tick, std::vector<InputTransition>& transitions) = 0;
};

// The source None: no actions in any tick (srs.fr.5.11). The default of a headless run, which has
// no keyboard (srs.fr.7.3).
class NoInputSource final : public IInputSource {
public:
    void takeTransitions(Tick tick, std::vector<InputTransition>& transitions) override;
};

// The keyboard queue of normal mode (srs.fr.5.5): step 1 of every render frame pushes the frame's
// keyboard transitions in the order they occurred, and the first tick that runs afterwards takes
// the whole list. When a render frame runs no tick the list stays queued for the next tick, so no
// transition is lost or given to two ticks.
class KeyboardQueue final : public IInputSource {
public:
    void push(InputTransition transition);
    // Drops every queued transition: when a run starts, and while another source is active
    // (srs.fr.5.11).
    void clear();
    [[nodiscard]] bool empty() const;

    void takeTransitions(Tick tick, std::vector<InputTransition>& transitions) override;

private:
    std::vector<InputTransition> queued_;
};

} // namespace engine
