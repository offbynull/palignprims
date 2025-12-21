#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graphs/pairwise_local_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_fitting_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_overlap_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_global_alignment_graph.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtracker.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_segmenter/hop.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_segmenter/segment.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_segmenter/resident_segmenter.h"
#include "offbynull/aligner/scorers/simple_scorer.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"
#include <cstddef>
#include <stdfloat>
#include <string>
#include <iostream>
#include <ostream>
#include <variant>
#include <cstdint>
#include <cstddef>
#include <stdexcept>

namespace {
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::hop::hop;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::segment::segment;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::resident_segmenter
        ::resident_segmenter;
    using offbynull::aligner::graphs::pairwise_global_alignment_graph::pairwise_global_alignment_graph;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph;
    using offbynull::aligner::graphs::pairwise_fitting_alignment_graph::pairwise_fitting_alignment_graph;
    using offbynull::aligner::graphs::pairwise_overlap_alignment_graph::pairwise_overlap_alignment_graph;
    using offbynull::aligner::scorers::simple_scorer::simple_scorer;
    using offbynull::utils::is_debug_mode;

    TEST(OABSRResidentSegmenterTest, SegmentationPointsLocal) {
        auto substitution_scorer {
            simple_scorer<is_debug_mode(), std::size_t, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64)
        };
        auto gap_scorer { simple_scorer<is_debug_mode(), std::size_t, char, char, std::float64_t>::create_gap(-1.0f64) };
        auto freeride_scorer { simple_scorer<is_debug_mode(), std::size_t, char, char, std::float64_t>::create_freeride(0.0f64) };
        std::string seq1 { "aaaaalmnaaaaa" };
        std::string seq2 { "zzzzzlVnzzzzz" };
        pairwise_local_alignment_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer),
            decltype(freeride_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };

        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        resident_segmenter<is_debug_mode(), decltype(g)> segmenter {};
        using hop_ = hop<E>;
        using segment_ = segment<N>;
        const auto& [parts, final_weight] { segmenter.backtrack_segmentation_points(g, 0.000001f64) };
        std::cout << final_weight << std::endl;
        for (const auto& part : parts) {
            if (const hop_* hop_ptr = std::get_if<hop_>(&part)) {
                const auto& [from_node, to_node] { hop_ptr->edge.inner_edge };
                const auto& [from_node_down_offset, from_node_right_offset] { from_node };
                const auto& [to_node_down_offset, to_node_right_offset] { to_node };
                const auto& type { hop_ptr->edge.type };
                std::cout << "hop: " << from_node_down_offset << ',' << from_node_right_offset
                    << " to "
                    << to_node_down_offset << ',' << to_node_right_offset
                    << " edgetype " << static_cast<std::uint8_t>(type)
                    << std::endl;
            } else if (const segment_* segment_ptr = std::get_if<segment_>(&part)) {
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

    TEST(OABSRResidentSegmenterTest, SegmentationPointsOverlap) {
        auto substitution_scorer {
            simple_scorer<is_debug_mode(), std::size_t, char, char, std::float64_t>::create_substitution(1.0f64, -1.0f64)
        };
        auto gap_scorer { simple_scorer<is_debug_mode(), std::size_t, char, char, std::float64_t>::create_gap(-1.0f64) };
        auto freeride_scorer { simple_scorer<is_debug_mode(), std::size_t, char, char, std::float64_t>::create_freeride(0.0f64) };
        std::string seq1 { "aaaaalmn" };
        std::string seq2 { "lmnzzzzz" };
        pairwise_overlap_alignment_graph<
            is_debug_mode(),
            std::size_t,
            std::float64_t,
            decltype(seq1),
            decltype(seq2),
            decltype(substitution_scorer),
            decltype(gap_scorer),
            decltype(freeride_scorer)
        > g {
            seq1,
            seq2,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };
        using N = typename decltype(g)::N;
        using E = typename decltype(g)::E;

        // // walk
        // offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker::backtracker<
        //     decltype(g),
        //     std::size_t
        // > backtracker_ {};
        // const auto& [path, weight] { backtracker_.find_max_path(g) };
        // for (const E& e : path) {
        //     const auto& [n1, n2] { e.inner_edge };
        //     std::cout << n1.first << '/' << n1.second << "->" << n2.first << '/' << n2.second << ' ';
        // }
        // std::cout << std::endl;
        // std::cout << weight << std::endl;

        // 0/0->5/0 5/0->6/1 6/1->7/2 7/2->8/3 8/3->8/8
        // 3

        resident_segmenter<is_debug_mode(), decltype(g)> segmenter {};
        using hop_ = hop<E>;
        using segment_ = segment<N>;
        const auto& [parts, final_weight] { segmenter.backtrack_segmentation_points(g, 0.000001f64) };
        std::cout << final_weight << std::endl;
        for (const auto& part : parts) {
            if (const hop_* hop_ptr = std::get_if<hop_>(&part)) {
                const auto& [from_node, to_node] { hop_ptr->edge.inner_edge };
                const auto& [from_node_down_offset, from_node_right_offset] { from_node };
                const auto& [to_node_down_offset, to_node_right_offset] { to_node };
                const auto& type { hop_ptr->edge.type };
                std::cout << "hop: " << from_node_down_offset << ',' << from_node_right_offset
                    << " to "
                    << to_node_down_offset << ',' << to_node_right_offset
                    << " edgetype " << static_cast<std::uint8_t>(type)
                    << std::endl;
            } else if (const segment_* segment_ptr = std::get_if<segment_>(&part)) {
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
