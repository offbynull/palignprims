#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/sequences/suffix_pad_sequence.h"
#include "gtest/gtest.h"
#include <string>

namespace {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::sequences::suffix_pad_sequence::suffix_pad_sequence;

    TEST(OASSuffixPadSequenceTest, SanityTest) {
        std::string data { "hello!" };
        suffix_pad_sequence<true, std::string> seq { data, 'x', 3zu };
        static_assert(sequence<decltype(seq)>);
        EXPECT_EQ(seq[0], 'h');
        EXPECT_EQ(seq[1], 'e');
        EXPECT_EQ(seq[2], 'l');
        EXPECT_EQ(seq[3], 'l');
        EXPECT_EQ(seq[4], 'o');
        EXPECT_EQ(seq[5], '!');
        EXPECT_EQ(seq[6], 'x');
        EXPECT_EQ(seq[7], 'x');
        EXPECT_EQ(seq[8], 'x');
    }
}
