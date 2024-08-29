#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/sequences/prefix_pad_sequence.h"
#include "gtest/gtest.h"
#include <string>
#include <type_traits>

namespace {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::sequences::prefix_pad_sequence::prefix_pad_sequence;
    using offbynull::aligner::sequences::prefix_pad_sequence::create_prefix_pad_sequence;

    TEST(OASPrefixPadSequenceTest, SanityTest) {
        std::string data { "hello!" };
        auto seq { create_prefix_pad_sequence<true>(data, 'x', 3zu) };
        static_assert(sequence<decltype(seq)>);
        static_assert(std::is_same_v<decltype(seq), prefix_pad_sequence<true, std::string>>);
        EXPECT_EQ(seq[0], 'x');
        EXPECT_EQ(seq[1], 'x');
        EXPECT_EQ(seq[2], 'x');
        EXPECT_EQ(seq[3], 'h');
        EXPECT_EQ(seq[4], 'e');
        EXPECT_EQ(seq[5], 'l');
        EXPECT_EQ(seq[6], 'l');
        EXPECT_EQ(seq[7], 'o');
        EXPECT_EQ(seq[8], '!');
    }
}
