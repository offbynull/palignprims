#include <tuple>
#include <type_traits>
#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/sequences/iota_sequence.h"
#include "offbynull/aligner/sequences/zip_sequence.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::sequences::iota_sequence::iota_sequence;
    using offbynull::aligner::sequences::zip_sequence::zip_sequence;
    using offbynull::aligner::sequences::zip_sequence::create_zip_sequence;

    TEST(OASZipSequenceTest, SanityTest) {
        iota_sequence<true, unsigned int> seq1 { 10u, 15u };
        iota_sequence<true, unsigned int> seq2 { 20u, 24u };
        iota_sequence<true, unsigned int> seq3 { 30u, 36u };
        auto seq { create_zip_sequence<true>(seq1, seq2, seq3) };
        static_assert(sequence<decltype(seq)>);
        static_assert(std::is_same_v<decltype(seq), zip_sequence<true, decltype(seq1), decltype(seq2), decltype(seq3)>>);
        EXPECT_EQ(seq[0], std::make_tuple(10, 20, 30));
        EXPECT_EQ(seq[1], std::make_tuple(11, 21, 31));
        EXPECT_EQ(seq[2], std::make_tuple(12, 22, 32));
        EXPECT_EQ(seq[3], std::make_tuple(13, 23, 33));
        EXPECT_EQ(seq.size(), 4zu);
    }
}
