#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SEGMENTER_SEGMENT_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SEGMENTER_SEGMENT_H

#include <stdexcept>
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtrackable_node.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::segment {
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtrackable_node::backtrackable_node;

    PACK_STRUCT_START
    /**
     * A segment of an @ref offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_pairwise_alignment_graph which
     * is uninterrupted by connections from / to resident nodes. That is, ...
     *
     *  * the segment's root node can have incoming edges from resident nodes, but not outgoing edges to resident nodes.
     *  * the segment's leaf node can have outgoing edges to resident nodes, but not incoming edges from resident nodes.
     *  * all other nodes in the segment must feed forward, meaning no edges to / from resident nodes).
     *
     * ```
     *     segment where nodes
     *     only feed forward to
     *     immediate neighbors
     *
     *  *---->*---->*---->*---->*
     *  |'.   |'.   |'.   |'.   |
     *  |  '. |  '. |  '. |  '. |
     *  v    vv    vv    vv    vv
     *  *---->*---->*---->*---->*
     *  |'.   |'.   |'.   |'.   |
     *  |  '. |  '. |  '. |  '. |
     *  v    vv    vv    vv    vv
     *  *---->*---->*---->*---->*
     *  |'.   |'.   |'.   |'.   |
     *  |  '. |  '. |  '. |  '. |
     *  v    vv    vv    vv    vv
     *  *---->*---->*---->*---->*
     * ```
     *
     * @tparam N Graph node type.
     */
    template<backtrackable_node N>
    struct segment {
        /** Top-left of uninterruptible graph segment. */
        N from_node;
        /** Bottom-right of uninterruptible graph segment. */
        N to_node;

        // segment(N from_node_,N to_node_) : from_node(from_node_), to_node(to_node_) {
        //     if constexpr(debug_mode) {
        //         // TODO: Is this check wrong? If it's a reversed graph, the condition will hit when it shouldn't?
        //         if (from_node > to_node) {
        //             throw std::runtime_error { "from_node must be top-left" };
        //         }
        //     }
        // }
    }
    PACK_STRUCT_STOP;
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SEGMENTER_SEGMENT_H
