#include <cstddef>
#include <string>
#include <set>
#include <vector>
#include <ranges>
#include <stdfloat>
#include <iostream>
#include <ostream>
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_local_alignment_graph.h"
#include "offbynull/aligner/graphs/suffix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::edge;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::edge_type;
    using offbynull::aligner::graphs::suffix_sliceable_pairwise_alignment_graph::suffix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;
    using offbynull::utils::copy_to_multiset;
    using offbynull::utils::copy_to_set;
    using offbynull::utils::copy_to_vector;

    auto substitution_scorer { simple_scorer<true, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64) };
    auto gap_scorer { simple_scorer<true, char, char, std::float64_t>::create_gap(0.0f64) };
    auto freeride_scorer { simple_scorer<true, char, char, std::float64_t>::create_freeride() };

    struct graph_bundle {
        std::string down_seq;
        std::string right_seq;
        pairwise_local_alignment_graph<
            true,
            std::size_t,
            std::float64_t,
            std::string,
            std::string,
            decltype(substitution_scorer),
            decltype(gap_scorer),
            decltype(freeride_scorer)
        > backing_g;
        suffix_sliceable_pairwise_alignment_graph<true, decltype(backing_g)> suffix_g;

        graph_bundle(
            std::string down_seq_,
            std::string right_seq_,
            decltype(backing_g)::N new_root_node
        )
        : down_seq { down_seq_ }
        , right_seq { right_seq_ }
        , backing_g {
            down_seq,
            right_seq,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        }
        , suffix_g { backing_g, new_root_node } {}
    };

    TEST(OAGSuffixSliceablePairwiseAlignmentGraphTest, ConceptCheck) {
        using G = decltype(graph_bundle { "234567", "2345678", { 5zu, 5zu }  }.suffix_g);
        static_assert(offbynull::aligner::graph::graph::readable_graph<G>);
        static_assert(offbynull::aligner::graph::pairwise_alignment_graph::readable_pairwise_alignment_graph<G>);
    }

    TEST(OAGSuffixSliceablePairwiseAlignmentGraphTest, ListNodes) {
        graph_bundle g_bundle { "234567", "2345678", { 5zu, 5zu }  };
        auto g { g_bundle.suffix_g };

        using N = typename decltype(g)::N;

        auto n = g.get_nodes();
        EXPECT_EQ(
            std::set(n.begin(), n.end()),
            (std::set<N> {
                N { 5zu, 5zu }, N { 5zu, 6zu }, N { 5zu, 7zu },
                N { 6zu, 5zu }, N { 6zu, 6zu }, N { 6zu, 7zu }
            })
        );
    }

    TEST(OAGSuffixSliceablePairwiseAlignmentGraphTest, ListEdges) {
        graph_bundle g_bundle { "234567", "2345678", { 5zu, 5zu }  };
        auto g { g_bundle.suffix_g };

        using E = typename decltype(g)::E;

        EXPECT_EQ(
            copy_to_multiset(g.get_edges()),
            (std::multiset<E> {
                // culled from backing graph
                // E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 0zu, 1zu } } },
                // E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 0zu, 2zu } } },
                // E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 0zu } } },
                // E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 1zu } } },
                // E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 1zu, 2zu } } },

                E { edge_type::NORMAL, { { 5zu, 5zu }, { 5zu, 6zu } } },
                E { edge_type::NORMAL, { { 5zu, 6zu }, { 5zu, 7zu } } },
                E { edge_type::NORMAL, { { 6zu, 5zu }, { 6zu, 6zu } } },
                E { edge_type::NORMAL, { { 6zu, 6zu }, { 6zu, 7zu } } },
                E { edge_type::NORMAL, { { 5zu, 5zu }, { 6zu, 5zu } } },
                E { edge_type::NORMAL, { { 5zu, 6zu }, { 6zu, 6zu } } },
                E { edge_type::NORMAL, { { 5zu, 7zu }, { 6zu, 7zu } } },
                E { edge_type::NORMAL, { { 5zu, 5zu }, { 6zu, 6zu } } },
                E { edge_type::NORMAL, { { 5zu, 6zu }, { 6zu, 7zu } } },

                E { edge_type::FREE_RIDE, { { 5zu, 5zu }, { 6zu, 7zu } } },
                E { edge_type::FREE_RIDE, { { 5zu, 6zu }, { 6zu, 7zu } } },
                E { edge_type::FREE_RIDE, { { 5zu, 7zu }, { 6zu, 7zu } } },
                E { edge_type::FREE_RIDE, { { 6zu, 5zu }, { 6zu, 7zu } } },
                E { edge_type::FREE_RIDE, { { 6zu, 6zu }, { 6zu, 7zu } } }
            })
        );
    }

    TEST(OAGSuffixSliceablePairwiseAlignmentGraphTest, NodesExist) {
        graph_bundle g_bundle { "234567", "2345678", { 5zu, 5zu }  };
        auto g { g_bundle.suffix_g };

        using N = typename decltype(g)::N;
        
        EXPECT_FALSE(g.has_node(N { 0zu, 0zu }));
        EXPECT_FALSE(g.has_node(N { 0zu, 1zu }));

        EXPECT_TRUE(g.has_node(N { 5zu, 5zu }));
        EXPECT_TRUE(g.has_node(N { 5zu, 6zu }));
        EXPECT_TRUE(g.has_node(N { 5zu, 7zu }));
        EXPECT_FALSE(g.has_node(N { 5zu, 8zu }));
        EXPECT_TRUE(g.has_node(N { 6zu, 5zu }));
        EXPECT_TRUE(g.has_node(N { 6zu, 6zu }));
        EXPECT_TRUE(g.has_node(N { 6zu, 7zu }));
        EXPECT_FALSE(g.has_node(N { 6zu, 8zu }));
        EXPECT_FALSE(g.has_node(N { 7zu, 8zu }));
    }

    TEST(OAGSuffixSliceablePairwiseAlignmentGraphTest, RightEdgesExist) {
        graph_bundle g_bundle { "234567", "2345678", { 5zu, 5zu }  };
        auto g { g_bundle.suffix_g };

        using E = typename decltype(g)::E;
        
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 0zu }, { 0zu, 1zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 1zu }, { 0zu, 2zu } } }));

        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 5zu, 5zu }, { 5zu, 6zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 5zu, 6zu }, { 5zu, 7zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 5zu, 7zu }, { 5zu, 8zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 6zu, 5zu }, { 6zu, 6zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 6zu, 6zu }, { 6zu, 7zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 6zu, 7zu }, { 6zu, 8zu } } }));
    }

    TEST(OAGSuffixSliceablePairwiseAlignmentGraphTest, DownEdgesExist) {
        graph_bundle g_bundle { "234567", "2345678", { 5zu, 5zu }  };
        auto g { g_bundle.suffix_g };

        using E = typename decltype(g)::E;
        
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 0zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 1zu }, { 1zu, 1zu } } }));

        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 5zu, 5zu }, { 6zu, 5zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 6zu, 5zu }, { 7zu, 5zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 5zu, 6zu }, { 6zu, 6zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 6zu, 6zu }, { 7zu, 6zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 5zu, 7zu }, { 6zu, 7zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 6zu, 7zu }, { 7zu, 7zu } } }));
    }

    TEST(OAGSuffixSliceablePairwiseAlignmentGraphTest, DiagEdgesExist) {
        graph_bundle g_bundle { "234567", "2345678", { 5zu, 5zu }  };
        auto g { g_bundle.suffix_g };

        using E = typename decltype(g)::E;
        
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 0zu, 0zu }, { 1zu, 1zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 1zu, 0zu }, { 2zu, 1zu } } }));

        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 5zu, 5zu }, { 6zu, 6zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 6zu, 5zu }, { 7zu, 6zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::NORMAL, { { 5zu, 6zu }, { 6zu, 7zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 6zu, 6zu }, { 7zu, 7zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 5zu, 7zu }, { 6zu, 8zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::NORMAL, { { 6zu, 7zu }, { 7zu, 38 } } }));
    }

    TEST(OAGSuffixSliceablePairwiseAlignmentGraphTest, FreeRideEdgesExist) {
        graph_bundle g_bundle { "234567", "2345678", { 5zu, 5zu }  };
        auto g { g_bundle.suffix_g };

        using E = typename decltype(g)::E;
        
        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 5zu, 5zu }, { 5zu, 5zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 5zu, 5zu }, { 5zu, 6zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 5zu, 5zu }, { 5zu, 7zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 5zu, 5zu }, { 6zu, 5zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 5zu, 5zu }, { 6zu, 6zu } } }));

        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 0zu, 0zu }, { 6zu, 7zu } } }));

        EXPECT_TRUE(g.has_edge(E { edge_type::FREE_RIDE, { { 5zu, 5zu }, { 6zu, 7zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::FREE_RIDE, { { 5zu, 6zu }, { 6zu, 7zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::FREE_RIDE, { { 5zu, 7zu }, { 6zu, 7zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::FREE_RIDE, { { 6zu, 5zu }, { 6zu, 7zu } } }));
        EXPECT_TRUE(g.has_edge(E { edge_type::FREE_RIDE, { { 6zu, 6zu }, { 6zu, 7zu } } }));
        EXPECT_FALSE(g.has_edge(E { edge_type::FREE_RIDE, { { 6zu, 7zu }, { 6zu, 7zu } } }));
    }

    TEST(OAGSuffixSliceablePairwiseAlignmentGraphTest, GetOutputs) {
        graph_bundle g_bundle { "234567", "2345678", { 5zu, 5zu }  };
        auto g { g_bundle.suffix_g };
        
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_EQ(
            copy_to_set(g.get_outputs(N { 5zu, 5zu })),
            (std::set<E> {
                E { edge_type::FREE_RIDE, { { 5zu, 5zu }, { 6zu, 7zu } } },
                E { edge_type::NORMAL, { { 5zu, 5zu }, { 5zu, 6zu } } },
                E { edge_type::NORMAL, { { 5zu, 5zu }, { 6zu, 5zu } } },
                E { edge_type::NORMAL, { { 5zu, 5zu }, { 6zu, 6zu } } }
            })
        );
        EXPECT_EQ(
            copy_to_set(g.get_outputs(N { 6zu, 7zu })),
            (std::set<E> {})
        );
        EXPECT_EQ(
            copy_to_set(g.get_outputs(N { 5zu, 6zu })),
            (std::set<E> {
                E { edge_type::FREE_RIDE, { { 5zu, 6zu }, { 6zu, 7zu } } },
                E { edge_type::NORMAL, { { 5zu, 6zu }, { 5zu, 7zu } } },
                E { edge_type::NORMAL, { { 5zu, 6zu }, { 6zu, 6zu } } },
                E { edge_type::NORMAL, { { 5zu, 6zu }, { 6zu, 7zu } } }
            })
        );
    }

    TEST(OAGSuffixSliceablePairwiseAlignmentGraphTest, GetInputs) {
        graph_bundle g_bundle { "234567", "2345678", { 5zu, 5zu }  };
        auto g { g_bundle.suffix_g };
        
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_EQ(
            copy_to_set(g.get_inputs(N { 5zu, 5zu })),
            (std::set<E> {})
        );
        EXPECT_EQ(
            copy_to_set(g.get_inputs(N { 6zu, 7zu })),
            (std::set<E> {
                E { edge_type::FREE_RIDE, { { 5zu, 5zu }, { 6zu, 7zu } } },
                E { edge_type::FREE_RIDE, { { 5zu, 6zu }, { 6zu, 7zu } } },
                E { edge_type::FREE_RIDE, { { 5zu, 7zu }, { 6zu, 7zu } } },
                E { edge_type::FREE_RIDE, { { 6zu, 5zu }, { 6zu, 7zu } } },
                E { edge_type::FREE_RIDE, { { 6zu, 6zu }, { 6zu, 7zu } } },
                E { edge_type::NORMAL, { { 5zu, 6zu }, { 6zu, 7zu } } },
                E { edge_type::NORMAL, { { 5zu, 7zu }, { 6zu, 7zu } } },
                E { edge_type::NORMAL, { { 6zu, 6zu }, { 6zu, 7zu } } }
            })
        );
        EXPECT_EQ(
            copy_to_set(g.get_inputs(N { 5zu, 7zu })),
            (std::set<E> {
                E { edge_type::NORMAL, { { 5zu, 6zu }, { 5zu, 7zu } } }
            })
        );
    }

    TEST(OAGSuffixSliceablePairwiseAlignmentGraphTest, GetOutputDegree) {
        graph_bundle g_bundle { "234567", "2345678", { 5zu, 5zu }  };
        auto g { g_bundle.suffix_g };

        using N = typename decltype(g)::N;
        
        EXPECT_EQ(g.get_out_degree(N { 5zu, 5zu } ), 4);
        EXPECT_EQ(g.get_out_degree(N { 6zu, 7zu } ), 0);
        EXPECT_EQ(g.get_out_degree(N { 5zu, 7zu } ), 2);
        EXPECT_EQ(g.get_out_degree(N { 6zu, 5zu } ), 2);
    }

    TEST(OAGSuffixSliceablePairwiseAlignmentGraphTest, GetInputDegree) {
        graph_bundle g_bundle { "234567", "2345678", { 5zu, 5zu }  };
        auto g { g_bundle.suffix_g };

        using N = typename decltype(g)::N;
        
        EXPECT_EQ(g.get_in_degree(N { 5zu, 5zu } ), 0);
        EXPECT_EQ(g.get_in_degree(N { 6zu, 7zu } ), 8);
        EXPECT_EQ(g.get_in_degree(N { 5zu, 7zu } ), 1);
        EXPECT_EQ(g.get_in_degree(N { 6zu, 5zu } ), 1);
    }

    TEST(OAGSuffixSliceablePairwiseAlignmentGraphTest, SlicedWalk) {
        graph_bundle g_bundle { "234567", "2345678", { 5zu, 5zu }  };
        auto g { g_bundle.suffix_g };
        
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        EXPECT_EQ(
            (copy_to_vector(g.slice_nodes(0u))),
            (std::vector<N> {
                N { 5zu, 5zu },
                N { 5zu, 6zu },
                N { 5zu, 7zu }
            })
        );
        EXPECT_EQ(
            (copy_to_vector(g.slice_nodes(1u))),
            (std::vector<N> {
                N { 6zu, 5zu },
                N { 6zu, 6zu },
                N { 6zu, 7zu }
            })
        );

        EXPECT_EQ(g.resident_nodes_capacity, 2zu); // directly proxied from backing graph
        auto resident_nodes { g.resident_nodes() };
        EXPECT_EQ(std::ranges::distance(resident_nodes.begin(), resident_nodes.end()), 1zu);
        auto resident_nodes_it { resident_nodes.begin() };
        EXPECT_EQ(*resident_nodes_it, (N { 6zu, 7zu }));
        ++resident_nodes_it;
        EXPECT_EQ(resident_nodes_it, resident_nodes.end());

        EXPECT_EQ(
            copy_to_vector(g.outputs_to_residents(N { 5zu, 5zu })),
            (std::vector<E> {
                E { edge_type::FREE_RIDE, { { 5zu, 5zu }, { 6zu, 7zu } } }
            })
        );
        EXPECT_EQ(
            copy_to_vector(g.outputs_to_residents(N { 5zu, 6zu })),
            (std::vector<E> {
                 E { edge_type::FREE_RIDE, { { 5zu, 6zu }, { 6zu, 7zu } } },
                 E { edge_type::NORMAL, { { 5zu, 6zu }, { 6zu, 7zu } } }
            })
        );
        EXPECT_EQ(
            copy_to_vector(g.outputs_to_residents(N { 5zu, 7zu })),
            (std::vector<E> {
                 E { edge_type::FREE_RIDE, { { 5zu, 7zu }, { 6zu, 7zu } } },
                 E { edge_type::NORMAL, { { 5zu, 7zu }, { 6zu, 7zu } } }
            })
        );
        EXPECT_EQ(
            copy_to_vector(g.outputs_to_residents(N { 6zu, 5zu })),
            (std::vector<E> {
                 E { edge_type::FREE_RIDE, { { 6zu, 5zu }, { 6zu, 7zu } } }
            })
        );
        EXPECT_EQ(
            copy_to_vector(g.outputs_to_residents(N { 6zu, 6zu })),
            (std::vector<E> {
                 E { edge_type::FREE_RIDE, { { 6zu, 6zu }, { 6zu, 7zu } } },
                 E { edge_type::NORMAL, { { 6zu, 6zu }, { 6zu, 7zu } } }
            })
        );
        EXPECT_EQ(
            copy_to_vector(g.outputs_to_residents(N { 6zu, 7zu })),
            (std::vector<E> {})
        );


        EXPECT_EQ(
            copy_to_vector(g.inputs_from_residents(N { 5zu, 5zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            copy_to_vector(g.inputs_from_residents(N { 5zu, 6zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            copy_to_vector(g.inputs_from_residents(N { 5zu, 7zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            copy_to_vector(g.inputs_from_residents(N { 6zu, 5zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            copy_to_vector(g.inputs_from_residents(N { 6zu, 6zu })),
            (std::vector<E> {})
        );
        EXPECT_EQ(
            copy_to_vector(g.inputs_from_residents(N { 6zu, 7zu })),
            (std::vector<E> {})
        );
    }

    TEST(OAGSuffixSliceablePairwiseAlignmentGraphTest, PrintBasic) {
        graph_bundle g_bundle { "234567", "2345678", { 5zu, 5zu }  };
        auto g { g_bundle.suffix_g };

        std::cout << "res" << std::endl;
        for (auto&& n : g.resident_nodes()) {
            std::cout << n << " ";
        }
        std::cout << std::endl;

        std::cout << "norm0" << std::endl;
        for (auto&& n : g.slice_nodes(0zu)) {
            std::cout << n << " ";
        }
        std::cout << std::endl;

        std::cout << "norm1" << std::endl;
        for (auto&& n : g.slice_nodes(1zu)) {
            std::cout << n << " ";
        }
        std::cout << std::endl;

        std::cout << "root" << std::endl;
        std::cout << g.get_root_node() << " ";
        std::cout << std::endl;

        std::cout << "leaf" << std::endl;
        std::cout << g.get_leaf_node() << " ";
        std::cout << std::endl;
    }
}