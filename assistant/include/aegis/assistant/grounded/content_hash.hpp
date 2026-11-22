#pragma once

#include <string>
#include <string_view>

namespace aegis::assistant {

// Content hash of an attached source file at the time of the question (srs.fr.11.4), as
// "sha256:" followed by 64 lowercase hex digits. A later edit of the file changes the hash, so a
// stale source citation is detectable.
[[nodiscard]] std::string contentHash(std::string_view content);

} // namespace aegis::assistant
