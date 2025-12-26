#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLOT_CONTAINER_SLOT_CONTAINER_HEAP_CONTAINER_CREATOR_PACK_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLOT_CONTAINER_SLOT_CONTAINER_HEAP_CONTAINER_CREATOR_PACK_H

#include <cstddef>
#include <vector>
#include <limits>
#include <stdexcept>
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtrackable_node.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtrackable_edge.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/slot_container/slot.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/slot_container/unimplemented_slot_container_container_creator_pack.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/concepts.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container
        ::slot_container_heap_container_creator_pack {
    using offbynull::aligner::concepts::weight;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtrackable_node::backtrackable_node;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtrackable_edge::backtrackable_edge;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container::slot::slot;
    using offbynull::utils::check_multiplication_nonoverflow_throwable;

    /**
     * @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container::slot_container_container_creator_pack::slot_container_container_creator_pack
     * that allocates its containers on the heap.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam N Graph node identifier type.
     * @tparam E Graph edge identifier type.
     * @tparam ED Graph edge data type (edge weight).
     * @tparam PARENT_COUNT Graph node incoming edge counter type. Must be wide enough to hold the maximum number of incoming edges across
     *      all nodes in the underlying pairwise alignment graph instance (e.g., across all nodes in any global pairwise alignment graph, a
     *      node can have at most 3 incoming edges).
     * @tparam SLOT_INDEX Slot indexer type. Must be wide enough to hold the value `grid_down_cnt * grid_right_cnt * grid_depth_cnt`
     *     (variables being multiplied are the dimensions of the underlying graph).
     */
    template<
        bool debug_mode,
        backtrackable_node N,
        backtrackable_edge E,
        weight ED,
        widenable_to_size_t PARENT_COUNT,
        widenable_to_size_t SLOT_INDEX
    >
    struct slot_container_heap_container_creator_pack {
        /**
         * @copydoc offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container::unimplemented_slot_container_container_creator_pack::unimplemented_slot_container_container_creator_pack::create_slot_container
         */
        std::vector<slot<N, E, ED, PARENT_COUNT>> create_slot_container(
            std::size_t grid_down_cnt,
            std::size_t grid_right_cnt,
            std::size_t grid_depth_cnt
        ) const {
            if constexpr (debug_mode) {
                check_multiplication_nonoverflow_throwable<std::size_t>(grid_down_cnt, grid_right_cnt, grid_depth_cnt);
            }
            std::size_t cnt { (grid_down_cnt * grid_right_cnt) * grid_depth_cnt };
            if constexpr (debug_mode) {
                if (std::numeric_limits<SLOT_INDEX>::max() < cnt) {
                    throw std::runtime_error { "SLOT_INDEX not wide enough to support grid_down_cnt * grid_right_cnt * grid_depth_cnt" };
                }
            }
            return std::vector<slot<N, E, ED, PARENT_COUNT>>(cnt);
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLOT_CONTAINER_SLOT_CONTAINER_HEAP_CONTAINER_CREATOR_PACK_H
