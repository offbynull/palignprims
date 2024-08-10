#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/sequences/sliding_window_sequence.h"
#include "gtest/gtest.h"
#include <boost/filesystem.hpp>
#include <string>
#include <array>
#include <vector>

namespace {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::sequences::sliding_window_sequence::compiletime_sliding_window_sequence;
    using offbynull::aligner::sequences::sliding_window_sequence::runtime_sliding_window_sequence;

    TEST(SlidingWindowSequenceTest, CompiletimeSanityTest) {
        std::string data { "hell" };
        compiletime_sliding_window_sequence<true, std::string, 2zu> seq { data };
        static_assert(sequence<decltype(seq)>);
        EXPECT_EQ(seq[0], (std::array<char, 2zu> { 'h', 'e' }));
        EXPECT_EQ(seq[1], (std::array<char, 2zu> { 'e', 'l' }));
        EXPECT_EQ(seq[2], (std::array<char, 2zu> { 'l', 'l' }));
        EXPECT_EQ(seq.size(), 3zu);
    }

    TEST(SlidingWindowSequenceTest, RuntimeSanityTest) {
        auto to_vector = [](const auto& range) {
            std::vector<char> ret {};
            for (char ch : range) {
                ret.push_back(ch);
            }
            return ret;
        };

        std::string data { "hell" };
        runtime_sliding_window_sequence<true, std::string> seq { data, 2zu };
        static_assert(sequence<decltype(seq)>);
        EXPECT_EQ(to_vector(seq[0]), (std::vector<char> { 'h', 'e' }));
        EXPECT_EQ(to_vector(seq[1]), (std::vector<char> { 'e', 'l' }));
        EXPECT_EQ(to_vector(seq[2]), (std::vector<char> { 'l', 'l' }));
        EXPECT_EQ(seq.size(), 3zu);
    }
}
