#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLOT_CONTAINER_SLOT_CONTAINER_CONTAINER_CREATOR_PACK_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLOT_CONTAINER_SLOT_CONTAINER_CONTAINER_CREATOR_PACK_H

#include <cstddef>
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtrackable_node.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtrackable_edge.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/slot_container/slot.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container::slot_container_container_creator_pack {
    using offbynull::aligner::concepts::weight;
    using offbynull::concepts::random_access_range_of_type;
    using offbynull::concepts::unqualified_object_type;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtrackable_node::backtrackable_node;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtrackable_edge::backtrackable_edge;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container::slot::slot;

    /**
     * Concept that's satisfied if `T` has the traits for creating the containers required by
     * @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container::slot_container::slot_container, referred
     * to as a container creator pack.
     *
     * @tparam T Type to check.
     * @tparam N Graph node type.
     * @tparam E Graph edge type.
     * @tparam ED Graph edge data type (edge weight).
     * @tparam PARENT_COUNT Graph node incoming edge counter type. Must be wide enough to hold the maximum number of incoming edges across
     *      all nodes in the underlying pairwise alignment graph instance (e.g., across all nodes in any global pairwise alignment graph, a
     *      node can have at most 3 incoming edges).
     */
    template<
        typename T,
        typename N,
        typename E,
        typename ED,
        typename PARENT_COUNT
    >
    concept slot_container_container_creator_pack =
        unqualified_object_type<T>
        && backtrackable_node<N>
        && backtrackable_edge<E>
        && weight<ED>
        && widenable_to_size_t<PARENT_COUNT>
        && requires(
            const T t,
            std::size_t grid_down_cnt,
            std::size_t grid_right_cnt,
            std::size_t grid_depth_cnt
        ) {
            { t.create_slot_container(grid_down_cnt, grid_right_cnt, grid_depth_cnt) } -> random_access_range_of_type<
                slot<N, E, ED, PARENT_COUNT>
            >;
        };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLOT_CONTAINER_SLOT_CONTAINER_CONTAINER_CREATOR_PACK_H
