#include "aegis/assistant/grounded/scripts_folder_reader.hpp"

#include <cstdint>
#include <fstream>
#include <iterator>
#include <system_error>
#include <utility>

#include "aegis/assistant/grounded/context_assembly.hpp"

namespace aegis::assistant {
namespace {

[[nodiscard]] bool isWithin(const std::filesystem::path& path,
                            const std::filesystem::path& folder) {
    auto pathPart = path.begin();
    for (auto folderPart = folder.begin(); folderPart != folder.end(); ++folderPart, ++pathPart) {
        if (pathPart == path.end() || *pathPart != *folderPart) {
            return false;
        }
    }
    return pathPart != path.end();
}

} // namespace

ScriptsFolderReader::ScriptsFolderReader(std::filesystem::path projectRoot)
    : projectRoot_(std::move(projectRoot)) {
}

std::optional<std::string> ScriptsFolderReader::readScript(std::string_view path) const {
    if (refuseScriptPath(path)) {
        return std::nullopt;
    }
    std::error_code error;
    const std::filesystem::path scripts =
        std::filesystem::canonical(projectRoot_ / std::filesystem::path(scriptsFolder), error);
    if (error) {
        return std::nullopt;
    }
    const std::filesystem::path resolved =
        std::filesystem::canonical(projectRoot_ / std::filesystem::path(std::string(path)), error);
    if (error || !isWithin(resolved, scripts) ||
        !std::filesystem::is_regular_file(resolved, error) || error) {
        return std::nullopt;
    }
    const std::uintmax_t size = std::filesystem::file_size(resolved, error);
    if (error || size > maximumSourceBytes) {
        return std::nullopt;
    }
    std::ifstream input(resolved, std::ios::binary);
    if (!input) {
        return std::nullopt;
    }
    std::string content((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    if (input.bad()) {
        return std::nullopt;
    }
    return content;
}

} // namespace aegis::assistant
