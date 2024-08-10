#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/sequences/transform_sequence.h"
#include "gtest/gtest.h"
#include <cstddef>

namespace {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::sequences::transform_sequence::transform_sequence;

    TEST(TransformSequenceTest, SanityTest) {
        // u8 literal suffix not enabled unless special flag is present? -fext-numeric-literals
        transform_sequence<true, std::size_t> seq { 3zu, [](std::size_t idx) { return idx * 2zu; } };
        static_assert(sequence<decltype(seq)>);
        EXPECT_EQ(seq[0], static_cast<std::uint8_t>(0));
        EXPECT_EQ(seq[1], static_cast<std::uint8_t>(2));
        EXPECT_EQ(seq[2], static_cast<std::uint8_t>(4));
        EXPECT_EQ(seq.size(), 3zu);
    }
}
