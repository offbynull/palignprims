#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BIDI_WALKER_UNIMPLEMENTED_BIDI_WALKER_CONTAINER_CREATOR_PACK_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BIDI_WALKER_UNIMPLEMENTED_BIDI_WALKER_CONTAINER_CREATOR_PACK_H

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker
        ::unimplemented_bidi_walker_container_creator_pack {

    /**
     * Unimplemented
     * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker::bidi_walker_container_creator_pack,
     * intended for documentation.
     */
    struct unimplemented_bidi_walker_container_creator_pack {
        /**
         * Create
         * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker::forward_walker::forward_walker
         * container creator pack (for forward walking the graph).
         *
         * @return @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker::forward_walker::forward_walker
         *     container creator pack, used by the forward walker.
         */
        auto create_forward_walker_container_creator_pack() const;

        /**
         * Create
         * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker::forward_walker::forward_walker
         * container creator pack (for forward walking a reversed view of the graph).
         *
         * @return @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker::forward_walker::forward_walker
         *     container creator pack, used by the backward walker.
         */
        auto create_backward_walker_container_creator_pack() const;
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BIDI_WALKER_UNIMPLEMENTED_BIDI_WALKER_CONTAINER_CREATOR_PACK_H