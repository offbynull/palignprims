#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_ROW_SLOT_CONTAINER_UNIMPLEMENTED_ROW_SLOT_CONTAINER_CONTAINER_CREATOR_PACK_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_ROW_SLOT_CONTAINER_UNIMPLEMENTED_ROW_SLOT_CONTAINER_CONTAINER_CREATOR_PACK_H

#include <cstddef>
#include "offbynull/aligner/concepts.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container
        ::unimplemented_row_slot_container_container_creator_pack {

    using offbynull::aligner::concepts::weight;

    /**
     * Unimplemented
     * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container::row_slot_container_container_creator_pack::row_slot_container_container_creator_pack,
     * intended for documentation.
     */
    struct unimplemented_row_slot_container_container_creator_pack {
        /**
         * Create random access container used to store slots.
         *
         * @param grid_right_cnt Right dimension of the underlying pairwise alignment graph instance.
         * @param grid_depth_cnt Depth dimension of the underlying pairwise alignment graph instance.
         * @param zero_weight Initial weight, equivalent to 0 for numeric weights.
         * @return Empty random access container which can hold at least `grid_right_cnt * grid_depth_cnt` elements.
         */
        auto create_slot_container(
            std::size_t grid_right_cnt,
            std::size_t grid_depth_cnt,
            weight auto zero_weight
        ) const;
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_ROW_SLOT_CONTAINER_UNIMPLEMENTED_ROW_SLOT_CONTAINER_CONTAINER_CREATOR_PACK_H
