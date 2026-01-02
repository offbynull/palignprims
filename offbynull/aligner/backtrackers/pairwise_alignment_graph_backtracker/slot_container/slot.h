#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLOT_CONTAINER_SLOT_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLOT_CONTAINER_SLOT_H

#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtrackable_node.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtrackable_edge.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"
#include "offbynull/utils.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container::slot {
    using offbynull::aligner::concepts::weight;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtrackable_node::backtrackable_node;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtrackable_edge::backtrackable_edge;

    // Because this struct is heavily templated, it's impossible to figure out how to organize member variables to reduce padding. One
    // option may be to ask the compiler to specifically pack the struct, but this may cause performance problems and/or bugs to happen
    // (depending on the architecture you're compiling for)?
    PACK_STRUCT_START
    /**
     * Element within
     * @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container::slot_container::slot_container. Each
     * slot is for a single node in the graph, tracking the
     * @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker::backtracker "backtracking algorithm's"
     * status for that node: Either the backtracking algorithm has parent nodes that need to be processed before it can process this node,
     * or the backtracking algorithm has processed all parent nodes and this node as well (holding on to the incoming edge with the highest
     * accumulated weight).
     *
     * Struct is packed when `OBN_PACK_STRUCTS` macro is defined (and platform supports struct packing).
     *
     * @tparam N Graph node identifier type.
     * @tparam PARENT_COUNT Graph node incoming edge counter type. Must be wide enough to hold the maximum number of incoming edges across
     *      all nodes in the underlying pairwise alignment graph instance (e.g., across all nodes in any global pairwise alignment graph, a
     *      node can have at most 3 incoming edges).
     * @tparam E Graph edge identifier type.
     * @tparam ED Graph edge data type (edge weight).
     */
    template<
        backtrackable_node N,
        backtrackable_edge E,
        weight ED,
        widenable_to_size_t PARENT_COUNT
    >
    struct slot {
        /** Node identifier this `slot` is assigned to. */
        N node;
        /** Number of `node` parents that have yet to be walked. */
        PARENT_COUNT unwalked_parent_cnt;
        /** Backtracking edge (value is valid only if `unwalked_parent_cnt == 0`). */
        E backtracking_edge;
        /** Backtracking weight (value is valid only if `unwalked_parent_cnt == 0`). */
        ED backtracking_weight;

        /**
         * Construct an @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container::slot::slot instance.
         *
         * @param node_ Identifier of node to be assigned to this slot.
         * @param unwalked_parent_cnt_ `node_`'s incoming edge count, assumed to all be unprocessed by the owning backtracker at time of
         *     creation.
         * @param backtracking_weight_ Initial backtracking weight, equivalent to 0 for numeric weights.
         */
        slot(
            N node_,
            PARENT_COUNT unwalked_parent_cnt_,
            ED backtracking_weight_
        )  // TODO: Should this change to const N& node_
        : node { node_ }
        , unwalked_parent_cnt { unwalked_parent_cnt_ }
        , backtracking_edge {}
        , backtracking_weight { backtracking_weight_ } {}
    }
    PACK_STRUCT_STOP;
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLOT_CONTAINER_SLOT_H
