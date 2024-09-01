#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/sequences/reversed_sequence.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"
#include <string>
#include <type_traits>

namespace {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::sequences::reversed_sequence::reversed_sequence;
    using offbynull::aligner::sequences::reversed_sequence::create_reversed_sequence;
    using offbynull::utils::is_debug_mode;

    TEST(OASReversedSequenceTest, SanityTest) {
        std::string data { "hello!" };
        auto seq { create_reversed_sequence<is_debug_mode()>(data) };
        static_assert(sequence<decltype(seq)>);
        static_assert(std::is_same_v<decltype(seq), reversed_sequence<is_debug_mode(), std::string>>);
        EXPECT_EQ(seq[0], '!');
        EXPECT_EQ(seq[1], 'o');
        EXPECT_EQ(seq[2], 'l');
        EXPECT_EQ(seq[3], 'l');
        EXPECT_EQ(seq[4], 'e');
        EXPECT_EQ(seq[5], 'h');
    }
}
