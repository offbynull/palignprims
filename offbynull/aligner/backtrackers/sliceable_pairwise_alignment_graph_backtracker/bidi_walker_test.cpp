#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graphs/pairwise_local_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_fitting_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_overlap_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_global_alignment_graph.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/bidi_walker.h"
#include "gtest/gtest.h"
#include <stdfloat>

namespace {
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker::bidi_walker;
    using offbynull::aligner::graphs::pairwise_global_alignment_graph::pairwise_global_alignment_graph;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph;
    using offbynull::aligner::graphs::pairwise_fitting_alignment_graph::pairwise_fitting_alignment_graph;
    using offbynull::aligner::graphs::pairwise_overlap_alignment_graph::pairwise_overlap_alignment_graph;

    TEST(BidiWalkerTest, WalkGlobal) {
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
        std::string seq1 { "abcdefg" };
        std::string seq2 { "abcZefg" };
        pairwise_global_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            match_lookup,
            indel_lookup
        };

        // walk
        bidi_walker<decltype(g)> bidi_walker_ { g };
        const auto& [weight, forward_walk_edge, backward_walk_edge] { bidi_walker_.walk_to_node({3u, 3u}) };
        std::cout << std::endl;
        std::cout << weight << std::endl;
        EXPECT_EQ(weight, 6u);
    }

    TEST(BidiWalkerTest, WalkLocal) {
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
        std::string seq1 { "aaalmaaa" };
        std::string seq2 { "zzzlmzzz" };
        pairwise_local_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            match_lookup,
            indel_lookup,
            freeride_lookup
        };

        // walk
        bidi_walker<decltype(g)> bidi_walker_ { g };
        for (unsigned int down_offset { 0u }; down_offset < g.grid_down_cnt; down_offset++) {
            for (unsigned int right_offset { 0u }; right_offset < g.grid_right_cnt; right_offset++) {
                const auto& [weight, forward_walk_edge, backward_walk_edge] {
                    bidi_walker_.walk_to_node({down_offset, right_offset})
                };
                std::cout << down_offset << ',' << right_offset << '=' << weight;
                if (forward_walk_edge.has_value()) {
                    const auto& [forward_walk_from_node, forward_walk_to_node] { (*forward_walk_edge).inner_edge };
                    const auto& [forward_walk_from_node_down, forward_walk_from_node_right] { forward_walk_from_node };
                    std::cout << '(' << "fh_from" << forward_walk_from_node_down << ',' << forward_walk_from_node_right << ')';
                } else {
                    std::cout << '(' << "fh_from_,_" << ')';
                }
                if (backward_walk_edge.has_value()) {
                    const auto& [backward_walk_from_node, backward_walk_to_node] { (*backward_walk_edge).inner_edge };
                    const auto& [backward_walk_to_node_down, backward_walk_to_node_right] { backward_walk_to_node };
                    std::cout << '(' << "bh_to" << backward_walk_to_node_down << ',' << backward_walk_to_node_right << ')';
                } else {
                    std::cout << '(' << "bh_to_,_" << ')';
                }
                std::cout << '\t';
            }
            std::cout << std::endl;
        }
        // const auto& weight { bidi_walker_.walk_to_node({3u, 3u}) };
        // std::cout << std::endl;
        // std::cout << weight << std::endl;
        // EXPECT_EQ(weight, 6u);
    }

    TEST(BidiWalkerTest, WalkFitting) {
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
        std::string seq1 { "aaalmnaaa" };
        std::string seq2 { "lmn" };
        pairwise_fitting_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            match_lookup,
            indel_lookup,
            freeride_lookup
        };

        // walk
        bidi_walker<decltype(g)> bidi_walker_ { g };
        for (unsigned int down_offset { 0u }; down_offset < g.grid_down_cnt; down_offset++) {
            for (unsigned int right_offset { 0u }; right_offset < g.grid_right_cnt; right_offset++) {
                const auto& [weight, forward_walk_edge, backward_walk_edge] {
                    bidi_walker_.walk_to_node({down_offset, right_offset})
                };
                std::cout << down_offset << ',' << right_offset << '=' << weight;
                if (forward_walk_edge.has_value()) {
                    const auto& [forward_walk_from_node, forward_walk_to_node] { (*forward_walk_edge).inner_edge };
                    const auto& [forward_walk_from_node_down, forward_walk_from_node_right] { forward_walk_from_node };
                    std::cout << '(' << "fh_from" << forward_walk_from_node_down << ',' << forward_walk_from_node_right << ')';
                } else {
                    std::cout << '(' << "fh_from_,_" << ')';
                }
                if (backward_walk_edge.has_value()) {
                    const auto& [backward_walk_from_node, backward_walk_to_node] { (*backward_walk_edge).inner_edge };
                    const auto& [backward_walk_to_node_down, backward_walk_to_node_right] { backward_walk_to_node };
                    std::cout << '(' << "bh_to" << backward_walk_to_node_down << ',' << backward_walk_to_node_right << ')';
                } else {
                    std::cout << '(' << "bh_to_,_" << ')';
                }
                std::cout << '\t';
            }
            std::cout << std::endl;
        }

        // test
        auto final_weight {
            std::get<0>(
                bidi_walker_.walk_to_node(g.get_leaf_node())
            )
        };
        std::cout << "final weight: " << final_weight << std::endl;
        for (const auto& node : g.resident_nodes()) {
            const auto& [down_offset, right_offset] { node };
            const auto& [weight, forward_walk_edge, backward_walk_edge] {
                bidi_walker_.walk_to_node({down_offset, right_offset})
            };
            std::cout << "resident node: " << down_offset << ',' << right_offset << '=' << weight;
            if (forward_walk_edge.has_value()) {
                const auto& [forward_walk_from_node, forward_walk_to_node] { (*forward_walk_edge).inner_edge };
                const auto& [forward_walk_from_node_down, forward_walk_from_node_right] { forward_walk_from_node };
                std::cout << '(' << "fh_from" << forward_walk_from_node_down << ',' << forward_walk_from_node_right << ')';
            } else {
                std::cout << '(' << "fh_from_,_" << ')';
            }
            if (backward_walk_edge.has_value()) {
                const auto& [backward_walk_from_node, backward_walk_to_node] { (*backward_walk_edge).inner_edge };
                const auto& [backward_walk_to_node_down, backward_walk_to_node_right] { backward_walk_to_node };
                std::cout << '(' << "bh_to" << backward_walk_to_node_down << ',' << backward_walk_to_node_right << ')';
            } else {
                std::cout << '(' << "bh_to_,_" << ')';
            }
            std::cout << std::endl;
        }
    }

    TEST(BidiWalkerTest, WalkOverlap) {
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

        // walk
        bidi_walker<decltype(g)> bidi_walker_ { g };
        for (unsigned int down_offset { 0u }; down_offset < g.grid_down_cnt; down_offset++) {
            for (unsigned int right_offset { 0u }; right_offset < g.grid_right_cnt; right_offset++) {
                const auto& [weight, forward_walk_edge, backward_walk_edge] {
                    bidi_walker_.walk_to_node({down_offset, right_offset})
                };
                std::cout << down_offset << ',' << right_offset << '=' << weight;
                if (forward_walk_edge.has_value()) {
                    const auto& [forward_walk_from_node, forward_walk_to_node] { (*forward_walk_edge).inner_edge };
                    const auto& [forward_walk_from_node_down, forward_walk_from_node_right] { forward_walk_from_node };
                    std::cout << '(' << "fh_from" << forward_walk_from_node_down << ',' << forward_walk_from_node_right << ')';
                } else {
                    std::cout << '(' << "fh_from_,_" << ')';
                }
                if (backward_walk_edge.has_value()) {
                    const auto& [backward_walk_from_node, backward_walk_to_node] { (*backward_walk_edge).inner_edge };
                    const auto& [backward_walk_to_node_down, backward_walk_to_node_right] { backward_walk_to_node };
                    std::cout << '(' << "bh_to" << backward_walk_to_node_down << ',' << backward_walk_to_node_right << ')';
                } else {
                    std::cout << '(' << "bh_to_,_" << ')';
                }
                std::cout << '\t';
            }
            std::cout << std::endl;
        }

        // test
        auto final_weight {
            std::get<0>(
                bidi_walker_.walk_to_node(g.get_leaf_node())
            )
        };
        std::cout << "final weight: " << final_weight << std::endl;
        for (const auto& node : g.resident_nodes()) {
            const auto& [down_offset, right_offset] { node };
            const auto& [weight, forward_walk_edge, backward_walk_edge] {
                bidi_walker_.walk_to_node({down_offset, right_offset})
            };
            std::cout << "resident node: " << down_offset << ',' << right_offset << '=' << weight;
            if (forward_walk_edge.has_value()) {
                const auto& [forward_walk_from_node, forward_walk_to_node] { (*forward_walk_edge).inner_edge };
                const auto& [forward_walk_from_node_down, forward_walk_from_node_right] { forward_walk_from_node };
                std::cout << '(' << "fh_from" << forward_walk_from_node_down << ',' << forward_walk_from_node_right << ')';
            } else {
                std::cout << '(' << "fh_from_,_" << ')';
            }
            if (backward_walk_edge.has_value()) {
                const auto& [backward_walk_from_node, backward_walk_to_node] { (*backward_walk_edge).inner_edge };
                const auto& [backward_walk_to_node_down, backward_walk_to_node_right] { backward_walk_to_node };
                std::cout << '(' << "bh_to" << backward_walk_to_node_down << ',' << backward_walk_to_node_right << ')';
            } else {
                std::cout << '(' << "bh_to_,_" << ')';
            }
            std::cout << std::endl;
        }
    }

    TEST(BidiWalkerTest, SegmentationPointsLocal) {
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
        std::string seq1 { "aaalmaaa" };
        std::string seq2 { "zzzlmzzz" };
        pairwise_local_alignment_graph<decltype(seq1), decltype(seq2)> g {
            seq1,
            seq2,
            match_lookup,
            indel_lookup,
            freeride_lookup
        };

        bidi_walker<decltype(g)> bidi_walker_ { g };
        using hop = decltype(bidi_walker_)::hop;
        using segment = decltype(bidi_walker_)::segment;
        const auto& [parts, final_weight] { bidi_walker_.backtrack_segmentation_points() };
        std::cout << final_weight << std::endl;
        for (const auto& part : parts) {
            if (const hop* hop_ptr = std::get_if<hop>(&part)) {
                const auto& [from_node, to_node] { hop_ptr->edge.inner_edge };
                const auto& [from_node_down_offset, from_node_right_offset] { from_node };
                const auto& [to_node_down_offset, to_node_right_offset] { to_node };
                const auto& type { hop_ptr->edge.type };
                std::cout << "hop: " << from_node_down_offset << ',' << from_node_right_offset
                    << " to "
                    << to_node_down_offset << ',' << to_node_right_offset
                    << " edgetype " << static_cast<std::uint8_t>(type)
                    << std::endl;
            } else if (const segment* segment_ptr = std::get_if<segment>(&part)) {
                const auto& [from_node_down_offset, from_node_right_offset] { segment_ptr->from_node };
                const auto& [to_node_down_offset, to_node_right_offset] { segment_ptr->to_node };
                std::cout << "segment: " << from_node_down_offset << ',' << from_node_right_offset
                    << " to "
                    << to_node_down_offset << ',' << to_node_right_offset
                    << std::endl;
            } else {
                throw std::runtime_error("This should never hapen");
            }
        }
    }

}
