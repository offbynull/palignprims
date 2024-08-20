#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/sequences/iota_sequence.h"
#include "gtest/gtest.h"
#include <cstddef>

namespace {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::sequences::iota_sequence::iota_sequence;

    TEST(OASIotaSequenceTest, SanityTest) {
        iota_sequence<true, unsigned int> seq { 3u, 6u };
        static_assert(sequence<decltype(seq)>);
        EXPECT_EQ(seq[0], 3u);
        EXPECT_EQ(seq[1], 4u);
        EXPECT_EQ(seq[2], 5u);
        EXPECT_EQ(seq.size(), 3zu);
    }
}
