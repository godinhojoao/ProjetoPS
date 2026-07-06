#ifndef EXTREF_RESOLVER_H
#define EXTREF_RESOLVER_H

#include <string>
#include <vector>
#include <cstdint>
#include "global_symbol_table.h"

struct ExtRef {
    std::string symbolName;
    std::vector<uint16_t> offsets;
};

class ExtRefResolver {
public:
    static std::vector<ExtRef> parseExtRefsFromFile(const std::string& objectFilePath);

    static bool resolve(
        const std::vector<ExtRef>& refs,
        const GlobalSymbolTable& gts,
        std::vector<uint8_t>& codeBuffer,
        std::vector<std::string>& errors
    );
};

#endif
