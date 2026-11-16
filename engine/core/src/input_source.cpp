#include "engine/core/input_source.hpp"

namespace engine {

void NoInputSource::takeTransitions(Tick /*tick*/, std::vector<InputTransition>& /*transitions*/) {
}

void KeyboardQueue::push(InputTransition transition) {
    queued_.push_back(transition);
}

void KeyboardQueue::clear() {
    queued_.clear();
}

bool KeyboardQueue::empty() const {
    return queued_.empty();
}

void KeyboardQueue::takeTransitions(Tick /*tick*/, std::vector<InputTransition>& transitions) {
    transitions.insert(transitions.end(), queued_.begin(), queued_.end());
    queued_.clear();
}

} // namespace engine
