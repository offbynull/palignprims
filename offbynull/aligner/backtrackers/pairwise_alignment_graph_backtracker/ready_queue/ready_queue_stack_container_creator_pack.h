#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_READY_QUEUE_READY_QUEUE_STACK_CONTAINER_CREATOR_PACK_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_READY_QUEUE_READY_QUEUE_STACK_CONTAINER_CREATOR_PACK_H

#include <cstddef>
#include <stdexcept>
#include "offbynull/utils.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/ready_queue/ready_queue_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/ready_queue/unimplemented_ready_queue_container_creator_pack.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::ready_queue::ready_queue_stack_container_creator_pack {
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::utils::static_vector_typer;
    using offbynull::utils::check_multiplication_nonoverflow_throwable;

    /**
     * @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::ready_queue::ready_queue_container_creator_pack::ready_queue_container_creator_pack
     * that allocates its containers on the stack.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam SLOT_INDEX Slot indexer type. Must be wide enough to hold the value `grid_down_cnt * grid_right_cnt * grid_depth_cnt`
     *     (variables being multiplied are the dimensions of the underlying pairwise alignment graph instance).
     * @tparam grid_down_cnt Expected down dimension of the underlying pairwise alignment graph instance.
     * @tparam grid_right_cnt Expected right dimension of the underlying pairwise alignment graph instance.
     * @tparam grid_depth_cnt Expected depth dimension of the underlying pairwise alignment graph instance.
     */
    template<
        bool debug_mode,
        widenable_to_size_t SLOT_INDEX,
        std::size_t grid_down_cnt,
        std::size_t grid_right_cnt,
        std::size_t grid_depth_cnt
    >
    struct ready_queue_stack_container_creator_pack {
    private:
        static constexpr std::size_t ELEM_COUNT { grid_down_cnt * grid_right_cnt * grid_depth_cnt };

    public:
        /** `create_queue_container()` return type. */
        using CONTAINER_TYPE = typename static_vector_typer<debug_mode, SLOT_INDEX, ELEM_COUNT>::type;

        /**
         * @copydoc offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::ready_queue::unimplemented_ready_queue_container_creator_pack::unimplemented_ready_queue_container_creator_pack::create_queue_container
         */
        CONTAINER_TYPE create_queue_container(
            std::size_t grid_down_cnt_,
            std::size_t grid_right_cnt_,
            std::size_t grid_depth_cnt_
        ) const {
            if constexpr (debug_mode) {
                check_multiplication_nonoverflow_throwable<std::size_t>(grid_down_cnt, grid_right_cnt, grid_depth_cnt);
                if (grid_down_cnt_ > grid_down_cnt || grid_right_cnt_ > grid_right_cnt || grid_depth_cnt_ > grid_depth_cnt) {
                    throw std::runtime_error { "Count mismatch" };
                }
            }
            return {};
        }
    };
}
#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_READY_QUEUE_READY_QUEUE_STACK_CONTAINER_CREATOR_PACK_H
