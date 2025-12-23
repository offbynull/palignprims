#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SLOT_CONTAINER_UNIMPLEMENTED_RESIDENT_SLOT_CONTAINER_CONTAINER_CREATOR_PACK_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SLOT_CONTAINER_UNIMPLEMENTED_RESIDENT_SLOT_CONTAINER_CONTAINER_CREATOR_PACK_H

#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_slot_container/resident_slot_with_node.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container
        ::unimplemented_resident_slot_container_container_creator_pack {
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container::resident_slot_with_node
        ::resident_slot_with_node;

    /**
     * Unimplemented
     * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container::resident_slot_container_container_creator_pack::resident_slot_container_container_creator_pack,
     * intended for documentation.
     */
    struct unimplemented_resident_slot_container_container_creator_pack {
        /**
         * Create random access container used to store slots.
         *
         * @param r Range of resident slots.
         * @return Empty random access container holding elements within `r`.
         */
        auto create_slot_container(
            auto&& r
        ) const;
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SLOT_CONTAINER_UNIMPLEMENTED_RESIDENT_SLOT_CONTAINER_CONTAINER_CREATOR_PACK_H
