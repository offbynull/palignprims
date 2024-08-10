#ifndef OFFBYNULL_ALIGNER_SEQUENCES_MMAP_SEQUENCE_H
#define OFFBYNULL_ALIGNER_SEQUENCES_MMAP_SEQUENCE_H

#include "offbynull/aligner/sequence/sequence.h"
#include <functional>
#include <cstddef>
#include <stdexcept>
#include <boost/iostreams/device/mapped_file.hpp>

namespace offbynull::aligner::sequences::mmap_sequence {
    using offbynull::aligner::sequence::sequence::sequence;
    using boost::iostreams::mapped_file_source;

    template<bool error_check, typename ELEM>
    class mmap_sequence {
    private:
        mapped_file_source file;
        const std::function<ELEM(const char*)> transformer;
        const std::size_t bytes_per_elem;

    public:
        mmap_sequence(const std::string& path)
        : mmap_sequence {
            path,
            [](const void* ptr) -> ELEM { return *reinterpret_cast<const ELEM*>(ptr); },
            sizeof(ELEM)
        } {}

        mmap_sequence(
            const std::string& path,
            std::function<ELEM(const char*)> transformer_,  // for custom unpacking logic (e.g. if C++ struct is padded but data isn't / endianness between platform and ifle)
            std::size_t bytes_per_elem_                     // for custom unpacking logic (e.g. if C++ struct is padded but data isn't)
        )
        : file { path, boost::iostreams::mapped_file_base::readonly }
        , transformer { transformer_ }
        , bytes_per_elem { bytes_per_elem_ } {
            if constexpr (error_check) {
                if (!file.is_open()) {
                    throw std::runtime_error("File not open");
                }
            }
        }

        ELEM operator[](std::size_t index) const {
            const char* ptr { file.data() };
            ptr += index * bytes_per_elem;
            return transformer(ptr);
        }

        std::size_t size() const {
            return file.size() / bytes_per_elem;
        }

        ~mmap_sequence() {
            file.close();
        }
    };
}

#endif //OFFBYNULL_ALIGNER_SEQUENCES_MMAP_SEQUENCE_H
