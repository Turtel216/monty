#include "../include/diagnostics.hpp"
#include <cstdlib>
#include <iostream>

namespace ggc {
[[nodiscard]] bool DiagnosticEngine::hasError() const noexcept {
  return errors.empty();
}

void DiagnosticEngine::report(SourceLocation loc, DiagCode code,
                              std::string details) noexcept {
  std::cout << "DiagnosticsEngine::report is not implemented\n";
  std::exit(1);
}

} // namespace ggc
