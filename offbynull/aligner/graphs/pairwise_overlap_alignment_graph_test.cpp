#include <stdfloat>
#include <string>
#include <set>
#include <vector>
#include <algorithm>
#include <type_traits>
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_overlap_alignment_graph.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::aligner::graphs::pairwise_overlap_alignment_graph::pairwise_overlap_alignment_graph;
    using offbynull::aligner::graphs::pairwise_overlap_alignment_graph::edge;
    using offbynull::aligner::graphs::pairwise_overlap_alignment_graph::edge_type;
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;

    auto substitution_scorer { simple_scorer<true, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
    auto gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(0.0f64) };
    auto freeride_scorer { simple_scorer<true, char, char, std::float64_t>::create_freeride() };

    TEST(OAGPairwiseOverlapAlignmentGraphTest, ConceptCheck) {
        using G = pairwise_overlap_alignment_graph<true, std::string, std::string>;
        static_assert(offbynull::aligner::graph::graph::readable_graph<G>);
        static_assert(offbynull::aligner::graph::pairwise_alignment_graph::readable_pairwise_alignment_graph<G>);
    }

    TEST(OAGPairwiseOverlapAlignmentGraphTest, ListNodes) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_overlap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;

        auto n = g.get_nodes();
        EXPECT_EQ(
            std::set(n.begin(), n.end()),
            (std::set<N> {
                N { 0zu, 0zu }, N { 0zu, 1zu }, N { 0zu, 2zu },
                N { 1zu, 0zu }, N { 1zu, 1zu }, N { 1zu, 2zu }
            })
        );
    }

    TEST(OAGPairwiseOverlapAlignmentGraphTest, ListEdges) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_overlap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };

        using E = typename decltype(g)::E;

        auto e = g.get_edges();
        std::vector<E> actual {}; // TODO: Can't pass being()/end() to constructor to automate this? Fails when end() is sentinel type
        for (auto _e : e) {
            actual.push_back(_e);
        }
        std::sort(actual.begin(), actual.end());
        EXPECT_EQ(
            actual,
            (std::vector<E> {
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 0zu } } },
                E { edge_type::FREE_RIDE, { { 1zu, 0zu }, { 1zu, 2zu } } },
                E { edge_type::FREE_RIDE, { { 1zu, 1zu }, { 1zu, 2zu } } },
                E { edge_type::NORMAL, { { 0zu, 0zu }, { 0zu, 1zu } } },
                E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 0zu } } },
                E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 1zu } } },
                E { edge_type::NORMAL, { { 0zu, 1zu }, { 0zu, 2zu } } },
                E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 1zu } } },
                E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 2zu } } },
                E { edge_type::NORMAL, { { 0zu, 2zu }, { 1zu, 2zu } } },
                E { edge_type::NORMAL, { { 1zu, 0zu }, { 1zu, 1zu } } },
                E { edge_type::NORMAL, { { 1zu, 1zu }, { 1zu, 2zu } } }
            })
        );
    }

    TEST(OAGPairwiseOverlapAlignmentGraphTest, NodesExist) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_overlap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;

        EXPECT_TRUE(g.has_node(N { 0zu, 0zu }));
        EXPECT_TRUE(g.has_node(N { 0zu, 1zu }));
        EXPECT_TRUE(g.has_node(N { 0zu, 2zu }));
        EXPECT_FALSE(g.has_node(N { 0zu, 3zu }));
        EXPECT_TRUE(g.has_node(N { 1zu, 0zu }));
        EXPECT_TRUE(g.has_node(N { 1zu, 1zu }));
        EXPECT_TRUE(g.has_node(N { 1zu, 2zu }));
        EXPECT_FALSE(g.has_node(N { 1zu, 3zu }));
        EXPECT_FALSE(g.has_node(N { 2zu, 3zu }));
    }

    TEST(OAGPairwiseOverlapAlignmentGraphTest, RightEdgesExist) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_overlap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };

        using E = typename decltype(g)::E;

        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 0zu }, { 0zu, 1zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 1zu }, { 0zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 2zu }, { 0zu, 3zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 0zu }, { 1zu, 1zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 1zu }, { 1zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 2zu }, { 1zu, 3zu } } }));
    }

    TEST(OAGPairwiseOverlapAlignmentGraphTest, DownEdgesExist) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_overlap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };

        using E = typename decltype(g)::E;

        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 0zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 0zu }, { 2zu, 0zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 1zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 1zu }, { 2zu, 1zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 2zu }, { 1zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 2zu }, { 2zu, 2zu } } }));
    }

    TEST(OAGPairwiseOverlapAlignmentGraphTest, DiagEdgesExist) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_overlap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };

        using E = typename decltype(g)::E;

        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 1zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 0zu }, { 2zu, 1zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 1zu }, { 2zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 2zu }, { 1zu, 3zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 2zu }, { 2zu, 3zu } } }));
    }

    TEST(OAGPairwiseOverlapAlignmentGraphTest, FreeRideEdgesExist) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_overlap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };

        using E = typename decltype(g)::E;

        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 0zu, 0zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 0zu, 1zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 0zu, 2zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 0zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 1zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 2zu } } }));

        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 1zu }, { 1zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 2zu }, { 1zu, 2zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::FREE_RIDE, { { 1zu, 0zu }, { 1zu, 2zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::FREE_RIDE, { { 1zu, 1zu }, { 1zu, 2zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 1zu, 2zu }, { 1zu, 2zu } } }));
    }

    TEST(OAGPairwiseOverlapAlignmentGraphTest, GetOutputs) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_overlap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };

        using E = typename decltype(g)::E;

        {
            std::vector<E> actual {}; // TODO: Can't pass being()/end() to constructor to automate this? Fails when end() is sentinel type
            for (auto _e : g.get_outputs( { 0zu, 0zu } )) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            EXPECT_EQ(
                actual,
                (std::vector<E> {
                    E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 0zu } } },
                    E { edge_type::NORMAL, { { 0zu, 0zu }, { 0zu, 1zu } } },
                    E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 0zu } } },
                    E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 1zu } } }
                })
            );
        }
        {
            std::vector<E> actual {}; // TODO: Can't pass being()/end() to constructor to automate this? Fails when end() is sentinel type
            for (auto _e : g.get_outputs( { 1zu, 2zu } )) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            EXPECT_EQ(
                actual,
                (std::vector<E> {})
            );
        }
        {
            std::vector<E> actual {}; // TODO: Can't pass being()/end() to constructor to automate this? Fails when end() is sentinel type
            for (auto _e : g.get_outputs( { 0zu, 2zu } )) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            EXPECT_EQ(
                actual,
                (std::vector<E> {
                    E { edge_type::NORMAL, { { 0zu, 2zu }, { 1zu, 2zu } } }
                })
            );
        }
        {
            std::vector<E> actual {}; // TODO: Can't pass being()/end() to constructor to automate this? Fails when end() is sentinel type
            for (auto _e : g.get_outputs( { 1zu, 0zu } )) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            EXPECT_EQ(
                actual,
                (std::vector<E> {
                    E { edge_type::FREE_RIDE, { { 1zu, 0zu }, { 1zu, 2zu } } },
                    E { edge_type::NORMAL, { { 1zu, 0zu }, { 1zu, 1zu } } },
                })
            );
        }
    }

    TEST(OAGPairwiseOverlapAlignmentGraphTest, GetInputs) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_overlap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };
        
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        {
            std::vector<E> actual {}; // TODO: Can't pass being()/end() to constructor to automate this? Fails when end() is sentinel type
            for (auto _e : g.get_inputs(N { 0zu, 0zu })) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            EXPECT_EQ(
                actual,
                (std::vector<E> {})
            );
        }
        {
            std::vector<E> actual {}; // TODO: Can't pass being()/end() to constructor to automate this? Fails when end() is sentinel type
            for (auto _e : g.get_inputs(N { 1zu, 2zu })) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            EXPECT_EQ(
                actual,
                (std::vector<E> {
                    E { edge_type::FREE_RIDE, { { 1zu, 0zu }, { 1zu, 2zu } } },
                    E { edge_type::FREE_RIDE, { { 1zu, 1zu }, { 1zu, 2zu } } },
                    E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 2zu } } },
                    E { edge_type::NORMAL, { { 0zu, 2zu }, { 1zu, 2zu } } },
                    E { edge_type::NORMAL, { { 1zu, 1zu }, { 1zu, 2zu } } }
                })
            );
        }
        {
            std::vector<E> actual {}; // TODO: Can't pass being()/end() to constructor to automate this? Fails when end() is sentinel type
            for (auto _e : g.get_inputs(N { 0zu, 2zu })) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            EXPECT_EQ(
                actual,
                (std::vector<E> {
                    E { edge_type::NORMAL, { { 0zu, 1zu }, { 0zu, 2zu } } }
                })
            );
        }
        {
            std::vector<E> actual {}; // TODO: Can't pass being()/end() to constructor to automate this? Fails when end() is sentinel type
            for (auto _e : g.get_inputs(N { 1zu, 0zu })) {
                actual.push_back(_e);
            }
            std::sort(actual.begin(), actual.end());
            EXPECT_EQ(
                actual,
                (std::vector<E> {
                    E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 0zu } } },
                    E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 0zu } } }
                })
            );
        }
    }

    TEST(OAGPairwiseOverlapAlignmentGraphTest, GetOutputDegree) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_overlap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;
        
        EXPECT_EQ(g.get_out_degree(N { 0zu, 0zu }), 4);
        EXPECT_EQ(g.get_out_degree(N { 1zu, 2zu }), 0);
        EXPECT_EQ(g.get_out_degree(N { 0zu, 2zu }), 1);
        EXPECT_EQ(g.get_out_degree(N { 1zu, 0zu }), 2);
    }

    TEST(OAGPairwiseOverlapAlignmentGraphTest, GetInputDegree) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_overlap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;

        EXPECT_EQ(g.get_in_degree(N { 0zu, 0zu }), 0);
        EXPECT_EQ(g.get_in_degree(N { 1zu, 2zu }), 5);
        EXPECT_EQ(g.get_in_degree(N { 0zu, 2zu }), 1);
        EXPECT_EQ(g.get_in_degree(N { 1zu, 0zu }), 2);
    }

    TEST(OAGPairwiseOverlapAlignmentGraphTest, SlicedWalk) {
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

        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_overlap_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };
    
        using G = std::decay_t<decltype(g)>;
        using N = typename G::N;
        using E = typename G::E;

        EXPECT_EQ(
            (to_vector(g.slice_nodes(0u))),
            (std::vector<N> {
                N { 0zu, 0zu },
                N { 0zu, 1zu },
                N { 0zu, 2zu }
            })
        );
        EXPECT_EQ(
            (to_vector(g.slice_nodes(1u))),
            (std::vector<N> {
                N { 1zu, 0zu },
                N { 1zu, 1zu },
                N { 1zu, 2zu }
            })
        );

        EXPECT_EQ(g.resident_nodes_capacity, 2zu);
        EXPECT_EQ(g.resident_nodes().size(), 2zu);
        EXPECT_EQ(g.resident_nodes()[0], (N { 0zu, 0zu }));
        EXPECT_EQ(g.resident_nodes()[1], (N { 1zu, 2zu }));

        EXPECT_EQ(
            to_vector(g.outputs_to_residents(N { 0zu, 0zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            to_vector(g.outputs_to_residents(N { 0zu, 1zu })),
            (std::vector<E> {
                E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 2zu } } }
            })
        );
        EXPECT_EQ(
            to_vector(g.outputs_to_residents(N { 0zu, 2zu })),
            (std::vector<E> {
                E { edge_type::NORMAL, { { 0zu, 2zu }, { 1zu, 2zu } } }
            })
        );
        EXPECT_EQ(
            to_vector(g.outputs_to_residents(N { 1zu, 0zu })),
            (std::vector<E> {
                E { edge_type::FREE_RIDE, { { 1zu, 0zu }, { 1zu, 2zu } } },
            })
        );
        EXPECT_EQ(
            to_vector(g.outputs_to_residents(N { 1zu, 1zu })),
            (std::vector<E> {
                E { edge_type::FREE_RIDE, { { 1zu, 1zu }, { 1zu, 2zu } } },
                E { edge_type::NORMAL, { { 1zu, 1zu }, { 1zu, 2zu } } }
            })
        );
        EXPECT_EQ(
            to_vector(g.outputs_to_residents(N { 1zu, 2zu })),
            (std::vector<E> {})
        );


        EXPECT_EQ(
            to_vector(g.inputs_from_residents(N { 0zu, 0zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            to_vector(g.inputs_from_residents(N { 0zu, 1zu })),
            (std::vector<E> {
                E { edge_type::NORMAL, { { 0zu, 0zu }, { 0zu, 1zu } } }
            })
        );
        EXPECT_EQ(
            to_vector(g.inputs_from_residents(N { 0zu, 2zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            to_vector(g.inputs_from_residents(N { 1zu, 0zu })),
            (std::vector<E> {
                E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 0zu } } },
                E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 0zu } } }
            })
        );
        EXPECT_EQ(
            to_vector(g.inputs_from_residents(N { 1zu, 1zu })),
            (std::vector<E> {
                E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 1zu } } }
            })
        );
        EXPECT_EQ(
            to_vector(g.inputs_from_residents(N { 1zu, 2zu })),
            (std::vector<E> { })
        );
    }
}