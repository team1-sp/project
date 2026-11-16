#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace engine {

// The input actions of Phase 1 (DEC-042 item 4, srs.fr.5.2). A project's Input Map, which can
// remove and rename actions (srs.fr.5.22 onwards), replaces this fixed list with the Editor.
enum class InputAction : std::uint8_t {
    MoveUp,
    MoveDown,
    MoveLeft,
    MoveRight,
    Interact,
};

inline constexpr std::size_t inputActionCount = 5;

// The action's name as recordings and logs write it: "MoveUp", "MoveDown", "MoveLeft",
// "MoveRight", "Interact".
[[nodiscard]] const char* inputActionName(InputAction action);

// One change of an action: it went down (pressed) or up (released) (srs.fr.5.5).
struct InputTransition {
    InputAction action = InputAction::MoveUp;
    bool down = false;

    friend bool operator==(const InputTransition&, const InputTransition&) = default;
};

// The input action state a tick reads: which actions were pressed or released in this tick and
// which are held at its end (srs.fr.1.14, 5.6, 5.7). The game loop keeps one for the run and
// applies each tick's ordered transition list to it in the input application phase.
class InputActionState {
public:
    // Applies the ordered transitions a tick consumes, by the rules of srs.fr.5.6: an action is
    // pressed in the tick if any of its transitions is down and released if any is up; it is held
    // at the end of the tick if its last transition is down, or if it has none and was held
    // before. Pressed and released last only for this tick (srs.fr.5.7). An empty list keeps the
    // held actions and clears pressed and released.
    void applyTick(std::span<const InputTransition> transitions);

    [[nodiscard]] bool pressed(InputAction action) const;
    [[nodiscard]] bool released(InputAction action) const;
    [[nodiscard]] bool held(InputAction action) const;

    friend bool operator==(const InputActionState&, const InputActionState&) = default;

private:
    std::array<bool, inputActionCount> pressed_{};
    std::array<bool, inputActionCount> released_{};
    std::array<bool, inputActionCount> held_{};
};

} // namespace engine
