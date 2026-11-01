#include "framework/source.hpp"
#include "framework/diagnostic.hpp"

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
    report(source.name, 1, 1, "An example diagnostic");
    return 0;
}
