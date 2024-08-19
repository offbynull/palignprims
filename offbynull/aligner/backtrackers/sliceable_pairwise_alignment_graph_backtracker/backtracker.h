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
    using offbynull::aligner::graphs::prefix_sliceable_pairwise_alignment_graph::prefix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::suffix_sliceable_pairwise_alignment_graph::suffix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::middle_sliceable_pairwise_alignment_graph::middle_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::reversed_sliceable_pairwise_alignment_graph::reversed_sliceable_pairwise_alignment_graph;
    using offbynull::concepts::random_access_range_of_type;
    using offbynull::concepts::range_of_type;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::utils::static_vector_typer;





    template<
        typename T,
        typename N,
        typename E,
        typename ED
    >
    concept backtracker_container_creator_pack =
        weight<ED>
        && requires(T t) {
            { t.create_resident_segmenter_container_creator_pack() } -> resident_segmenter_container_creator_pack<N, E, ED>;
            { t.create_sliced_subdivider_container_creator_pack() } -> sliced_subdivider_container_creator_pack<N, E, ED>;
            { t.create_path_container() } -> random_access_range_of_type<E>;
        };

    template<
        bool debug_mode,
        typename N,
        typename E,
        weight ED
    >
    struct backtracker_heap_container_creator_pack {
        resident_segmenter_heap_container_creator_pack<debug_mode, N, E, ED> create_resident_segmenter_container_creator_pack() {
            return {};
        }

        sliced_subdivider_heap_container_creator_pack<debug_mode, N, E, ED> create_sliced_subdivider_container_creator_pack() {
            return {};
        }

        std::vector<E> create_path_container() {
            return {};
        }
    };

    template<
        bool debug_mode,
        typename N,
        typename E,
        weight ED,
        std::size_t grid_right_cnt,
        std::size_t grid_depth_cnt,
        std::size_t resident_nodes_capacity,
        std::size_t path_edge_capacity
    >
    struct backtracker_stack_container_creator_pack {
        resident_segmenter_stack_container_creator_pack<debug_mode, N, E, ED, grid_right_cnt, grid_depth_cnt, resident_nodes_capacity> create_resident_segmenter_container_creator_pack() {
            return {};
        }

        sliced_subdivider_stack_container_creator_pack<debug_mode, N, E, ED, grid_right_cnt, grid_depth_cnt, resident_nodes_capacity> create_sliced_subdivider_container_creator_pack() {
            return {};
        }

        using CONTAINER_TYPE = typename static_vector_typer<E, path_edge_capacity, debug_mode>::type;
        CONTAINER_TYPE create_path_container() const  {
            return {};
        }
    };





    template<
        bool debug_mode,
        readable_sliceable_pairwise_alignment_graph G,
        backtracker_container_creator_pack<typename G::N, typename G::E, typename G::ED> CONTAINER_CREATOR_PACK = backtracker_heap_container_creator_pack<debug_mode, typename G::N, typename G::E, typename G::ED>
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

        using RESIDENT_SEGMENTER_CONTAINER_CREATOR_PACK = decltype(std::declval<CONTAINER_CREATOR_PACK>().create_resident_segmenter_container_creator_pack());
        using SLICED_SUBDIVIDER_CONTAINER_CREATOR_PACK = decltype(std::declval<CONTAINER_CREATOR_PACK>().create_sliced_subdivider_container_creator_pack());
        using PATH_CONTAINER = decltype(std::declval<CONTAINER_CREATOR_PACK>().create_path_container());

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
            PATH_CONTAINER path { container_creator_pack.create_path_container() };
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
                    throw std::runtime_error("This should never hapen");
                }
            }
            return std::make_pair(path, final_weight);
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKER_H
