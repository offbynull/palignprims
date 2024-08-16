#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/sequences/chunked_sequence.h"
#include "gtest/gtest.h"
#include <boost/filesystem.hpp>
#include <string>
#include <array>
#include <vector>

namespace {
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::sequences::chunked_sequence::chunked_sequence;

    TEST(ChunkedSequenceTest, RuntimeSanityTest) {
        auto to_vector = [](const auto& range) {
            std::vector<char> ret {};
            for (char ch : range) {
                ret.push_back(ch);
            }
            return ret;
        };

        std::string data { "hello!!" };
        chunked_sequence<true, std::string> seq { data, 2zu };
        static_assert(sequence<decltype(seq)>);
        EXPECT_EQ(to_vector(seq[0]), (std::vector<char> { 'h', 'e' }));
        EXPECT_EQ(to_vector(seq[1]), (std::vector<char> { 'l', 'l' }));
        EXPECT_EQ(to_vector(seq[2]), (std::vector<char> { 'o', '!' }));
        EXPECT_EQ(seq.size(), 3zu);
    }
}
