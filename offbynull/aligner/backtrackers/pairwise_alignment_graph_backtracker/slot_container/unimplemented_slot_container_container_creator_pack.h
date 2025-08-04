#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLOT_CONTAINER_UNIMPLEMENTED_SLOT_CONTAINER_CONTAINER_CREATOR_PACK_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLOT_CONTAINER_UNIMPLEMENTED_SLOT_CONTAINER_CONTAINER_CREATOR_PACK_H

#include <cstddef>

namespace offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container
        ::unimplemented_slot_container_container_creator_pack {
    /**
     * Unimplemented
     * @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container::slot_container_container_creator_pack::slot_container_container_creator_pack,
     * intended for documentation.
     */
    struct unimplemented_slot_container_container_creator_pack {
        /**
         * Create random access container used to store slot elements.
         *
         * @param grid_down_cnt Down dimension of the underlying pairwise alignment graph instance.
         * @param grid_right_cnt Right dimension of the underlying pairwise alignment graph instance.
         * @param grid_depth_cnt Depth dimension of the underlying pairwise alignment graph instance.
         * @return Empty random access container which can hold at least `grid_down_cnt * grid_right_cnt * grid_depth_cnt` elements.
         */
        auto create_slot_container(
            std::size_t grid_down_cnt,
            std::size_t grid_right_cnt,
            std::size_t grid_depth_cnt
        ) const;
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLOT_CONTAINER_UNIMPLEMENTED_SLOT_CONTAINER_CONTAINER_CREATOR_PACK_H
