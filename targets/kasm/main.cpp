#include "kasmlexer.hpp"
#include "kasmparser.hpp"
#include "assembler.hpp"
#include <memory>
int main() {

   auto lexer = std::make_unique<KasmLexer>();
   auto parser = std::make_unique<KasmParser>(*lexer);
   auto assembler = std::make_unique<Assembler>("x86", "source code", std::move(lexer), std::move(parser));

    return 0;
}