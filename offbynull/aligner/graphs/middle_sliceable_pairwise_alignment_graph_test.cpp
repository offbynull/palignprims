#include <cstddef>
#include <algorithm>
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_local_alignment_graph.h"
#include "offbynull/aligner/graphs/middle_sliceable_pairwise_alignment_graph.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::edge;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::edge_type;
    using offbynull::aligner::graphs::middle_sliceable_pairwise_alignment_graph::middle_sliceable_pairwise_alignment_graph;

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
    auto freeride_lookup {
        [](
            const auto& edge
        ) -> std::float64_t {
            return 0.0f64;
        }
    };

    struct graph_bundle {
        std::string down_seq;
        std::string right_seq;
        pairwise_local_alignment_graph<std::string, std::string> backing_g;
        middle_sliceable_pairwise_alignment_graph<decltype(backing_g)> middle_g;

        graph_bundle(
            std::string _down_seq,
            std::string _right_seq,
            unsigned int down_offset,
            unsigned int right_offset,
            unsigned int down_cnt,
            unsigned int right_cnt
        )
        : down_seq{_down_seq}
        , right_seq{_right_seq}
        , backing_g{
            down_seq,
            right_seq,
            match_lookup,
            indel_lookup,
            freeride_lookup
        }
        , middle_g{backing_g, down_offset, right_offset, down_cnt, right_cnt} {}
    };

    TEST(MiddlePairwiseAlignmentGraphTest, ConceptCheck) {
        using G = decltype(graph_bundle{"abc", "acc", 1zu, 1zu, 2zu, 2zu}.middle_g);
        static_assert(offbynull::aligner::graph::graph::readable_graph<G>);
        static_assert(offbynull::aligner::graph::pairwise_alignment_graph::readable_parwise_alignment_graph<G>);
    }

    TEST(MiddlePairwiseAlignmentGraphTest, ListNodes) {
        graph_bundle g_bundle { "abc", "acc", 1zu, 1zu, 2zu, 2zu };
        auto g { g_bundle.middle_g };

        auto n = g.get_nodes();
        EXPECT_EQ(
            std::set(n.begin(), n.end()),
            (std::set {
                std::pair{1zu, 1zu}, std::pair{1zu, 2zu},
                std::pair{2zu, 1zu}, std::pair{2zu, 2zu},
            })
        );
    }

    TEST(MiddlePairwiseAlignmentGraphTest, ListEdges) {
        graph_bundle g_bundle { "abc", "acc", 1zu, 1zu, 2zu, 2zu };
        auto g { g_bundle.middle_g };

        using E = typename std::remove_reference_t<decltype(g)>::E;

        auto e = g.get_edges();
        std::multiset<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
        for (auto _e : e) {
            actual.insert(_e);
        }
        EXPECT_EQ(
            actual,
            (std::multiset<E> {
                edge { edge_type::NORMAL, std::pair { std::pair{1zu, 1zu}, std::pair{1zu, 2zu} } },
                edge { edge_type::NORMAL, std::pair { std::pair{1zu, 1zu}, std::pair{2zu, 2zu} } },
                edge { edge_type::NORMAL, std::pair { std::pair{1zu, 1zu}, std::pair{2zu, 1zu} } },
                edge { edge_type::NORMAL, std::pair { std::pair{1zu, 2zu}, std::pair{2zu, 2zu} } },
                edge { edge_type::NORMAL, std::pair { std::pair{2zu, 1zu}, std::pair{2zu, 2zu} } },
            })
        );
    }

    TEST(MiddlePairwiseAlignmentGraphTest, NodesExist) {
        graph_bundle g_bundle { "abc", "acc", 1zu, 1zu, 2zu, 2zu };
        auto g { g_bundle.middle_g };

        EXPECT_FALSE(g.has_node({0zu, 0zu}));
        EXPECT_FALSE(g.has_node({0zu, 1zu}));
        EXPECT_FALSE(g.has_node({0zu, 2zu}));
        EXPECT_FALSE(g.has_node({0zu, 3zu}));
        EXPECT_FALSE(g.has_node({1zu, 0zu}));
        EXPECT_TRUE(g.has_node({1zu, 1zu}));
        EXPECT_TRUE(g.has_node({1zu, 2zu}));
        EXPECT_FALSE(g.has_node({1zu, 3zu}));
        EXPECT_FALSE(g.has_node({2zu, 0zu}));
        EXPECT_TRUE(g.has_node({2zu, 1zu}));
        EXPECT_TRUE(g.has_node({2zu, 2zu}));
        EXPECT_FALSE(g.has_node({2zu, 3zu}));
        EXPECT_FALSE(g.has_node({3zu, 0zu}));
        EXPECT_FALSE(g.has_node({3zu, 1zu}));
        EXPECT_FALSE(g.has_node({3zu, 2zu}));
        EXPECT_FALSE(g.has_node({3zu, 3zu}));
    }

    TEST(MiddlePairwiseAlignmentGraphTest, EdgesExist) {
        graph_bundle g_bundle { "abc", "acc", 1zu, 1zu, 2zu, 2zu };
        auto g { g_bundle.middle_g };

        EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{0zu, 0zu}, {0zu, 1zu}}}));
        EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{0zu, 1zu}, {0zu, 2zu}}}));
        EXPECT_TRUE(g.has_edge({ edge_type::NORMAL, { {1zu, 1zu}, {1zu, 2zu} } }));
        EXPECT_TRUE(g.has_edge({ edge_type::NORMAL, { {1zu, 1zu}, {2zu, 2zu} } }));
        EXPECT_TRUE(g.has_edge({ edge_type::NORMAL, { {1zu, 1zu}, {2zu, 1zu} } }));
        EXPECT_TRUE(g.has_edge({ edge_type::NORMAL, { {1zu, 2zu}, {2zu, 2zu} } }));
        EXPECT_TRUE(g.has_edge({ edge_type::NORMAL, { {2zu, 1zu}, {2zu, 2zu} } }));
        EXPECT_FALSE(g.has_edge({edge_type::NORMAL, {{6zu, 7zu}, {6zu, 8zu}}}));
    }

    TEST(MiddlePairwiseAlignmentGraphTest, GetOutputs) {
        graph_bundle g_bundle { "abc", "acc", 1zu, 1zu, 2zu, 2zu };
        auto g { g_bundle.middle_g };

        using E = typename std::remove_reference_t<decltype(g)>::E;

        {
            std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
            for (auto _e : g.get_outputs( { 1zu, 1zu } )) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            std::vector<E> expected {
                { edge_type::NORMAL, {{1zu, 1zu}, {1zu, 2zu} } },
                { edge_type::NORMAL, {{1zu, 1zu}, {2zu, 1zu} } },
                { edge_type::NORMAL, {{1zu, 1zu}, {2zu, 2zu} } }
            };
            EXPECT_EQ(actual, expected);
        }
        {
            std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
            for (auto _e : g.get_outputs( { 2zu, 2zu } )) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            EXPECT_EQ(
                actual,
                (std::vector<E> {})
            );
        }
        {
            std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
            for (auto _e : g.get_outputs( { 1zu, 2zu } )) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            EXPECT_EQ(
                actual,
                (std::vector<E> {
                    { edge_type::NORMAL, { {1zu, 2zu}, {2zu, 2zu} } }
                })
            );
        }
    }

    TEST(MiddlePairwiseAlignmentGraphTest, GetInputs) {
        graph_bundle g_bundle { "abc", "acc", 1zu, 1zu, 2zu, 2zu };
        auto g { g_bundle.middle_g };

        using E = typename std::remove_reference_t<decltype(g)>::E;

        {
            std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
            for (auto _e : g.get_inputs( std::pair{ 1zu, 1zu } )) {
                actual.push_back(_e);
            }
            EXPECT_EQ(
                actual,
                (std::vector<E> {})
            );
        }
        {
            std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
            for (auto _e : g.get_inputs( { 2zu, 2zu } )) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            EXPECT_EQ(
                actual,
                (std::vector<E> {
                    { edge_type::NORMAL, {{1zu, 1zu}, {2zu, 2zu} } },
                    { edge_type::NORMAL, {{1zu, 2zu}, {2zu, 2zu} } },
                    { edge_type::NORMAL, {{2zu, 1zu}, {2zu, 2zu} } }
                })
            );
        }
        {
            std::vector<E> actual {}; // TODO: Can't pass being() and end() to constructor to automate this? Doesn't like end() with sentinel type
            for (auto _e : g.get_inputs( std::pair{1zu, 2zu} )) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            EXPECT_EQ(
                actual,
                (std::vector<E> {
                    { edge_type::NORMAL, { {1zu, 1zu}, {1zu, 2zu} } }
                })
            );
        }
    }

    TEST(MiddlePairwiseAlignmentGraphTest, GetOutputDegree) {
        graph_bundle g_bundle { "abc", "acc", 1zu, 1zu, 2zu, 2zu };
        auto g { g_bundle.middle_g };

        EXPECT_EQ(g.get_out_degree(std::pair{ 1zu, 1zu } ), 3);
        EXPECT_EQ(g.get_out_degree(std::pair{ 1zu, 2zu } ), 1);
        EXPECT_EQ(g.get_out_degree(std::pair{ 2zu, 1zu } ), 1);
        EXPECT_EQ(g.get_out_degree(std::pair{ 2zu, 2zu } ), 0);
    }

    TEST(MiddlePairwiseAlignmentGraphTest, GetInputDegree) {
        graph_bundle g_bundle { "abc", "acc", 1zu, 1zu, 2zu, 2zu };
        auto g { g_bundle.middle_g };

        EXPECT_EQ(g.get_in_degree(std::pair{ 1zu, 1zu } ), 0);
        EXPECT_EQ(g.get_in_degree(std::pair{ 1zu, 2zu } ), 1);
        EXPECT_EQ(g.get_in_degree(std::pair{ 2zu, 1zu } ), 1);
        EXPECT_EQ(g.get_in_degree(std::pair{ 2zu, 2zu } ), 3);
    }

    TEST(MiddlePairwiseAlignmentGraphTest, SlicedWalk) {
        auto to_vector {
            [](auto &&r) {
                auto it { r.begin() };
                std::vector<std::decay_t<decltype(*it)>> ret {};
                while (it != r.end()) {
                    ret.push_back(*it);
                    ++it;
                }
                return ret;
            }
        };

        graph_bundle g_bundle { "abc", "acc", 1zu, 1zu, 2zu, 2zu };
        auto g { g_bundle.middle_g };

        using G = std::decay_t<decltype(g)>;
        using N = typename G::N;
        using E = typename G::E;

        EXPECT_EQ(G::slice_nodes_capacity(g.grid_down_cnt, g.grid_right_cnt), 2zu);
        EXPECT_EQ(g.slice_first_node(0zu), (N { 1zu, 1zu }));
        EXPECT_EQ(g.slice_last_node(0zu), (N { 1zu, 2zu }));
        EXPECT_EQ(g.slice_first_node(1zu), (N { 2zu, 1zu }));
        EXPECT_EQ(g.slice_last_node(1zu), (N { 2zu, 2zu }));

        EXPECT_EQ(G::resident_nodes_capacity(g.grid_down_cnt, g.grid_right_cnt), 2zu); // directly proxied from backing graph
        auto resident_nodes { g.resident_nodes() };
        EXPECT_EQ(std::distance(resident_nodes.begin(), resident_nodes.end()), 0zu);
        auto resident_nodes_it { resident_nodes.begin() };
        EXPECT_EQ(resident_nodes_it, resident_nodes.end());

        EXPECT_EQ(g.slice_next_node(N { 1zu, 1zu }), (N { 1zu, 2zu }));
        EXPECT_EQ(g.slice_next_node(N { 2zu, 1zu }), (N { 2zu, 2zu }));

        EXPECT_EQ(g.slice_prev_node(N { 2zu, 2zu }), (N { 2zu, 1zu }));
        EXPECT_EQ(g.slice_prev_node(N { 1zu, 2zu }), (N { 1zu, 1zu }));

        EXPECT_EQ(
            to_vector(g.outputs_to_residents(N { 1zu, 1zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            to_vector(g.outputs_to_residents(N { 1zu, 2zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            to_vector(g.outputs_to_residents(N { 2zu, 1zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            to_vector(g.outputs_to_residents(N { 2zu, 2zu })),
            (std::vector<E> {})
        );

        EXPECT_EQ(
            to_vector(g.inputs_from_residents(N { 2zu, 2zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            to_vector(g.inputs_from_residents(N { 2zu, 1zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            to_vector(g.inputs_from_residents(N { 1zu, 2zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            to_vector(g.inputs_from_residents(N { 1zu, 1zu })),
            (std::vector<E> {})
        );
    }
}