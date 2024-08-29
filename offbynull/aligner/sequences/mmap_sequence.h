#ifndef OFFBYNULL_ALIGNER_SEQUENCES_MMAP_SEQUENCE_H
#define OFFBYNULL_ALIGNER_SEQUENCES_MMAP_SEQUENCE_H

#include "offbynull/aligner/sequence/sequence.h"
#include <cstddef>
#include <stdexcept>
#include <string>
#include <boost/iostreams/device/mapped_file.hpp>

namespace offbynull::aligner::sequences::mmap_sequence {
    using offbynull::aligner::sequence::sequence::sequence;
    using boost::iostreams::mapped_file_source;

    template<bool debug_mode>
    class mmap_sequence {
    private:
        mapped_file_source file;

    public:
        mmap_sequence(
            const std::string& path
        )
        : file { path, boost::iostreams::mapped_file_base::readonly } {
            if constexpr (debug_mode) {
                if (!file.is_open()) {
                    throw std::runtime_error { "File not open" };
                }
            }
        }

        char operator[](std::size_t index) const {
            const char* ptr { file.data() + index };
            return *ptr;
        }

        std::size_t size() const {
            return file.size();
        }

        ~mmap_sequence() {
            file.close();
        }
    };

    template<bool debug_mode>
    auto create_mmap_sequence(
        const std::string& path
    ) {
        return mmap_sequence<debug_mode> { path };
    }
}

#endif //OFFBYNULL_ALIGNER_SEQUENCES_MMAP_SEQUENCE_H
