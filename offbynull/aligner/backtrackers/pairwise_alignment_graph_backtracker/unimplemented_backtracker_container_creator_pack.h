#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_UNIMPLEMENTED_BACKTRACKER_CONTAINER_CREATOR_PACK_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_UNIMPLEMENTED_BACKTRACKER_CONTAINER_CREATOR_PACK_H

#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtracker_container_creator_pack.h"

namespace offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::unimplemented_backtracker_container_creator_pack {

    /**
     * Unimplemented
     * @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker_container_creator_pack::backtracker_container_creator_pack,
     * intended for documentation.
     */
    struct unimplemented_backtracker_container_creator_pack {
        /**
         * Create @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container::slot_container container
         * creator pack.
         *
         * @return @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container::slot_container container
         * creator pack.
         */
        auto create_slot_container_container_creator_pack() const;

        /**
         * Create @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::ready_queue::ready_queue container creator
         * pack.
         *
         * @return @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::ready_queue::ready_queue container creator
         * pack.
         */
        auto create_ready_queue_container_creator_pack() const;

        /**
         * Create random access container used to store path within a graph (sequence of graph edges).
         *
         * @return Empty random access container.
         */
        auto create_path_container() const;
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_UNIMPLEMENTED_BACKTRACKER_CONTAINER_CREATOR_PACK_H
