#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_READY_QUEUE_UNIMPLEMENTED_READY_QUEUE_CONTAINER_CREATOR_PACK_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_READY_QUEUE_UNIMPLEMENTED_READY_QUEUE_CONTAINER_CREATOR_PACK_H

#include <cstddef>
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::ready_queue
        ::unimplemented_ready_queue_container_creator_pack {
    using offbynull::concepts::widenable_to_size_t;

    /**
     * Unimplemented
     * @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::ready_queue::ready_queue_container_creator_pack::ready_queue_container_creator_pack,
     * intended for documentation.
     *
     * @tparam SLOT_INDEX Slot indexer type. Must be wide enough to hold the value `grid_down_cnt * grid_right_cnt * grid_depth_cnt`
     *     (variables being multiplied are the dimensions of the underlying pairwise alignment graph instance).
     */
    template<
        widenable_to_size_t SLOT_INDEX
    >
    struct unimplemented_ready_queue_container_creator_pack {
        /**
         * Create random access container used to store internal queue elements.
         *
         * The behavior of this function is undefined if `SLOT_INDEX` isn't wide enough to hold
         * `grid_down_cnt * grid_right_cnt * grid_depth_cnt`.
         *
         * @param grid_down_cnt Down dimension of the underlying pairwise alignment graph instance.
         * @param grid_right_cnt Right dimension of the underlying pairwise alignment graph instance.
         * @param grid_depth_cnt Depth dimension of the underlying pairwise alignment graph instance.
         * @return Empty random access container which can hold at least `grid_down_cnt * grid_right_cnt * grid_depth_cnt` elements.
         */
        auto create_queue_container(
            std::size_t grid_down_cnt,
            std::size_t grid_right_cnt,
            std::size_t grid_depth_cnt
        ) const;
    };
}
#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_READY_QUEUE_UNIMPLEMENTED_READY_QUEUE_CONTAINER_CREATOR_PACK_H
