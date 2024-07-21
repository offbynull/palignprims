#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graphs/pairwise_fitting_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_overlap_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_local_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_global_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_extended_gap_alignment_graph.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtracker.h"
#include "gtest/gtest.h"
#include <stdfloat>

namespace {
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtracker::backtracker;
    using offbynull::aligner::graphs::pairwise_global_alignment_graph::pairwise_global_alignment_graph;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph;
    using offbynull::aligner::graphs::pairwise_overlap_alignment_graph::pairwise_overlap_alignment_graph;
    using offbynull::aligner::graphs::pairwise_fitting_alignment_graph::pairwise_fitting_alignment_graph;
    using offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::pairwise_extended_gap_alignment_graph;
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

    TEST(SliceablePairwiseAlignmentGraphBacktrackerTest, LocalTest) {
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
                return -1.0f64;
            }
        };
        auto freeride_lookup {
            [](
                const auto& edge
            ) -> std::float64_t {
                return 0.0f64;
            }
        };
        std::string seq1 { "aaaaalmnaaaaa" };
        std::string seq2 { "zzzzzlmnzzzzz" };
        pairwise_local_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            match_lookup,
            indel_lookup,
            freeride_lookup
        };
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        // walk
        backtracker<decltype(g)> backtracker_ { };
        const auto& [path, weight] { backtracker_.find_max_path(g) };
        for (const E& e : path) {
            const auto& [n1, n2] { e.inner_edge };
            std::cout << n1.first << '/' << n1.second << "->" << n2.first << '/' << n2.second << ' ';
        }
        std::cout << std::endl;
        // std::cout << weight << std::endl;

        using offbynull::aligner::graphs::pairwise_local_alignment_graph::edge_type;
        std::vector<E> expected_path {
            E { edge_type::FREE_RIDE, { N {0zu, 0zu}, N {5zu, 5zu} } },
            E { edge_type::NORMAL, { N {5zu, 5zu}, N {6zu, 6zu} } },
            E { edge_type::NORMAL, { N {6zu, 6zu}, N {7zu, 7zu} } },
            E { edge_type::NORMAL, { N {7zu, 7zu}, N {8zu, 8zu} } },
            E { edge_type::FREE_RIDE, { N {8zu, 8zu}, N {13zu, 13zu} } }
        };
        EXPECT_EQ(expected_path, path);
    }

    TEST(SliceablePairwiseAlignmentGraphBacktrackerTest, OverlapTest) {
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
                return -1.0f64;
            }
        };
        auto freeride_lookup {
            [](
                const auto& edge
            ) -> std::float64_t {
                return 0.0f64;
            }
        };
        std::string seq1 { "aaaaalmn" };
        std::string seq2 { "lmnzzzzz" };
        pairwise_overlap_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            match_lookup,
            indel_lookup,
            freeride_lookup
        };
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        // walk
        backtracker<decltype(g)> backtracker_ { };
        const auto& [path, weight] { backtracker_.find_max_path(g) };
        for (const E& e : path) {
            const auto& [n1, n2] { e.inner_edge };
            std::cout << n1.first << '/' << n1.second << "->" << n2.first << '/' << n2.second << ' ';
        }
        std::cout << std::endl;
        // std::cout << weight << std::endl;

        using offbynull::aligner::graphs::pairwise_overlap_alignment_graph::edge_type;
        std::vector<E> expected_path {
            E { edge_type::FREE_RIDE, { N {0zu, 0zu}, N {5zu, 0zu} } },
            E { edge_type::NORMAL, { N {5zu, 0zu}, N {6zu, 1zu} } },
            E { edge_type::NORMAL, { N {6zu, 1zu}, N {7zu, 2zu} } },
            E { edge_type::NORMAL, { N {7zu, 2zu}, N {8zu, 3zu} } },
            E { edge_type::FREE_RIDE, { N {8zu, 3zu}, N {8zu, 8zu} } }
        };
        EXPECT_EQ(expected_path, path);
    }

    TEST(SliceablePairwiseAlignmentGraphBacktrackerTest, FittingTest) {
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
                return -1.0f64;
            }
        };
        auto freeride_lookup {
            [](
                const auto& edge
            ) -> std::float64_t {
                return 0.0f64;
            }
        };
        std::string seq1 { "aaaaalmnaaaaa" };
        std::string seq2 { "lmn" };
        pairwise_fitting_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            match_lookup,
            indel_lookup,
            freeride_lookup
        };
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        // walk
        backtracker<decltype(g)> backtracker_ { };
        const auto& [path, weight] { backtracker_.find_max_path(g) };
        for (const E& e : path) {
            const auto& [n1, n2] { e.inner_edge };
            std::cout << n1.first << '/' << n1.second << "->" << n2.first << '/' << n2.second << ' ';
        }
        std::cout << std::endl;
        // std::cout << weight << std::endl;

        using offbynull::aligner::graphs::pairwise_fitting_alignment_graph::edge_type;
        std::vector<E> expected_path {
            E { edge_type::FREE_RIDE, { N {0zu, 0zu}, N {5zu, 0zu} } },
            E { edge_type::NORMAL, { N {5zu, 0zu}, N {6zu, 1zu} } },
            E { edge_type::NORMAL, { N {6zu, 1zu}, N {7zu, 2zu} } },
            E { edge_type::NORMAL, { N {7zu, 2zu}, N {8zu, 3zu} } },
            E { edge_type::FREE_RIDE, { N {8zu, 3zu}, N {13zu, 3zu} } }
        };
        EXPECT_EQ(expected_path, path);
    }

    TEST(SliceablePairwiseAlignmentGraphBacktrackerTest, ExtendedGapTest) {
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
                return -1.0f64;
            }
        };
        auto extended_indel_lookup {
            [](
                const auto& edge
            ) -> std::float64_t {
                return 0.0f64;
            }
        };
        auto freeride_lookup {
            [](
                const auto& edge
            ) -> std::float64_t {
                return 0.0f64;
            }
        };
        std::string seq1 { "aaalaaamaaanaa" };
        std::string seq2 { "lmn" };
        pairwise_extended_gap_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            match_lookup,
            indel_lookup,
            extended_indel_lookup,
            freeride_lookup
        };
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        // walk
        backtracker<decltype(g)> backtracker_ { };
        const auto& [path, weight] { backtracker_.find_max_path(g) };
        for (const E& e : path) {
            const auto& [n1, n2] { e.inner_edge };
            std::cout << n1.first << '/' << n1.second << "->" << n2.first << '/' << n2.second << ' ';
        }
        std::cout << std::endl;
        // std::cout << weight << std::endl;

        using offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::layer;
        std::vector<E> expected_path {
            E { N {layer::DIAGONAL, 0zu, 0zu}, N {layer::DIAGONAL, 5zu, 0zu} },
        };
        EXPECT_EQ(expected_path, path);
    }
}