#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKER_H

#include <ranges>
#include <algorithm>
#include <iostream>
#include "path_container.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/concepts.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_segmenter.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/sliced_subdivider.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/helpers/container_creators.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/prefix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/suffix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/middle_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/reversed_sliceable_pairwise_alignment_graph.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtracker {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::hop;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::segment;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::resident_segmenter;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::resident_segmenter_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::resident_segmenter_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::resident_segmenter_stack_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::sliced_subdivider::sliced_subdivider;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::sliced_subdivider::sliced_subdivider_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::sliced_subdivider::sliced_subdivider_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::sliced_subdivider::sliced_subdivider_stack_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::concepts::backtrackable_node;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::concepts::backtrackable_edge;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker::forward_walker;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker::slot;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::element;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::path_container;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::path_container_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::path_container_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::path_container_stack_container_creator_pack;
    using offbynull::helpers::container_creators::container_creator;
    using offbynull::helpers::container_creators::container_creator_of_type;
    using offbynull::helpers::container_creators::vector_container_creator;
    using offbynull::helpers::container_creators::static_vector_container_creator;
    using offbynull::aligner::graphs::prefix_sliceable_pairwise_alignment_graph::prefix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::suffix_sliceable_pairwise_alignment_graph::suffix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::middle_sliceable_pairwise_alignment_graph::middle_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::reversed_sliceable_pairwise_alignment_graph::reversed_sliceable_pairwise_alignment_graph;
    using offbynull::concepts::range_of_type;
    using offbynull::concepts::widenable_to_size_t;





    template<
        typename T,
        typename G
    >
    concept backtracker_container_creator_pack =
        readable_sliceable_pairwise_alignment_graph<G>
        && resident_segmenter_container_creator_pack<typename T::RESIDENT_SEGMENTER_CONTAINER_CREATOR_PACK, G>
        && sliced_subdivider_container_creator_pack<typename T::SLICED_SUBDIVIDER_CONTAINER_CREATOR_PACK, G>
        && container_creator_of_type<typename T::PATH_CONTAINER_CREATOR, typename G::E>;

    template<
        bool debug_mode,
        readable_sliceable_pairwise_alignment_graph G
    >
    struct backtracker_heap_container_creator_pack {
        using E = typename G::E;

        using RESIDENT_SEGMENTER_CONTAINER_CREATOR_PACK=resident_segmenter_heap_container_creator_pack<debug_mode, G>;
        using SLICED_SUBDIVIDER_CONTAINER_CREATOR_PACK=sliced_subdivider_heap_container_creator_pack<debug_mode, G>;
        using PATH_CONTAINER_CREATOR=vector_container_creator<E, debug_mode>;
    };

    template<
        bool debug_mode,
        readable_sliceable_pairwise_alignment_graph G,
        std::size_t grid_down_cnt,
        std::size_t grid_right_cnt
    >
    struct backtracker_stack_container_creator_pack {
        using E = typename G::E;

        using RESIDENT_SEGMENTER_CONTAINER_CREATOR_PACK=resident_segmenter_stack_container_creator_pack<
            debug_mode,
            G,
            grid_down_cnt,
            grid_right_cnt
        >;
        using SLICED_SUBDIVIDER_CONTAINER_CREATOR_PACK=sliced_subdivider_stack_container_creator_pack<
            debug_mode,
            G,
            grid_down_cnt,
            grid_right_cnt
        >;
        using PATH_CONTAINER_CREATOR=static_vector_container_creator<
            E,
            G::limits(grid_down_cnt, grid_right_cnt).max_path_edge_cnt,
            debug_mode
        >;
    };





    template<
        bool debug_mode,
        readable_sliceable_pairwise_alignment_graph G,
        backtracker_container_creator_pack<G> CONTAINER_CREATOR_PACK=backtracker_heap_container_creator_pack<debug_mode, G>
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

        using RESIDENT_SEGMENTER_CONTAINER_CREATOR_PACK=typename CONTAINER_CREATOR_PACK::RESIDENT_SEGMENTER_CONTAINER_CREATOR_PACK;
        using SLICED_SUBDIVIDER_CONTAINER_CREATOR_PACK=typename CONTAINER_CREATOR_PACK::SLICED_SUBDIVIDER_CONTAINER_CREATOR_PACK;
        using PATH_CONTAINER_CREATOR=typename CONTAINER_CREATOR_PACK::PATH_CONTAINER_CREATOR;

    public:
        backtracker() {}

        auto find_max_path(
            const G& g,
            const ED final_weight_comparison_tolerance
        ) {
            resident_segmenter<
                debug_mode,
                G,
                RESIDENT_SEGMENTER_CONTAINER_CREATOR_PACK
            > resident_segmenter_ {};
            using hop_ = hop<E>;
            using segment_ = segment<N>;
            const auto& [parts, final_weight] { resident_segmenter_.backtrack_segmentation_points(g, final_weight_comparison_tolerance) };
            auto path { PATH_CONTAINER_CREATOR {}.create_empty(std::nullopt) };
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
                        g_segment
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
                    throw std::runtime_error("This should never hapen");
                }
            }
            return std::make_pair(path, final_weight);
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKER_H
