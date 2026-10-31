#include "kasmlexer.hpp"
#include "kasmparser.hpp"
#include "assembler.hpp"
#include <memory>
#include <iostream>
#include <vector>
#include "aliastable.hpp"
int main() {


   auto lexer = std::make_unique<KasmLexer>();
   auto parser = std::make_unique<KasmParser>(*lexer);
   for(const auto& operators : parser->operator_aliases())
   {
      std::cout << "Operator: " << operators.symbol << " operatorses: ";
      for(const auto& value : operators.values)
          std::cout << value << " ";
      std::cout << std::endl;
   }
   for(const auto& keywords : parser->keyword_aliases())
   {
      std::cout << "Keyword: " << keywords.symbol << " aliases: ";
      for(const auto& value : keywords.values)
          std::cout << value << " ";
      std::cout << std::endl;
   }
   auto assembler = std::make_unique<Assembler>("x86", "source code", std::move(lexer), std::move(parser));
   return 0;
}
