#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/sequences/iota_sequence.h"
#include "gtest/gtest.h"
#include <type_traits>

namespace {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::sequences::iota_sequence::iota_sequence;
    using offbynull::aligner::sequences::iota_sequence::create_iota_sequence;

    TEST(OASIotaSequenceTest, SanityTest) {
        auto seq { create_iota_sequence<true>(3u, 6u) };
        static_assert(sequence<decltype(seq)>);
        static_assert(std::is_same_v<decltype(seq), iota_sequence<true, unsigned int>>);
        EXPECT_EQ(seq[0], 3u);
        EXPECT_EQ(seq[1], 4u);
        EXPECT_EQ(seq[2], 5u);
        EXPECT_EQ(seq.size(), 3zu);
    }
}
