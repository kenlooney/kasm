#include "source.hpp"
#include "diagnostic.hpp"
#include "span.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        report("<command line>", 1, 1, "Expected a source file");
        return 1;
    }
    Source source;
    if (!Source::load(argv[1], source)) {
        report(argv[1], 1, 1, "Could not open file");
        return 1;
    }
    Span span{0, source.text.size()};
    std::cout << "Bytes in source: " << span.end - span.begin << "\n";
    return 0;
}
