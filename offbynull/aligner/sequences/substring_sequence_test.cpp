#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/sequences/substring_sequence.h"
#include "gtest/gtest.h"
#include <string>
#include <type_traits>

namespace {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::sequences::substring_sequence::substring_sequence;
    using offbynull::aligner::sequences::substring_sequence::create_substring_sequence;

    TEST(OASSubstringSequenceTest, SanityTest) {
        std::string data { "hello!" };
        auto seq { create_substring_sequence<true>(data, 1zu, 3zu) };
        static_assert(sequence<decltype(seq)>);
        static_assert(std::is_same_v<decltype(seq), substring_sequence<true, std::string>>);
        EXPECT_EQ(seq[0], 'e');
        EXPECT_EQ(seq[1], 'l');
        EXPECT_EQ(seq[2], 'l');
    }
}
