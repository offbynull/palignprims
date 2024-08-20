#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graphs/pairwise_local_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_global_alignment_graph.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/sliced_subdivider.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "gtest/gtest.h"
#include <stdfloat>

namespace {
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::sliced_subdivider::sliced_subdivider;
    using offbynull::aligner::graphs::pairwise_global_alignment_graph::pairwise_global_alignment_graph;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::edge_type;
    using offbynull::aligner::graphs::middle_sliceable_pairwise_alignment_graph::middle_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;

    TEST(OABSSlicedSubdividerTest, GlobalTest) {
        auto substitution_scorer { simple_scorer<true, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(0.0f64) };
        std::string seq1 { "abc" };
        std::string seq2 { "azc" };
        pairwise_global_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        // walk
        sliced_subdivider<true, decltype(g)> subdivider { g };
        auto path { subdivider.subdivide() };
        auto backward_path_view { path.walk_path_backward() };
        std::vector<E> forward_path(backward_path_view.begin(), backward_path_view.end());
        std::reverse(forward_path.begin(), forward_path.end());
        for (const E& e : forward_path) {
            std::cout << e << ' ';
        }
        std::cout << std::endl;
        // std::cout << weight << std::endl;

        std::vector<E> option1 {
            E { N { 0zu, 0zu }, N { 1zu, 1zu } },
            E { N { 1zu, 1zu }, N { 2zu, 1zu } },
            E { N { 2zu, 1zu }, N { 2zu, 2zu } },
            E { N { 2zu, 2zu }, N { 3zu, 3zu } }
        };
        std::vector<E> option2 {
            E { N { 0zu, 0zu }, N { 1zu, 1zu } },
            E { N { 1zu, 1zu }, N { 1zu, 2zu } },
            E { N { 1zu, 2zu }, N { 2zu, 2zu } },
            E { N { 2zu, 2zu }, N { 3zu, 3zu } }
        };
        EXPECT_TRUE(forward_path == option1 || forward_path == option2);
    }

    TEST(OABSSlicedSubdividerTest, IsolatedLocalTest) {
        auto substitution_scorer { simple_scorer<true, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(0.0f64) };
        auto freeride_scorer { simple_scorer<true, char, char, std::float64_t>::create_freeride(0.0f64) };
        std::string seq1 { "aaaaalmnaaaaa" };
        std::string seq2 { "zzzzzlVnzzzzz" };
        pairwise_local_alignment_graph<true, decltype(seq1), decltype(seq2)> g_ {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };
        using N = typename decltype(g_)::N;
        using E = typename decltype(g_)::E;
        middle_sliceable_pairwise_alignment_graph<true, decltype(g_)> g {
            g_,
            N { 5u, 5u },
            N { 8u, 8u }
        };

        // walk
        sliced_subdivider<true, decltype(g)> subdivider { g };
        auto path { subdivider.subdivide() };
        auto backward_path_view { path.walk_path_backward() };
        std::vector<E> forward_path(backward_path_view.begin(), backward_path_view.end());
        std::reverse(forward_path.begin(), forward_path.end());
        for (const E& e : forward_path) {
            std::cout << e << ' ';
        }
        std::cout << std::endl;
        // std::cout << weight << std::endl;

        std::vector<E> option1 {
            E { edge_type::NORMAL, { N { 5zu, 5zu }, N { 6zu, 6zu } } },
            E { edge_type::NORMAL, { N { 6zu, 6zu }, N { 7zu, 6zu } } },
            E { edge_type::NORMAL, { N { 7zu, 6zu }, N { 7zu, 7zu } } },
            E { edge_type::NORMAL, { N { 7zu, 7zu }, N { 8zu, 8zu } } }
        };
        std::vector<E> option2 {
            E { edge_type::NORMAL, { N { 5zu, 5zu }, N { 6zu, 6zu } } },
            E { edge_type::NORMAL, { N { 6zu, 6zu }, N { 6zu, 7zu } } },
            E { edge_type::NORMAL, { N { 6zu, 7zu }, N { 7zu, 7zu } } },
            E { edge_type::NORMAL, { N { 7zu, 7zu }, N { 8zu, 8zu } } }
        };
        EXPECT_TRUE(forward_path == option1 || forward_path == option2);
    }

    TEST(OABSSlicedSubdividerTest, UnisolatedLocalTest) {
        auto substitution_scorer { simple_scorer<true, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
        auto gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(0.0f64) };
        auto freeride_scorer { simple_scorer<true, char, char, std::float64_t>::create_freeride(0.0f64) };
        std::string seq1 { "aaaaalmnaaaaa" };
        std::string seq2 { "zzzzzlVnzzzzz" };
        pairwise_local_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        // walk
        // EXPECT_THROW((sliced_subdivider<true, decltype(g)> { g }), std::runtime_error);
        // THIS WILL NOT THROW, because subdivider accepts it if root and leaf are both resident nodes (segmenter will
        // segment graph based on resident node edges that get passed through)
    }
}
