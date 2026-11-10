#pragma once

namespace engine {

// Turns on the C runtime's memory-leak report at process exit (PB-011.3): heap blocks that are
// still allocated when the process ends are listed on the standard error output, after the line
// "Detected memory leaks!". Only Debug builds with the Microsoft C runtime have this report;
// in every other build the call does nothing. Call it once, first thing in main().
void enableLeakReportAtExit();

} // namespace engine
