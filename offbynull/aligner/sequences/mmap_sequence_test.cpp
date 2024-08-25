#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/sequences/mmap_sequence.h"
#include "offbynull/aligner/sequences/transform_sequence.h"
#include "offbynull/aligner/sequences/chunked_sequence.h"
#include "gtest/gtest.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <ios>

namespace {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::sequences::mmap_sequence::mmap_sequence;
    using offbynull::aligner::sequences::transform_sequence::create_transform_sequence;
    using offbynull::aligner::sequences::chunked_sequence::create_stack_chunked_sequence;

    TEST(OASMmapSequenceTest, SanityTest1) {
        boost::filesystem::path temp_path { boost::filesystem::temp_directory_path() / boost::filesystem::unique_path() };

        std::ofstream file { temp_path.string(), std::ios::binary };
        char x { 100 };
        file.write(&x, 1zu);
        file.close();

        mmap_sequence<true> seq { temp_path.string() };
        EXPECT_EQ(seq[0], 100);
    }

    TEST(OASMmapSequenceTest, SanityTest2) {
        boost::filesystem::path temp_path { boost::filesystem::temp_directory_path() / boost::filesystem::unique_path() };

        std::ofstream file { temp_path.string(), std::ios::binary };
        std::int32_t x { 42 };
        std::int32_t y { 100 };
        file.write(reinterpret_cast<const char*>(&x), sizeof(x));
        file.write(reinterpret_cast<const char*>(&y), sizeof(y));
        file.close();

        mmap_sequence<true> seq1 { temp_path.string() };
        auto seq2 {
            create_stack_chunked_sequence<true, sizeof(std::int32_t)>(
                seq1
            )
        };
        auto seq3 {
            create_transform_sequence<true>(
                seq2,
                [](const auto& chunk) {
                    std::istringstream buffer_stream { chunk.data() };
                    std::int32_t out {};
                    buffer_stream.read(reinterpret_cast<char*>(&out), sizeof(out));
                    return out;
                }
            )
        };
        static_assert(sequence<decltype(seq3)>);
        EXPECT_EQ(seq3[0], 42);
        EXPECT_EQ(seq3[1], 100);
    }
}
