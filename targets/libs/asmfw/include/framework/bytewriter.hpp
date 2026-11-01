#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <ostream>
 
class ByteWriter {
public:
    explicit ByteWriter(const std::string& filename) : filename_(filename) {}
    virtual ~ByteWriter() = default;
    virtual bool write_to_file(const std::vector<std::uint8_t>& bytes) = 0;
    virtual bool write_to_stream(std::ostream& os, const std::vector<std::uint8_t>& bytes) = 0;
    std::string get_filename() const { return filename_; }
    void set_filename(const std::string& filename) { filename_ = filename; }
protected:
    std::string filename_;

};