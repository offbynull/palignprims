#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graphs/pairwise_local_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_global_alignment_graph.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtracker.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/sliced_walker.h"
#include "gtest/gtest.h"
#include <stdfloat>

namespace {
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtracker::backtracker;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::sliced_walker::slot;
    using offbynull::aligner::graphs::pairwise_global_alignment_graph::pairwise_global_alignment_graph;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph;

    TEST(SliceablePairwiseAlignmentGraphBacktrackTest, ForwardWalkWithoutResidents) {
        auto match_lookup {
            [](
                const auto& edge,
                const char& down_elem,
                const char& right_elem
            ) -> std::float64_t {
                if (down_elem == right_elem) {
                    return 1.0f64;
                } else {
                    return -1.0f64;
                }
            }
        };
        auto indel_lookup {
            [](
                const auto& edge
            ) -> std::float64_t {
                return 0.0f64;
            }
        };
        std::string seq1 { "abc" };
        std::string seq2 { "azc" };
        pairwise_global_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            match_lookup,
            indel_lookup
        };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        // walk
        backtracker<decltype(g), std::float64_t> _backtracker{ g };
        const auto& [path, weight] {
            _backtracker.find_max_path(
                g
            )
        };
        for (const E& e : path) {
            const auto& [n1, n2] { e };
            std::cout << n1.first << '/' << n1.second << "->" << n2.first << '/' << n2.second << ' ';
        }
        std::cout << std::endl;
        std::cout << weight << std::endl;

        std::vector<E> option1 {
            E { N {0zu, 0zu}, N {1zu, 1zu} },
            E { N {1zu, 1zu}, N {2zu, 1zu} },
            E { N {2zu, 1zu}, N {2zu, 2zu} },
            E { N {2zu, 2zu}, N {3zu, 3zu} }
        };
        std::vector<E> option2 {
            E { N {0zu, 0zu}, N {1zu, 1zu} },
            E { N {1zu, 1zu}, N {1zu, 2zu} },
            E { N {1zu, 2zu}, N {2zu, 2zu} },
            E { N {2zu, 2zu}, N {3zu, 3zu} }
        };
        EXPECT_TRUE(path == option1 || path == option2);

        // EXPECT_EQ(
        //     path,
        //     (std::vector<E> {
        //         { std::pair{0zu, 0zu}, std::pair{1zu, 1zu} },
        //         { std::pair{1zu, 1zu}, std::pair{2zu, 1zu} },
        //         { std::pair{2zu, 1zu}, std::pair{2zu, 2zu} },
        //         { std::pair{2zu, 2zu}, std::pair{3zu, 3zu} }
        //     })
        // );
    }
}
