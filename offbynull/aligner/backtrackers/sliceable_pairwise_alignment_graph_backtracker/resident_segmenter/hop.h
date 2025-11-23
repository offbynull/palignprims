#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SEGMENTER_HOP_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SEGMENTER_HOP_H

#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtrackable_edge.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::hop {
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtrackable_edge::backtrackable_edge;

    PACK_STRUCT_START
    /**
     * An edge connecting two
     * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::segment::segment "segments"
     * within the same @ref offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_pairwise_alignment_graph. The
     * edge making up a hop has at least one of its nodes as a resident node. That is, ...
     *
     *  * either the source node is a resident node.
     *  * either the destination node is a resident node.
     *  * both the source node and destination node are resident nodes.
     *
     * ```
     *           segment
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
     *                           '.
     *                             '.
     *                               '.
     *                                 '.
     *                                   '.  hop
     *                                     '.
     *                                       '.
     *                                         '.
     *                                           '.
     *                                             v      segment
     *                                              *---->*---->*---->*-
     *                                              |'.   |'.   |'.   |'
     *                                              |  '. |  '. |  '. |
     *                                              v    vv    vv    vv
     *                                              *---->*---->*---->*-
     *                                              |'.   |'.   |'.   |'
     *                                              |  '. |  '. |  '. |
     *                                              v    vv    vv    vv
     *                                              *---->*---->*---->*-
     * ```
     *
     * @tparam E Graph edge type.
     */
    template<backtrackable_edge E>
    struct hop {
        /** Edge connecting two segments. */
        E edge;
    }
    PACK_STRUCT_STOP;
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SEGMENTER_HOP_H
