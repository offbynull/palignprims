#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKER_HEAP_CONTAINER_CREATOR_PACK_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKER_HEAP_CONTAINER_CREATOR_PACK_H

#include <cstddef>
#include <vector>
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtrackable_node.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtrackable_edge.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/unimplemented_backtracker_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/ready_queue/ready_queue_heap_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/slot_container/slot_container_heap_container_creator_pack.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker_heap_container_creator_pack {
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtrackable_node::backtrackable_node;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtrackable_edge::backtrackable_edge;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container
        ::slot_container_heap_container_creator_pack::slot_container_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::ready_queue::ready_queue_heap_container_creator_pack
        ::ready_queue_heap_container_creator_pack;
    using offbynull::concepts::widenable_to_size_t;

    /**
     * @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker_container_creator_pack::backtracker_container_creator_pack
     * that allocates its containers on the heap.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam N Graph node type.
     * @tparam E Graph edge type.
     * @tparam ED Graph edge data type (edge weight).
     * @tparam PARENT_COUNT Graph node incoming edge counter type. Must be wide enough to hold the maximum number of incoming edges across
     *     all nodes in the underlying pairwise alignment graph instance (e.g., across all nodes in any global pairwise alignment graph, a
     *     node can have at most 3 incoming edges).
     * @tparam SLOT_INDEX Slot indexer type. Must be wide enough to hold the value `grid_down_cnt * grid_right_cnt * grid_depth_cnt`
     *     (variables being multiplied are the dimensions of the underlying graph).
     * @tparam minimize_allocations `true` to force `create_ready_queue_container_creator_pack()` to create
     *     @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::ready_queue::ready_queue::ready_queue` that's primed
     *     with `grid_down_cnt * grid_right_cnt * grid_depth_cnt` reserved elements (variables being multiplied are the dimensions of the
     *     underlying pairwise alignment graph instance), thereby removing/reducing the need for adhoc reallocations.
     */
    template<
        bool debug_mode,
        backtrackable_node N,
        backtrackable_edge E,
        weight ED,
        widenable_to_size_t PARENT_COUNT,
        widenable_to_size_t SLOT_INDEX,
        bool minimize_allocations
    >
    struct backtracker_heap_container_creator_pack {
        /**
         * @copydoc offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::unimplemented_backtracker_container_creator_pack::unimplemented_backtracker_container_creator_pack::create_slot_container_container_creator_pack
         */
        slot_container_heap_container_creator_pack<
            debug_mode,
            N,
            E,
            ED,
            PARENT_COUNT
        > create_slot_container_container_creator_pack() const {
            return {};
        }

        /**
         * @copydoc offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::unimplemented_backtracker_container_creator_pack::unimplemented_backtracker_container_creator_pack::create_ready_queue_container_creator_pack
         */
        ready_queue_heap_container_creator_pack<
            debug_mode,
            SLOT_INDEX,
            minimize_allocations
        > create_ready_queue_container_creator_pack() const {
            return {};
        }

        /**
         * @copydoc offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::unimplemented_backtracker_container_creator_pack::unimplemented_backtracker_container_creator_pack::create_path_container
         */
        std::vector<E> create_path_container(std::size_t path_edge_capacity) const {
            std::vector<E> ret {};
            if constexpr (minimize_allocations) {
                ret.reserve(path_edge_capacity);
            }
            return ret;
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKER_HEAP_CONTAINER_CREATOR_PACK_H
