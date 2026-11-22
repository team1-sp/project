#include "editor/external_code_editor.hpp"

#include <cstdint>
#include <string>
#include <utility>

#ifdef _WIN32
#include <process.h>
#else
#include <thread>

#include <spawn.h>
#include <sys/wait.h>

extern char** environ;
#endif

namespace editor {

ProcessExternalCodeEditor::ProcessExternalCodeEditor(std::string executable)
    : executable_(std::move(executable)) {
}

bool ProcessExternalCodeEditor::open(const engine::SourceLocation& source) {
    std::string target = source.file;
    if (source.line > 0) {
        target += ":" + std::to_string(source.line);
    }

#ifdef _WIN32
    const char* arguments[] = {executable_.c_str(), "--goto", target.c_str(), nullptr};
    return _spawnvp(_P_DETACH, executable_.c_str(), arguments) != -1;
#else
    pid_t process = 0;
    char* arguments[] = {executable_.data(), const_cast<char*>("--goto"), target.data(), nullptr};
    if (posix_spawnp(&process, executable_.c_str(), nullptr, nullptr, arguments, environ) != 0) {
        return false;
    }
    // Reap the detached editor command without blocking the UI thread.
    std::thread([process] {
        int status = 0;
        (void)waitpid(process, &status, 0);
    }).detach();
    return true;
#endif
}

} // namespace editor
