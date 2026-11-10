#include "engine/core/leak_check.hpp"

// The only compiler-specific code in engine/core, kept in this one file (DEC-046). Other
// compilers skip it; their leak checking is left to sanitizers.
#if defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
#endif

namespace engine {

void enableLeakReportAtExit() {
#if defined(_MSC_VER) && defined(_DEBUG)
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
    // By default the report goes only to the debugger's output window, which a terminal or a
    // CI run never shows, so send it to the standard error output as well.
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
#endif
}

} // namespace engine
