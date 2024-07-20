#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graphs/pairwise_local_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_global_alignment_graph.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/sliced_subdivider.h"
#include "gtest/gtest.h"
#include <stdfloat>

namespace {
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::sliced_subdivider::sliced_subdivider;
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
        sliced_subdivider<decltype(g)> subdivider { g };
        auto path { subdivider.subdivide() };
        std::vector<E> path_as_vector {};
        for (const E& e : path.walk_path_backward(g)) {
            path_as_vector.insert(path_as_vector.begin(), e);
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
        EXPECT_TRUE(path_as_vector == option1 || path_as_vector == option2);
    }

    // TEST(SliceablePairwiseAlignmentGraphBacktrackTest, ForwardWalkWithResidents) {
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
    //             return -1.0f64;
    //         }
    //     };
    //     auto freeride_lookup {
    //         [](
    //             const auto& edge
    //         ) -> std::float64_t {
    //             return 0.0f64;
    //         }
    //     };
    //     std::string seq1 { "aalmnaa" };
    //     std::string seq2 { "zzlmnzz" };
    //     pairwise_local_alignment_graph<decltype(seq1), decltype(seq2)> g {
    //         seq1,
    //         seq2,
    //         match_lookup,
    //         indel_lookup,
    //         freeride_lookup
    //     };
    //
    //     using N = typename decltype(g)::N;
    //     using E = typename decltype(g)::E;
    //
    //     // walk classical
    //     {
    //         classical_backtracker::backtracker<decltype(g), std::size_t> _backtracker{};
    //         const auto& [path, weight] {
    //             _backtracker.find_max_path(
    //                 g
    //             )
    //         };
    //         for (const E& e : path) {
    //             const auto& offsets { g.edge_to_element_offsets(e) };
    //             const auto& [n1, n2] { e.inner_edge };
    //             if (!offsets.has_value()) {
    //                 std::cout << "FREERIDE   "
    //                         << n1.first << '/' << n1.second << "->" << n2.first << '/' << n2.second << '(' << (int)e.type << ") " << std::endl;
    //                 continue;
    //             }
    //             const auto& [idx1, idx2] { *offsets };
    //             std::cout << (idx1.has_value() ? seq1[*idx1] : '-') << " vs " << (idx2.has_value() ? seq2[*idx2] : '-') << "   "
    //                     << n1.first << '/' << n1.second << "->" << n2.first << '/' << n2.second << '(' << (int)e.type << ") " << std::endl;
    //         }
    //         std::cout << weight << std::endl;
    //     }
    //     // walk
    //     {
    //         backtracker<decltype(g)> _backtracker{ g };
    //         const auto& [path, weight] {
    //             _backtracker.find_max_path(
    //                 g
    //             )
    //         };
    //         for (const E& e : path) {
    //             const auto& offsets { g.edge_to_element_offsets(e) };
    //             const auto& [n1, n2] { e.inner_edge };
    //             if (!offsets.has_value()) {
    //                 std::cout << "FREERIDE   "
    //                         << n1.first << '/' << n1.second << "->" << n2.first << '/' << n2.second << '(' << (int)e.type << ") " << std::endl;
    //                 continue;
    //             }
    //             const auto& [idx1, idx2] { *offsets };
    //             std::cout << (idx1.has_value() ? seq1[*idx1] : '-') << " vs " << (idx2.has_value() ? seq2[*idx2] : '-') << "   "
    //                     << n1.first << '/' << n1.second << "->" << n2.first << '/' << n2.second << '(' << (int)e.type << ") " << std::endl;
    //         }
    //         std::cout << std::endl;
    //         std::cout << weight << std::endl;
    //     }

        // std::vector<E> option1 {
        //     E { N {0zu, 0zu}, N {1zu, 1zu} },
        //     E { N {1zu, 1zu}, N {2zu, 1zu} },
        //     E { N {2zu, 1zu}, N {2zu, 2zu} },
        //     E { N {2zu, 2zu}, N {3zu, 3zu} }
        // };
        // std::vector<E> option2 {
        //     E { N {0zu, 0zu}, N {1zu, 1zu} },
        //     E { N {1zu, 1zu}, N {1zu, 2zu} },
        //     E { N {1zu, 2zu}, N {2zu, 2zu} },
        //     E { N {2zu, 2zu}, N {3zu, 3zu} }
        // };
        // EXPECT_TRUE(path == option1 || path == option2);

        // EXPECT_EQ(
        //     path,
        //     (std::vector<E> {
        //         { std::pair{0zu, 0zu}, std::pair{1zu, 1zu} },
        //         { std::pair{1zu, 1zu}, std::pair{2zu, 1zu} },
        //         { std::pair{2zu, 1zu}, std::pair{2zu, 2zu} },
        //         { std::pair{2zu, 2zu}, std::pair{3zu, 3zu} }
        //     })
        // );
    // }
    //
    //
    //
    // TEST(SliceablePairwiseAlignmentGraphBacktrackTest, PathSegmentAlignmentMatch) {
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
    //
    //     std::string seq1 { "aaahellozzzfellowzzz" };
    //     std::string seq2 { "mellowdddtrellow" };
    //     pairwise_global_alignment_graph<decltype(seq1), decltype(seq2)> g {
    //         seq1,
    //         seq2,
    //         match_lookup,
    //         indel_lookup
    //     };
    //
    //     using E = decltype(g)::E;
    //     using ED = decltype(g)::ED;
    //
    //     backtracker<decltype(g)> _backtracker{g};
    //     const auto& [path, weight] {
    //         _backtracker.find_max_path(g)
    //     };
    //     for (const E& e : path) {
    //         const auto& offsets { g.edge_to_element_offsets(e) };
    //         if (!offsets.has_value()) {
    //             continue;
    //         }
    //         const auto& [idx1, idx2] { *offsets };
    //         std::cout << (idx1.has_value() ? seq1[*idx1] : '-') << " vs " << (idx2.has_value() ? seq2[*idx2] : '-') << std::endl;
    //     }
    //     std::cout << weight << std::endl;
    // }
}
