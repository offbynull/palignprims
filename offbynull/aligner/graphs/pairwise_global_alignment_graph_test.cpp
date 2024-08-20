#include <cstddef>
#include <limits>
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_global_alignment_graph.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::aligner::graphs::pairwise_global_alignment_graph::pairwise_global_alignment_graph;
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;
    
    auto substitution_scorer { simple_scorer<true, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
    auto gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(0.0f64) };

    TEST(OAGPairwiseGlobalAlignmentGraphTest, ConceptCheck) {
        using G = pairwise_global_alignment_graph<true, std::string, std::string>;
        static_assert(offbynull::aligner::graph::graph::readable_graph<G>);
        static_assert(offbynull::aligner::graph::pairwise_alignment_graph::readable_pairwise_alignment_graph<G>);
    }

    TEST(OAGPairwiseGlobalAlignmentGraphTest, ListNodes) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_global_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };
        
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;
        
        auto n = g.get_nodes();
        EXPECT_EQ(
            std::set(n.begin(), n.end()),
            (std::set {
                N { 0zu, 0zu }, N { 0zu, 1zu }, N { 0zu, 2zu },
                N { 1zu, 0zu }, N { 1zu, 1zu }, N { 1zu, 2zu }
            })
        );
    }

    TEST(OAGPairwiseGlobalAlignmentGraphTest, ListEdges) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_global_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };
        
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        auto e = g.get_edges();
        std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
        for (auto _e : e) {
            actual.insert(_e);
        }
        EXPECT_EQ(
            actual,
            (std::set<E> {
                E { N { 0zu, 0zu }, N { 0zu, 1zu } },
                E { N { 0zu, 1zu }, N { 0zu, 2zu } },
                E { N { 1zu, 0zu }, N { 1zu, 1zu } },
                E { N { 1zu, 1zu }, N { 1zu, 2zu } },
                E { N { 0zu, 0zu }, N { 1zu, 0zu } },
                E { N { 0zu, 1zu }, N { 1zu, 1zu } },
                E { N { 0zu, 2zu }, N { 1zu, 2zu } },
                E { N { 0zu, 0zu }, N { 1zu, 1zu } },
                E { N { 0zu, 1zu }, N { 1zu, 2zu } }
            })
        );
    }

    TEST(OAGPairwiseGlobalAlignmentGraphTest, NodesExist) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_global_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };
        
        EXPECT_TRUE(g.has_node({ 0zu, 0zu }));
        EXPECT_TRUE(g.has_node({ 0zu, 1zu }));
        EXPECT_TRUE(g.has_node({ 0zu, 2zu }));
        EXPECT_FALSE(g.has_node({ 0zu, 3zu }));
        EXPECT_TRUE(g.has_node({ 1zu, 0zu }));
        EXPECT_TRUE(g.has_node({ 1zu, 1zu }));
        EXPECT_TRUE(g.has_node({ 1zu, 2zu }));
        EXPECT_FALSE(g.has_node({ 1zu, 3zu }));
        EXPECT_FALSE(g.has_node({ 2zu, 3zu }));
    }

    TEST(OAGPairwiseGlobalAlignmentGraphTest, RightEdgesExist) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_global_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        EXPECT_TRUE(g.has_edge({ { 0zu, 0zu }, { 0zu, 1zu } }));
        EXPECT_TRUE(g.has_edge({ { 0zu, 1zu }, { 0zu, 2zu } }));
        EXPECT_FALSE(g.has_edge({ { 0zu, 2zu }, { 0zu, 3zu } }));
        EXPECT_TRUE(g.has_edge({ { 1zu, 0zu }, { 1zu, 1zu } }));
        EXPECT_TRUE(g.has_edge({ { 1zu, 1zu }, { 1zu, 2zu } }));
        EXPECT_FALSE(g.has_edge({ { 1zu, 2zu }, { 1zu, 3zu } }));
    }

    TEST(OAGPairwiseGlobalAlignmentGraphTest, DownEdgesExist) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_global_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };
        
        EXPECT_TRUE(g.has_edge({ { 0zu, 0zu }, { 1zu, 0zu } }));
        EXPECT_FALSE(g.has_edge({ { 1zu, 0zu }, { 2zu, 0zu } }));
        EXPECT_TRUE(g.has_edge({ { 0zu, 1zu }, { 1zu, 1zu } }));
        EXPECT_FALSE(g.has_edge({ { 1zu, 1zu }, { 2zu, 1zu } }));
        EXPECT_TRUE(g.has_edge({ { 0zu, 2zu }, { 1zu, 2zu } }));
        EXPECT_FALSE(g.has_edge({ { 1zu, 2zu }, { 2zu, 2zu } }));
    }

    TEST(OAGPairwiseGlobalAlignmentGraphTest, DiagEdgesExist) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_global_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };
        
        EXPECT_TRUE(g.has_edge({ { 0zu, 0zu }, { 1zu, 1zu } }));
        EXPECT_FALSE(g.has_edge({ { 1zu, 0zu }, { 2zu, 1zu } }));
        EXPECT_TRUE(g.has_edge({ { 0zu, 1zu }, { 1zu, 2zu } }));
        EXPECT_FALSE(g.has_edge({ { 1zu, 1zu }, { 2zu, 2zu } }));
        EXPECT_FALSE(g.has_edge({ { 0zu, 2zu }, { 1zu, 3zu } }));
        EXPECT_FALSE(g.has_edge({ { 1zu, 2zu }, { 2zu, 3zu } }));
    }

    TEST(OAGPairwiseGlobalAlignmentGraphTest, GetOutputs) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_global_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        {
            std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
            for (auto _e : g.get_outputs( N { 0zu, 0zu } )) {
                actual.insert(_e);
            }
            EXPECT_EQ(
                actual,
                (std::set<E> {
                    E { N { 0zu, 0zu }, N { 0zu, 1zu } },
                    E { N { 0zu, 0zu }, N { 1zu, 0zu } },
                    E { N { 0zu, 0zu }, N { 1zu, 1zu } }
                })
            );
        }
        {
            std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
            for (auto _e : g.get_outputs( N { 1zu, 2zu } )) {
                actual.insert(_e);
            }
            EXPECT_EQ(
                actual,
                (std::set<E> {})
            );
        }
        {
            std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
            for (auto _e : g.get_outputs( N { 0zu, 2zu } )) {
                actual.insert(_e);
            }
            EXPECT_EQ(
                actual,
                (std::set<E> {
                    E { N { 0zu, 2zu }, N { 1zu, 2zu } }
                })
            );
        }
        {
            std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
            for (auto _e : g.get_outputs( N { 1zu, 0zu } )) {
                actual.insert(_e);
            }
            EXPECT_EQ(
                actual,
                (std::set<E> {
                    E { N { 1zu, 0zu }, N { 1zu, 1zu } }
                })
            );
        }
    }

    TEST(OAGPairwiseGlobalAlignmentGraphTest, GetInputs) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_global_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };
        
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        {
            std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
            for (auto _e : g.get_inputs( N { 0zu, 0zu } )) {
                actual.insert(_e);
            }
            EXPECT_EQ(
                actual,
                (std::set<E> {})
            );
        }
        {
            std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
            for (auto _e : g.get_inputs( N { 1zu, 2zu } )) {
                actual.insert(_e);
            }
            EXPECT_EQ(
                actual,
                (std::set<E> {
                    E { N { 0zu, 2zu }, N { 1zu, 2zu } },
                    E { N { 1zu, 1zu }, N { 1zu, 2zu } },
                    E { N { 0zu, 1zu }, N { 1zu, 2zu } }
                })
            );
        }
        {
            std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
            for (auto _e : g.get_inputs( N { 0zu, 2zu } )) {
                actual.insert(_e);
            }
            EXPECT_EQ(
                actual,
                (std::set<E> {
                    E { N { 0zu, 1zu }, N { 0zu, 2zu } }
                })
            );
        }
        {
            std::set<E> actual {}; // TODO: I can't use being() and end() within set's constructor to automate this?
            for (auto _e : g.get_inputs( N { 1zu, 0zu } )) {
                actual.insert(_e);
            }
            EXPECT_EQ(
                actual,
                (std::set<E> {
                    E { N { 0zu, 0zu }, N { 1zu, 0zu } }
                })
            );
        }
    }

    TEST(OAGPairwiseGlobalAlignmentGraphTest, GetOutputDegree) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_global_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        using N = typename decltype(g)::N;

        EXPECT_EQ(g.get_out_degree(N { 0zu, 0zu } ), 3);
        EXPECT_EQ(g.get_out_degree(N { 1zu, 2zu } ), 0);
        EXPECT_EQ(g.get_out_degree(N { 0zu, 2zu } ), 1);
        EXPECT_EQ(g.get_out_degree(N { 1zu, 0zu } ), 1);
    }

    TEST(OAGPairwiseGlobalAlignmentGraphTest, GetInputDegree) {
        std::string seq1 { "a" };
        std::string seq2 { "ac" };
        pairwise_global_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };

        using N = typename decltype(g)::N;
        
        EXPECT_EQ(g.get_in_degree(N { 0zu, 0zu } ), 0);
        EXPECT_EQ(g.get_in_degree(N { 1zu, 2zu } ), 3);
        EXPECT_EQ(g.get_in_degree(N { 0zu, 2zu } ), 1);
        EXPECT_EQ(g.get_in_degree(N { 1zu, 0zu } ), 1);
    }

    TEST(OAGPairwiseGlobalAlignmentGraphTest, SlicedWalk) {
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
        pairwise_global_alignment_graph<true, decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer
        };
        
        using G = std::decay_t<decltype(g)>;
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

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

        EXPECT_EQ(g.resident_nodes_capacity, 0zu);
        EXPECT_EQ(g.resident_nodes().size(), 0zu);

        EXPECT_EQ(
            to_vector(g.outputs_to_residents(N { 0zu, 0zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            to_vector(g.outputs_to_residents(N { 0zu, 1zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            to_vector(g.outputs_to_residents(N { 0zu, 2zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            to_vector(g.outputs_to_residents(N { 1zu, 0zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            to_vector(g.outputs_to_residents(N { 1zu, 1zu })),
            (std::vector<E> {})
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
            (std::vector<E> {})
        );
        EXPECT_EQ(
            to_vector(g.inputs_from_residents(N { 0zu, 2zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            to_vector(g.inputs_from_residents(N { 1zu, 0zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            to_vector(g.inputs_from_residents(N { 1zu, 1zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            to_vector(g.inputs_from_residents(N { 1zu, 2zu })),
            (std::vector<E> {})
        );
    }
}