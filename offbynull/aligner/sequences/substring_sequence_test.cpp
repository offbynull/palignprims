#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/sequences/substring_sequence.h"
#include "gtest/gtest.h"
#include <string>

namespace {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::sequences::substring_sequence::substring_sequence;

    TEST(MiddleSequenceTest, SanityTest) {
        std::string data { "hello!" };
        substring_sequence<true, std::string> seq { data, 1zu, 3zu };
        static_assert(sequence<decltype(seq)>);
        EXPECT_EQ(seq[0], 'e');
        EXPECT_EQ(seq[1], 'l');
        EXPECT_EQ(seq[2], 'l');
    }
}
