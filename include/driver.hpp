#pragma once

#include "generator.hpp"
#include "parser.hpp"

namespace monty {

void process(CodeGenerator &generator, Parser &parser) noexcept;
void linkToRuntime(const std::string &output);

void handleExtern(CodeGenerator &generator, Parser &parser) noexcept;
void handleDefinition(CodeGenerator &generator, Parser &parser) noexcept;
void handleTopLevelExpression(CodeGenerator &generator,
                              Parser &parser) noexcept;

} // namespace monty
