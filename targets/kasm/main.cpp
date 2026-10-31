#include "kasmlexer.hpp"
#include "kasmparser.hpp"
#include "assembler.hpp"
#include <memory>
#include <iostream>
#include <vector>
#include "operatoralias.hpp"
int main() {


   auto lexer = std::make_unique<KasmLexer>();
   auto parser = std::make_unique<KasmParser>(*lexer);
   for(const auto& alias : parser->operator_aliases())
   {
      std::cout << "Operator: " << alias.operator_symbol << " Aliases: ";
      for(const auto& value : alias.values)
          std::cout << value << " ";
      std::cout << std::endl;
   }
   auto assembler = std::make_unique<Assembler>("x86", "source code", std::move(lexer), std::move(parser));
   return 0;
}
