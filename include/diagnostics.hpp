#pragma once

#include <string>
#include <vector>

namespace monty {
namespace syn {

struct SourceLoc {
  int line;
  int col;
};

class Diagnostics {
public:
  struct Error {
    std::string message;
    SourceLoc loc;
  };

  void report(const std::string &msg, SourceLoc loc) {
    errors.push_back({msg, loc});
    // fprintf(stderr, "Error at %d:%d: %s\n", loc.line, loc.col, msg.c_str());
    // TODO move to print erros
  }

  const std::vector<Error> &getErrors() const { return errors; }
  bool hasErrors() const { return !errors.empty(); }
  void printErors() const;

private:
  std::vector<Error> errors;
};

} // namespace syn
} // namespace monty
