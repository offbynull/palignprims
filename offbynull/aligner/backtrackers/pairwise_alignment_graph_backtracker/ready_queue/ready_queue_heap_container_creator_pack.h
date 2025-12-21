#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_READY_QUEUE_READY_QUEUE_HEAP_CONTAINER_CREATOR_PACK_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_READY_QUEUE_READY_QUEUE_HEAP_CONTAINER_CREATOR_PACK_H

#include <cstddef>
#include <vector>
#include "offbynull/concepts.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/ready_queue/ready_queue_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/ready_queue/unimplemented_ready_queue_container_creator_pack.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::ready_queue::ready_queue_heap_container_creator_pack {
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::utils::check_multiplication_nonoverflow;

    /**
     * @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::ready_queue::ready_queue_container_creator_pack::ready_queue_container_creator_pack
     * that allocates its containers on the heap.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam SLOT_INDEX Slot indexer type. Must be wide enough to hold the value `grid_down_cnt * grid_right_cnt * grid_depth_cnt`
     *     (variables being multiplied are the dimensions of the underlying pairwise alignment graph instance).
     * @tparam minimize_allocations `true` to force `create_queue_container()` to return container with
     *     `grid_down_cnt * grid_right_cnt * grid_depth_cnt` reserved elements (variables being multiplied are the dimensions of the
     *     underlying pairwise alignment graph instance), thereby removing/reducing the need for adhoc reallocations.
     */
    template<
        bool debug_mode,
        widenable_to_size_t SLOT_INDEX,
        bool minimize_allocations
    >
    struct ready_queue_heap_container_creator_pack {
        /**
         * @copydoc offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::ready_queue::unimplemented_ready_queue_container_creator_pack::unimplemented_ready_queue_container_creator_pack::create_queue_container
         */
        std::vector<SLOT_INDEX> create_queue_container(
            std::size_t grid_down_cnt,
            std::size_t grid_right_cnt,
            std::size_t grid_depth_cnt
        ) const {
            if constexpr (debug_mode) {
                check_multiplication_nonoverflow<std::size_t>(grid_down_cnt, grid_right_cnt, grid_depth_cnt);
            }
            std::vector<SLOT_INDEX> ret {};
            if constexpr (minimize_allocations) {
                ret.reserve(grid_down_cnt * grid_right_cnt * grid_depth_cnt);
            }
            return ret;
        }
    };
}
#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_READY_QUEUE_READY_QUEUE_HEAP_CONTAINER_CREATOR_PACK_H
