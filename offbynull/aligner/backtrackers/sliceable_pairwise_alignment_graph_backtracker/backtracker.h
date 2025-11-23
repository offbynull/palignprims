#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKER_H

#include <algorithm>
#include <cstddef>
#include <utility>
#include <variant>
#include <type_traits>
#include <stdexcept>
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtrackable_node.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtrackable_edge.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtracker_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtracker_heap_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtracker_stack_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_segmenter/resident_segmenter.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/sliced_subdivider/sliced_subdivider.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/prefix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/suffix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/middle_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/reversed_sliceable_pairwise_alignment_graph.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtracker {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::hop::hop;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::segment::segment;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::resident_segmenter
        ::resident_segmenter;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::sliced_subdivider
        ::sliced_subdivider::sliced_subdivider;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtrackable_node::backtrackable_node;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtrackable_edge::backtrackable_edge;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtracker_container_creator_pack
        ::backtracker_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtracker_heap_container_creator_pack
        ::backtracker_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtracker_stack_container_creator_pack
        ::backtracker_stack_container_creator_pack;
    using offbynull::aligner::graphs::prefix_sliceable_pairwise_alignment_graph::prefix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::suffix_sliceable_pairwise_alignment_graph::suffix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::middle_sliceable_pairwise_alignment_graph::middle_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::reversed_sliceable_pairwise_alignment_graph::reversed_sliceable_pairwise_alignment_graph;
    using offbynull::concepts::random_access_range_of_type;
    using offbynull::concepts::range_of_type;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::concepts::unqualified_object_type;
    using offbynull::utils::static_vector_typer;

    template<
        bool debug_mode,
        readable_sliceable_pairwise_alignment_graph G,
        backtracker_container_creator_pack<
            typename G::N,
            typename G::E,
            typename G::ED
        > CONTAINER_CREATOR_PACK = backtracker_heap_container_creator_pack<
            debug_mode,
            typename G::N,
            typename G::E,
            typename G::ED,
            true
        >
    >
    requires backtrackable_node<typename G::N> &&
        backtrackable_edge<typename G::E>
    class backtracker {
    private:
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using INDEX = typename G::INDEX;

        using RESIDENT_SEGMENTER_CONTAINER_CREATOR_PACK =
            decltype(std::declval<CONTAINER_CREATOR_PACK>().create_resident_segmenter_container_creator_pack());
        using SLICED_SUBDIVIDER_CONTAINER_CREATOR_PACK =
            decltype(std::declval<CONTAINER_CREATOR_PACK>().create_sliced_subdivider_container_creator_pack());
        using PATH_CONTAINER = decltype(std::declval<CONTAINER_CREATOR_PACK>().create_path_container(0zu));

        CONTAINER_CREATOR_PACK container_creator_pack;

    public:
        backtracker(
            CONTAINER_CREATOR_PACK container_creator_pack_ = {}
        )
        : container_creator_pack { container_creator_pack_ } {}

        auto find_max_path(
            const G& g,
            const ED final_weight_comparison_tolerance
        ) {
            resident_segmenter<
                debug_mode,
                G,
                RESIDENT_SEGMENTER_CONTAINER_CREATOR_PACK
            > resident_segmenter_ {
                container_creator_pack.create_resident_segmenter_container_creator_pack()
            };
            using hop_ = hop<E>;
            using segment_ = segment<N>;
            const auto& [parts, final_weight] { resident_segmenter_.backtrack_segmentation_points(g, final_weight_comparison_tolerance) };
            PATH_CONTAINER path {
                container_creator_pack.create_path_container(
                    g.path_edge_capacity
                )
            };
            for (const auto& part : parts) {
                if (const hop_* hop_ptr = std::get_if<hop_>(&part)) {
                    path.push_back(hop_ptr->edge);
                } else if (const segment_* segment_ptr = std::get_if<segment_>(&part)) {
                    middle_sliceable_pairwise_alignment_graph<debug_mode, G> g_segment {
                        g,
                        segment_ptr->from_node,
                        segment_ptr->to_node
                    };
                    sliced_subdivider<
                        debug_mode,
                        decltype(g_segment),
                        SLICED_SUBDIVIDER_CONTAINER_CREATOR_PACK
                    > subdivider {
                        g_segment,
                        container_creator_pack.create_sliced_subdivider_container_creator_pack()
                    };
                    auto path_container { subdivider.subdivide() };
                    auto size_before { path.size() };
                    for (const E& edge : path_container.walk_path_backward()) {
                        path.push_back(edge);
                    }
                    std::reverse(
                        path.begin() + size_before,
                        path.end()
                    );
                } else {
                    throw std::runtime_error { "This should never happen" };
                }
            }
            return std::make_pair(path, final_weight);
        }
    };


    template<
        bool debug_mode,
        bool minimize_allocations
    >
    auto heap_find_max_path(
        const readable_sliceable_pairwise_alignment_graph auto& g,
        typename std::remove_cvref_t<decltype(g)>::ED final_weight_comparison_tolerance
    ) {
        using G = std::remove_cvref_t<decltype(g)>;
        return backtracker<
            debug_mode,
            G,
            backtracker_heap_container_creator_pack<
                debug_mode,
                typename G::N,
                typename G::E,
                typename G::ED,
                minimize_allocations
            >
        > {}.find_max_path(g, final_weight_comparison_tolerance);
    }

    template<
        bool debug_mode,
        std::size_t grid_right_cnt,
        std::size_t grid_depth_cnt,
        std::size_t resident_nodes_capacity,
        std::size_t path_edge_capacity
    >
    auto stack_find_max_path(
        const readable_sliceable_pairwise_alignment_graph auto& g,
        typename std::remove_cvref_t<decltype(g)>::ED final_weight_comparison_tolerance
    ) {
        using G = std::remove_cvref_t<decltype(g)>;
        using N = typename G::N;
        using E = typename G::E;
        using ED = typename G::ED;
        if constexpr (debug_mode) {
            if (g.grid_right_cnt != grid_right_cnt
                || g.grid_depth_cnt != grid_depth_cnt
                || g.resident_nodes_capacity != resident_nodes_capacity
                || g.path_edge_capacity != path_edge_capacity) {
                throw std::runtime_error { "Unexpected graph dimensions" };
            }
        }
        return backtracker<
            debug_mode,
            G,
            backtracker_stack_container_creator_pack<
                debug_mode,
                N,
                E,
                ED,
                grid_right_cnt,
                grid_depth_cnt,
                resident_nodes_capacity,
                path_edge_capacity
            >
        > {}.find_max_path(g, final_weight_comparison_tolerance);
    }
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKER_H
