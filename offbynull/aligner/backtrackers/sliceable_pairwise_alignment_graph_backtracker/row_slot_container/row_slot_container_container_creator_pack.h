#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_ROW_SLOT_CONTAINER_ROW_SLOT_CONTAINER_CONTAINER_CREATOR_PACK_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_ROW_SLOT_CONTAINER_ROW_SLOT_CONTAINER_CONTAINER_CREATOR_PACK_H

#include <cstddef>
#include "offbynull/concepts.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/row_slot_container/slot.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtrackable_edge.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container
        ::row_slot_container_container_creator_pack {
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container::slot::slot;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtrackable_edge::backtrackable_edge;
    using offbynull::aligner::concepts::weight;
    using offbynull::concepts::random_access_sequence_container;
    using offbynull::concepts::unqualified_object_type;

    /**
     * Concept that's satisfied if `T` has the traits for creating the containers required by
     * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container::row_slot_container::row_slot_container,
     * referred to as a container creator pack.
     *
     * @tparam T Type to check.
     * @tparam E Graph edge identifier type.
     * @tparam ED Graph edge data type (edge weight).
     */
    template<
        typename T,
        typename E,
        typename ED
    >
    concept row_slot_container_container_creator_pack =
        unqualified_object_type<T>
        && backtrackable_edge<E>
        && weight<ED>
        && requires(const T t, std::size_t grid_right_cnt, std::size_t grid_depth_cnt, ED zero_weight) {
            { t.create_slot_container(grid_right_cnt, grid_depth_cnt, zero_weight) } -> random_access_sequence_container<slot<E, ED>>;
        };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_ROW_SLOT_CONTAINER_ROW_SLOT_CONTAINER_CONTAINER_CREATOR_PACK_H
