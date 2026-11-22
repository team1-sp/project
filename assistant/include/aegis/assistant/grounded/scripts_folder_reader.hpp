#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

#include "aegis/assistant/grounded/evidence_sources.hpp"

namespace aegis::assistant {

// Production ProjectSourceReader: reads, never writes, regular files inside
// <projectRoot>/Scripts. Paths refused by refuseScriptPath(), files whose resolved location leaves
// the Scripts folder (for example through a symbolic link) and files over 200 KB are not read.
class ScriptsFolderReader final : public ProjectSourceReader {
public:
    explicit ScriptsFolderReader(std::filesystem::path projectRoot);

    [[nodiscard]] std::optional<std::string> readScript(std::string_view path) const override;

private:
    std::filesystem::path projectRoot_;
};

} // namespace aegis::assistant
