#include "framework/bytewriter.hpp"

enum class KasmEndianness {
    Little,
    Big
};
class KasmWriter : public ByteWriter {
public:
    explicit KasmWriter(const std::string& filename, KasmEndianness endianness) : ByteWriter(filename), endianness_(endianness) {}
    bool write_to_file(const std::vector<std::uint8_t>& bytes) override;
    bool write_to_stream(std::ostream& os, const std::vector<std::uint8_t>& bytes) override;
    std::vector<std::uint8_t> get_buffer() const { return buffer_; }
    void set_buffer(const std::vector<std::uint8_t>& buffer) { buffer_ = buffer; }
    void clear_buffer() { buffer_.clear(); }
    KasmEndianness get_endianness() const { return endianness_; }
    void set_endianness(KasmEndianness endianness) { endianness_ = endianness; }
private:
    KasmEndianness endianness_;
    std::vector<std::uint8_t> buffer_;
};