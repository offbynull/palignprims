#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/sequences/iota_sequence.h"
#include "offbynull/aligner/sequences/transform_sequence.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"
#include <cstdint>

namespace {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::sequences::iota_sequence::iota_sequence;
    using offbynull::aligner::sequences::transform_sequence::transform_sequence;
    using offbynull::aligner::sequences::transform_sequence::create_transform_sequence;
    using offbynull::utils::is_debug_mode;

    TEST(OASTransformSequenceTest, SanityTest) {
        iota_sequence<is_debug_mode(), unsigned int> backing_seq { 0u, 3u };
        auto seq {
            create_transform_sequence<is_debug_mode()>(
                backing_seq,
                [](unsigned int x) { return x * 2; }
            )
        };
        static_assert(sequence<decltype(seq)>);
        EXPECT_EQ(seq[0], static_cast<std::uint8_t>(0));
        EXPECT_EQ(seq[1], static_cast<std::uint8_t>(2));
        EXPECT_EQ(seq[2], static_cast<std::uint8_t>(4));
        EXPECT_EQ(seq.size(), 3zu);
    }
}
