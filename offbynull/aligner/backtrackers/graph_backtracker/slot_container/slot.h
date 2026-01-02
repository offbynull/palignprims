#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_SLOT_CONTAINER_SLOT_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_SLOT_CONTAINER_SLOT_H

#include <cstddef>
#include <variant>
#include "offbynull/aligner/backtrackers/graph_backtracker/backtrackable_node.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/backtrackable_edge.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::backtrackers::graph_backtracker::slot_container::slot {
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::backtrackers::graph_backtracker::backtrackable_node::backtrackable_node;
    using offbynull::aligner::backtrackers::graph_backtracker::backtrackable_edge::backtrackable_edge;

    // TODO: Switch the class below to use std::variant / std::holds_alternative() / std::visit()

    PACK_STRUCT_START
    /**
     * Element within @ref offbynull::aligner::backtrackers::graph_backtracker::slot_container::slot_container::slot_container. Each slot is
     * for a single node in the graph, tracking the
     * @ref offbynull::aligner::backtrackers::graph_backtracker::backtracker::backtracker "backtracking algorithm's" status for that node:
     * Either the backtracking algorithm has parent nodes that need to be processed before it can process this node, or the backtracking
     * algorithm has processed all parent nodes and this node as well (holding on to the incoming edge with the highest accumulated weight).
     *
     * Struct is packed when `OBN_PACK_STRUCTS` macro is defined (and platform supports struct packing).
     *
     * @tparam N Graph node identifier type.
     * @tparam E Graph edge identifier type.
     * @tparam WEIGHT Graph edge's weight type.
     */
    template<backtrackable_node N, backtrackable_edge E, weight WEIGHT>
    struct slot {
        /** Identifier of node assigned to this slot. */
        N node;
        /** Of all incoming edges into `node`, the number of parents left to be processed by the owning backtracker. */
        std::size_t unwalked_parent_cnt;
        /** Of all incoming edges into `node`, the one with the maximum cumulative weight. Value should be ignored unless
         * `unwalked_parent_cnt == 0`. */
        E backtracking_edge;
        /** `backtracking_edge`'s maximum cumulative weight. Value should be ignored unless `unwalked_parent_cnt == 0`. */
        WEIGHT backtracking_weight;

        /**
         * Construct an @ref offbynull::aligner::backtrackers::graph_backtracker::slot_container::slot::slot instance.
         *
         * @param node_ Identifier of node to be assigned to this slot.
         * @param unwalked_parent_cnt_ `node_`'s incoming edge count, assumed to all be unprocessed by the owning backtracker at time of
         *     creation.
         * @param backtracking_weight_ Initial backtracking weight, equivalent to 0 for numeric weights.
         */
        slot(
            N node_,
            std::size_t unwalked_parent_cnt_,
            WEIGHT backtracking_weight_
        )  // TODO: Should this change to const N& node_
        : node { node_ }
        , unwalked_parent_cnt { unwalked_parent_cnt_ }
        , backtracking_edge {}
        , backtracking_weight { backtracking_weight_ } {}
    }
    PACK_STRUCT_STOP;
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_SLOT_CONTAINER_SLOT_H
