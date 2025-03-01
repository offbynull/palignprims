#ifndef OFFBYNULL_ALIGNER_SEQUENCES_MMAP_SEQUENCE_H
#define OFFBYNULL_ALIGNER_SEQUENCES_MMAP_SEQUENCE_H

#include "offbynull/aligner/sequence/sequence.h"
#include <cstddef>
#include <stdexcept>
#include <string>
#include <boost/iostreams/device/mapped_file.hpp>

/**
 * @ref offbynull::aligner::sequence::sequence::sequence backed by a mmap'd file.
 *
 * @author Kasra Faghihi
 */
namespace offbynull::aligner::sequences::mmap_sequence {
    using offbynull::aligner::sequence::sequence::sequence;
    using boost::iostreams::mapped_file_source;

    /**
     * An @ref offbynull::aligner::sequence::sequence::sequence backed by a mmap'd file.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     */
    template<bool debug_mode>
    class mmap_sequence {
    private:
        mapped_file_source file;

    public:
        /**
         * Construct an offbynull::aligner::sequences::mmap_sequence::mmap_sequence instance.
         *
         * @param path Path of file to be mmap'd.
         * @throws std::runtime_error If `debug_mode` and problems mmap'ing `path`.
         */
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

        /**
         * Get element at index `index`.
         *
         * @param index Index of element.
         * @return Value at `index`.
         */
        char operator[](std::size_t index) const {
            const char* ptr { file.data() + index };
            return *ptr;
        }

        /**
         * Get number of elements.
         *
         * @return Number of elements.
         */
        std::size_t size() const {
            return file.size();
        }

        ~mmap_sequence() {
            file.close();
        }
    };

    /**
     * Convenience function to create an @ref offbynull::aligner::sequences::mmap_sequence::mmap_sequence instance.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @param path Path of file to be mmap'd.
     * @return Newly created @ref offbynull::aligner::sequences::mmap_sequence::mmap_sequence instance.
     * @throws std::runtime_error If `debug_mode` and problems mmap'ing `path`.
     */
    template<bool debug_mode>
    auto create_mmap_sequence(
        const std::string& path
    ) {
        return mmap_sequence<debug_mode> { path };
    }
}

#endif //OFFBYNULL_ALIGNER_SEQUENCES_MMAP_SEQUENCE_H
