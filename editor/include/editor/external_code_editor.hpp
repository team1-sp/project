#pragma once

#include <string>

#include "editor/console_model.hpp"

namespace editor {

// Launches an external editor process without going through a shell. The executable is supplied
// by the Editor preference/environment and defaults to the portable command name `code` in main.
class ProcessExternalCodeEditor final : public IExternalCodeEditor {
public:
    explicit ProcessExternalCodeEditor(std::string executable);
    bool open(const engine::SourceLocation& source) override;

private:
    std::string executable_;
};

} // namespace editor
