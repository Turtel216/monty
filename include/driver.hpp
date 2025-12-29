#pragma once

#include "generator.hpp"
#include "parser.hpp"

namespace monty {

void process(CodeGenerator &generator, Parser &parser) noexcept;

void handleExtern(CodeGenerator &generator, Parser &parser) noexcept;
void handleDefinition(CodeGenerator &generator, Parser &parser) noexcept;
void handleTopLevelExpression(CodeGenerator &generator,
                              Parser &parser) noexcept;

} // namespace monty
