#pragma once

#include "../include/compiler_error.hpp"
#include <string>
#include <vector>

namespace ggc {

enum class DiagCode {
  ExpectedSemicolon,
  // TODO
};

class SourceLocation {
public:
  std::string file;
  int line;
  int col;

  SourceLocation(std::string _file, int _line, int _col) noexcept
      : file(_file), line(_line), col(_col) {}
};

class DiagnosticEngine {
private:
  std::vector<CompilerError> errors;

public:
  void report(SourceLocation loc, DiagCode code, std::string details) noexcept;
  [[nodiscard]] bool hasError() const noexcept;
};
} // namespace ggc
