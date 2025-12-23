#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_READY_QUEUE_READY_QUEUE_CONTAINER_CREATOR_PACK_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_READY_QUEUE_READY_QUEUE_CONTAINER_CREATOR_PACK_H

#include <cstddef>
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::ready_queue::ready_queue_container_creator_pack {
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::concepts::random_access_sequence_container;
    using offbynull::concepts::unqualified_object_type;

    /**
     * Concept that's satisfied if `T` has the traits for creating the containers required by
     * @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::ready_queue::ready_queue::ready_queue, referred to as a
     * container creator pack.
     *
     * @tparam T Type to check.
     * @tparam SLOT_INDEX Slot indexer type. Must be wide enough to hold the value `grid_down_cnt * grid_right_cnt * grid_depth_cnt`
     *     (variables being multiplied are the dimensions of the underlying pairwise alignment graph instance).
     */
    template<
        typename T,
        typename SLOT_INDEX
    >
    concept ready_queue_container_creator_pack =
        unqualified_object_type<T>
        && widenable_to_size_t<SLOT_INDEX>
        && requires(
            const T t,
            std::size_t grid_down_cnt,
            std::size_t grid_right_cnt,
            std::size_t grid_depth_cnt
        ) {
            { t.create_queue_container(grid_down_cnt, grid_right_cnt, grid_depth_cnt) } -> random_access_sequence_container<SLOT_INDEX>;
        };
}
#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_READY_QUEUE_READY_QUEUE_CONTAINER_CREATOR_PACK_H
