#include "kasm/kasmwriter.hpp"
#include <algorithm>
#include <fstream>

bool KasmWriter::write_to_file(const std::vector<std::uint8_t>& bytes) {

    std::ofstream ofs(filename_, std::ios::binary);
    if (!ofs) {
        return false;
    }
    return write_to_stream(ofs, bytes);
}

bool KasmWriter::write_to_stream(std::ostream& os, const std::vector<std::uint8_t>& bytes) {
    if (!os) {
        return false;
    }
    if (endianness_ == KasmEndianness::Big) {
        auto reversed = bytes;
        std::reverse(reversed.begin(), reversed.end());
        os.write(reinterpret_cast<const char*>(reversed.data()), reversed.size());
    } else {
        os.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    }
    return os.good();
}
