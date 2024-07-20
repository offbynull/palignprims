#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKER_H

#include <ranges>
#include <algorithm>
#include <iostream>
#include "path_container.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/concepts.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/container_creator_packs.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/bidi_walker.h"
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
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker::bidi_walker;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::sliced_subdivider::sliced_subdivider;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::container_creator_packs::container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::container_creator_packs::heap_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::concepts::backtrackable_node;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::concepts::backtrackable_edge;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker::forward_walker;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker::slot;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::path_container;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::element;
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
        readable_sliceable_pairwise_alignment_graph G,
        container_creator_pack<G, typename G::ED> CONTAINER_CREATOR_PACK = heap_container_creator_pack<G, typename G::ED, true>,
        bool error_check = true
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

        using SLICE_SLOT_CONTAINER_CREATOR = typename CONTAINER_CREATOR_PACK::SLICE_SLOT_CONTAINER_CREATOR;
        using RESIDENT_SLOT_CONTAINER_CREATOR = typename CONTAINER_CREATOR_PACK::RESIDENT_SLOT_CONTAINER_CREATOR;
        using ELEMENT_CONTAINER_CREATOR = typename CONTAINER_CREATOR_PACK::ELEMENT_CONTAINER_CREATOR;
        using PATH_CONTAINER_CREATOR = typename CONTAINER_CREATOR_PACK::PATH_CONTAINER_CREATOR;

        SLICE_SLOT_CONTAINER_CREATOR slice_slot_container_creator;
        RESIDENT_SLOT_CONTAINER_CREATOR resident_slot_container_creator;
        ELEMENT_CONTAINER_CREATOR element_container_creator;
        PATH_CONTAINER_CREATOR path_container_creator;

    public:
        backtracker(
            SLICE_SLOT_CONTAINER_CREATOR slice_slot_container_creator = {},
            RESIDENT_SLOT_CONTAINER_CREATOR resident_slot_container_creator = {},
            ELEMENT_CONTAINER_CREATOR element_container_creator = {},
            PATH_CONTAINER_CREATOR path_container_creator = {}
        )
        : slice_slot_container_creator { slice_slot_container_creator }
        , resident_slot_container_creator { resident_slot_container_creator }
        , element_container_creator { element_container_creator }
        , path_container_creator { path_container_creator } {}

        auto find_max_path(G& g) {
            bidi_walker<
                G,
                SLICE_SLOT_CONTAINER_CREATOR,
                RESIDENT_SLOT_CONTAINER_CREATOR,
                error_check
            > bidi_walker_ {
                g,
                slice_slot_container_creator,
                resident_slot_container_creator
            };
            using hop = decltype(bidi_walker_)::hop;
            using segment = decltype(bidi_walker_)::segment;
            const auto& [parts, final_weight] { bidi_walker_.backtrack_segmentation_points() };
            auto path { path_container_creator.create_empty(std::nullopt) };
            for (const auto& part : parts) {
                if (const hop* hop_ptr = std::get_if<hop>(&part)) {
                    path.push_back(hop_ptr->edge);
                } else if (const segment* segment_ptr = std::get_if<segment>(&part)) {
                    middle_sliceable_pairwise_alignment_graph<G> g_segment {
                        g,
                        segment_ptr->from_node,
                        segment_ptr->to_node
                    };
                    sliced_subdivider<
                        decltype(g_segment),
                        SLICE_SLOT_CONTAINER_CREATOR,
                        ELEMENT_CONTAINER_CREATOR,
                        error_check
                    > subdivider {
                        g_segment,
                        slice_slot_container_creator,
                        element_container_creator
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
