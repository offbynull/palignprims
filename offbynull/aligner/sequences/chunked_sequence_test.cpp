#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/sequences/chunked_sequence.h"
#include "gtest/gtest.h"
#include <boost/filesystem.hpp>
#include <string>
#include <array>
#include <vector>

namespace {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::sequences::chunked_sequence::compiletime_chunked_sequence;
    using offbynull::aligner::sequences::chunked_sequence::runtime_chunked_sequence;

    TEST(ChunkedSequenceTest, CompiletimeSanityTest) {
        std::string data { "hello!!" };
        compiletime_chunked_sequence<true, std::string, 2zu> seq { data };
        static_assert(sequence<decltype(seq)>);
        EXPECT_EQ(seq[0], (std::array<char, 2zu> { 'h', 'e' }));
        EXPECT_EQ(seq[1], (std::array<char, 2zu> { 'l', 'l' }));
        EXPECT_EQ(seq[2], (std::array<char, 2zu> { 'o', '!' }));
        EXPECT_EQ(seq.size(), 3zu);
    }

    TEST(ChunkedSequenceTest, RuntimeSanityTest) {
        auto to_vector = [](const auto& range) {
            std::vector<char> ret {};
            for (char ch : range) {
                ret.push_back(ch);
            }
            return ret;
        };

        std::string data { "hello!!" };
        runtime_chunked_sequence<true, std::string> seq { data, 2zu };
        static_assert(sequence<decltype(seq)>);
        EXPECT_EQ(to_vector(seq[0]), (std::vector<char> { 'h', 'e' }));
        EXPECT_EQ(to_vector(seq[1]), (std::vector<char> { 'l', 'l' }));
        EXPECT_EQ(to_vector(seq[2]), (std::vector<char> { 'o', '!' }));
        EXPECT_EQ(seq.size(), 3zu);
    }
}
