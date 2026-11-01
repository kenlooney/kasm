#pragma once
#include <string>
#include <vector>

struct Alias {
    std::string symbol;
    std::vector<std::string> values;
};

class AliasTable {
public:
    void add_alias(const Alias& alias, std::string symbol = "") {
        if (!symbol.empty()) {
            Alias new_alias = alias;
            new_alias.symbol = symbol;
            aliases_.push_back(new_alias);
        } else {
            aliases_.push_back(alias);
        }
    }

    const std::vector<Alias>& aliases() const {
        return aliases_;
    }

private:
    std::vector<Alias> aliases_;
};
