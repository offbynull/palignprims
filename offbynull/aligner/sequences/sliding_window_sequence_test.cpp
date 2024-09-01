#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/sequences/sliding_window_sequence.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"
#include <string>
#include <vector>

namespace {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::sequences::sliding_window_sequence::sliding_window_sequence;
    using offbynull::aligner::sequences::sliding_window_sequence::create_heap_sliding_window_sequence;
    using offbynull::aligner::sequences::sliding_window_sequence::create_stack_sliding_window_sequence;
    using offbynull::utils::copy_to_vector;
    using offbynull::utils::is_debug_mode;

    TEST(OASSlidingWindowSequenceTest, HeapSanityTest) {
        std::string data { "hell" };
        auto seq { create_heap_sliding_window_sequence<is_debug_mode()>(data, 2zu) };
        static_assert(sequence<decltype(seq)>);
        EXPECT_EQ(copy_to_vector(seq[0]), (std::vector<char> { 'h', 'e' }));
        EXPECT_EQ(copy_to_vector(seq[1]), (std::vector<char> { 'e', 'l' }));
        EXPECT_EQ(copy_to_vector(seq[2]), (std::vector<char> { 'l', 'l' }));
        EXPECT_EQ(seq.size(), 3zu);
    }

    TEST(OASSlidingWindowSequenceTest, StackSanityTest) {
        std::string data { "hell" };
        auto seq { create_stack_sliding_window_sequence<is_debug_mode(), 2zu>(data) };
        static_assert(sequence<decltype(seq)>);
        EXPECT_EQ(copy_to_vector(seq[0]), (std::vector<char> { 'h', 'e' }));
        EXPECT_EQ(copy_to_vector(seq[1]), (std::vector<char> { 'e', 'l' }));
        EXPECT_EQ(copy_to_vector(seq[2]), (std::vector<char> { 'l', 'l' }));
        EXPECT_EQ(seq.size(), 3zu);
    }
}
