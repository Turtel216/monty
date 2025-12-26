#include "../include/parser.hpp"

int main(int argc, char *argv[]) {

  ggc::Parser parser;
  fprintf(stderr, "ready> ");
  parser.getNextToken();

  parser.replLoop();

  return 0;
}
