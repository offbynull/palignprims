#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graphs/pairwise_local_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_global_alignment_graph.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtracker.h"
#include "gtest/gtest.h"
#include <stdfloat>

namespace {
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtracker::backtracker;
    using offbynull::aligner::graphs::pairwise_global_alignment_graph::pairwise_global_alignment_graph;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::edge_type;
    using offbynull::aligner::graphs::middle_sliceable_pairwise_alignment_graph::middle_sliceable_pairwise_alignment_graph;

    TEST(SliceablePairwiseAlignmentGraphBacktrackerTest, GlobalTest) {
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
        backtracker<decltype(g)> backtracker_ { };
        const auto& [path, weight] { backtracker_.find_max_path(g) };
        for (const E& e : path) {
            const auto& [n1, n2] { e };
            std::cout << n1.first << '/' << n1.second << "->" << n2.first << '/' << n2.second << ' ';
        }
        std::cout << std::endl;
        // std::cout << weight << std::endl;

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
    }

    // TEST(SliceablePairwiseAlignmentGraphBacktrackerTest, IsolatedLocalTest) {
    //     auto match_lookup {
    //         [](
    //             const auto& edge,
    //             const char& down_elem,
    //             const char& right_elem
    //         ) -> std::float64_t {
    //             if (down_elem == right_elem) {
    //                 return 1.0f64;
    //             } else {
    //                 return -1.0f64;
    //             }
    //         }
    //     };
    //     auto indel_lookup {
    //         [](
    //             const auto& edge
    //         ) -> std::float64_t {
    //             return 0.0f64;
    //         }
    //     };
    //     auto freeride_lookup {
    //         [](
    //             const auto& edge
    //         ) -> std::float64_t {
    //             return 0.0f64;
    //         }
    //     };
    //     std::string seq1 { "aaaaalmnaaaaa" };
    //     std::string seq2 { "zzzzzlVnzzzzz" };
    //     pairwise_local_alignment_graph<decltype(seq1), decltype(seq2)> g {
    //         seq1,
    //         seq2,
    //         match_lookup,
    //         indel_lookup,
    //         freeride_lookup
    //     };
    //     using N = typename decltype(g)::N;
    //     using E = typename decltype(g)::E;
    //
    //     // walk
    //     sliced_subdivider<decltype(g)> subdivider { g };
    //     auto path { subdivider.subdivide() };
    //     auto backward_path_view { path.walk_path_backward() };
    //     std::vector<E> forward_path(backward_path_view.begin(), backward_path_view.end());
    //     std::reverse(forward_path.begin(), forward_path.end());
    //     for (const E& e : forward_path) {
    //         const auto& [n1, n2] { e.inner_edge };
    //         std::cout << n1.first << '/' << n1.second << "->" << n2.first << '/' << n2.second << ' ';
    //     }
    //     std::cout << std::endl;
    //     // std::cout << weight << std::endl;
    //
    //     std::vector<E> option1 {
    //         E { edge_type::NORMAL, { N {5zu, 5zu}, N {6zu, 6zu} } },
    //         E { edge_type::NORMAL, { N {6zu, 6zu}, N {7zu, 6zu} } },
    //         E { edge_type::NORMAL, { N {7zu, 6zu}, N {7zu, 7zu} } },
    //         E { edge_type::NORMAL, { N {7zu, 7zu}, N {8zu, 8zu} } }
    //     };
    //     std::vector<E> option2 {
    //         E { edge_type::NORMAL, { N {5zu, 5zu}, N {6zu, 6zu} } },
    //         E { edge_type::NORMAL, { N {6zu, 6zu}, N {6zu, 7zu} } },
    //         E { edge_type::NORMAL, { N {6zu, 7zu}, N {7zu, 7zu} } },
    //         E { edge_type::NORMAL, { N {7zu, 7zu}, N {8zu, 8zu} } }
    //     };
    //     EXPECT_TRUE(forward_path == option1 || forward_path == option2);
    // }
}