#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/sequences/mmap_sequence.h"
#include "gtest/gtest.h"
#include <boost/filesystem.hpp>
#include <fstream>

namespace {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::sequences::mmap_sequence::mmap_sequence;

    TEST(MmapSequenceTest, SanityTest) {
        boost::filesystem::path temp_path { boost::filesystem::temp_directory_path() / boost::filesystem::unique_path() };

        std::ofstream file { temp_path.string(), std::ios::binary };
        int x { 42 };
        int y { 100 };
        file.write(reinterpret_cast<char*>(&x), sizeof(x));
        file.write(reinterpret_cast<char*>(&y), sizeof(y));
        file.close();

        mmap_sequence<true, int> seq { temp_path.string() };
        static_assert(sequence<decltype(seq)>);
        EXPECT_EQ(seq[0], 42);
        EXPECT_EQ(seq[1], 100);
    }
}
