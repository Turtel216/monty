#pragma once

#include "generator.hpp"
#include "parser.hpp"

namespace monty {
namespace drv {

void process(gen::CodeGenerator &generator, syn::Parser &parser) noexcept;
void linkToRuntime(const std::string &output);
void cleanUp(const std::string &objectFile);

void handleExtern(gen::CodeGenerator &generator, syn::Parser &parser) noexcept;
void handleDefinition(gen::CodeGenerator &generator,
                      syn::Parser &parser) noexcept;
void handleTopLevelExpression(gen::CodeGenerator &generator,
                              syn::Parser &parser) noexcept;
} // namespace drv
} // namespace monty
