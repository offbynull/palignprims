#ifndef OFFBYNULL_ALIGNER_BACKTRACK_SLICED_BACKTRACK_H
#define OFFBYNULL_ALIGNER_BACKTRACK_SLICED_BACKTRACK_H

#include <cstddef>
#include <functional>
#include <ranges>
#include <algorithm>
#include "offbynull/aligner/backtrack/sliced_walker.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/backtrack/container_creator.h"
#include "offbynull/aligner/backtrack/container_creators.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/concepts.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::backtrack::sliced_backtrack {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_parwise_alignment_graph;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::backtrack::sliced_walker::sliced_forward_walker;
    using offbynull::aligner::backtrack::sliced_walker::sliced_backward_walker;
    using offbynull::aligner::backtrack::container_creator::container_creator;
    using offbynull::aligner::backtrack::container_creators::vector_container_creator;
    using offbynull::concepts::range_of_type;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::utils::max_element;

    template<
        readable_sliceable_parwise_alignment_graph G,
        weight WEIGHT,
        container_creator SLICE_SLOT_ALLOCATOR=vector_container_creator<slot<typename G::N, WEIGHT>>,
        container_creator RESIDENT_SLOT_ALLOCATOR=vector_container_creator<slot<typename G::N, WEIGHT>>,
        bool error_check = true
    >
    requires requires(typename G::N n)
    {
        {n < n} -> std::same_as<bool>;
    }
    class sliced_backtracker {
    private:
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using INDEX = typename G::INDEX;
        sliced_forward_walker<G, WEIGHT, SLICE_SLOT_ALLOCATOR, RESIDENT_SLOT_ALLOCATOR, error_check> forward_walker;
        sliced_backward_walker<G, WEIGHT, SLICE_SLOT_ALLOCATOR, RESIDENT_SLOT_ALLOCATOR, error_check> backward_walker;
        INDEX down_mid_offset;

    public:
        sliced_backtract(
            G& graph_,
            std::function<WEIGHT(const E&)> get_edge_weight_func_,
            SLICE_SLOT_ALLOCATOR slice_slot_container_creator = {},
            RESIDENT_SLOT_ALLOCATOR resident_slot_container_creator = {}
        )
        : forward_walker{ g, get_weight_weight_func_, slice_slot_container_creator, resident_slot_container_creator }
        , backward_walker{ g, get_weight_weight_func_, slice_slot_container_creator, resident_slot_container_creator }
        , down_mid_offset{ g.down_node_cnt / 2.0 } {}

        auto walk() {
            while (true) {
                forward_walker.next();
            }
            sliced_forward_walker<G, WEIGHT, SLICE_SLOT_ALLOCATOR, RESIDENT_SLOT_ALLOCATOR, error_check> forward { g };
            sliced_backward_walker<G, WEIGHT, SLICE_SLOT_ALLOCATOR, RESIDENT_SLOT_ALLOCATOR, error_check> backward;
            graph.down_node_cnt / 2;
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACK_SLICED_BACKTRACK_H
