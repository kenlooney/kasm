#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <utility>

struct OperatorAlias {
    std::string operator_symbol;
    std::vector<std::string> values;
};

class OperatorAliasTable {
public:
    void add_alias(const OperatorAlias& alias, std::string symbol = "") {
        if (!symbol.empty()) {
            OperatorAlias new_alias = alias;
            new_alias.operator_symbol = symbol;
            aliases_.push_back(new_alias);
        } else {
            aliases_.push_back(alias);
        }
    }

    bool load_from_file(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        std::vector<OperatorAlias> loaded_aliases;
        std::string line;
        while (std::getline(file, line)) {
            // Parse the line into operator_symbol and values
            // Example format: operator_symbol value1 value2 ...
            std::istringstream iss(line);
            std::string operator_symbol;
            if (!(iss >> operator_symbol)) {
                continue;
            }
            OperatorAlias alias;
            alias.operator_symbol = operator_symbol;
            std::string value;
            while (iss >> value) {
                alias.values.push_back(value);
            }
            loaded_aliases.push_back(alias);
        }
        if (file.bad()) return false;
        aliases_ = std::move(loaded_aliases);
        return true;
    }
    bool save_to_file(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        for (const auto& alias : aliases_) {
            file << alias.operator_symbol;
            for (const auto& value : alias.values) {
                file << " " << value;
            }
            file << "\n";
        }
        file.close();
        return !file.fail();
    }
    const std::vector<OperatorAlias>& aliases() const {
        return aliases_;
    }

private:
    std::vector<OperatorAlias> aliases_;
};
