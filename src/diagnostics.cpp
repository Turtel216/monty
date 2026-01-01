#include "../include/diagnostics.hpp"
#include <cstdio>
#include <cstdlib>
#include <iostream>

namespace monty {
namespace syn {
void Diagnostics::printErors() const {
  for (const auto &err : this->errors) {
    fprintf(stderr, "Error at %d:%d: %s\n", err.loc.line, err.loc.col,
            err.message.c_str());
  }
}
} // namespace syn
} // namespace monty
