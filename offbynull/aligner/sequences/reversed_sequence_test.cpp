#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/sequences/reversed_sequence.h"
#include "gtest/gtest.h"
#include <string>

namespace {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::sequences::reversed_sequence::reversed_sequence;

    TEST(ReversedSequenceTest, SanityTest) {
        std::string data { "hello!" };
        reversed_sequence<true, std::string> seq { data };
        static_assert(sequence<decltype(seq)>);
        EXPECT_EQ(seq[0], '!');
        EXPECT_EQ(seq[1], 'o');
        EXPECT_EQ(seq[2], 'l');
        EXPECT_EQ(seq[3], 'l');
        EXPECT_EQ(seq[4], 'e');
        EXPECT_EQ(seq[5], 'h');
    }
}
