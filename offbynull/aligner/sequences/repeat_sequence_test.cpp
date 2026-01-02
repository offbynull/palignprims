#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/sequences/repeat_sequence.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"
#include <string>
#include <type_traits>

namespace {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::sequences::repeat_sequence::repeat_sequence;
    using offbynull::aligner::sequences::repeat_sequence::create_repeat_sequence;
    using offbynull::utils::is_debug_mode;

    TEST(OASRepeatSequenceTest, SanityTest) {
        std::string data { "hello!" };
        auto seq { create_repeat_sequence<is_debug_mode()>(data, 2zu) };
        static_assert(sequence<decltype(seq)>);
        static_assert(std::is_same_v<decltype(seq), repeat_sequence<is_debug_mode(), std::string>>);
        EXPECT_EQ(seq[0], 'h');
        EXPECT_EQ(seq[1], 'e');
        EXPECT_EQ(seq[2], 'l');
        EXPECT_EQ(seq[3], 'l');
        EXPECT_EQ(seq[4], 'o');
        EXPECT_EQ(seq[5], '!');
        EXPECT_EQ(seq[6], 'h');
        EXPECT_EQ(seq[7], 'e');
        EXPECT_EQ(seq[8], 'l');
        EXPECT_EQ(seq[9], 'l');
        EXPECT_EQ(seq[10], 'o');
        EXPECT_EQ(seq[11], '!');
    }
}
