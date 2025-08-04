#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLOT_CONTAINER_SLOT_CONTAINER_STACK_CONTAINER_CREATOR_PACK_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLOT_CONTAINER_SLOT_CONTAINER_STACK_CONTAINER_CREATOR_PACK_H

#include <cstddef>
#include <array>
#include <stdexcept>
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtrackable_node.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtrackable_edge.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/slot_container/slot.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container
        ::slot_container_stack_container_creator_pack {
    using offbynull::aligner::concepts::weight;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtrackable_node::backtrackable_node;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtrackable_edge::backtrackable_edge;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container::slot::slot;

    /**
     * @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container::slot_container_container_creator_pack::slot_container_container_creator_pack
     * that allocates its containers on the stack.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam N Graph node type.
     * @tparam E Graph edge type.
     * @tparam ED Graph edge data type (edge weight).
     * @tparam PARENT_COUNT Graph node incoming edge counter type. Must be wide enough to hold the maximum number of incoming edges across
     *      all nodes in the underlying pairwise alignment graph instance (e.g., across all nodes in any global pairwise alignment graph, a
     *      node can have at most 3 incoming edges).
     * @tparam grid_down_cnt Expected down dimension of the underlying pairwise alignment graph instance.
     * @tparam grid_right_cnt Expected right dimension of the underlying pairwise alignment graph instance.
     * @tparam grid_depth_cnt Expected depth dimension of the underlying pairwise alignment graph instance.
     */
    template<
        bool debug_mode,
        backtrackable_node N,
        backtrackable_edge E,
        weight ED,
        widenable_to_size_t PARENT_COUNT,
        std::size_t grid_down_cnt,
        std::size_t grid_right_cnt,
        std::size_t grid_depth_cnt
    >
    struct slot_container_stack_container_creator_pack {
    private:
        static constexpr std::size_t ELEM_COUNT { grid_down_cnt * grid_right_cnt * grid_depth_cnt };

    public:
        /**
         * @copydoc offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container::unimplemented_slot_container_container_creator_pack::unimplemented_slot_container_container_creator_pack::create_slot_container
         */
        std::array<slot<N, E, ED, PARENT_COUNT>, ELEM_COUNT> create_slot_container(
            std::size_t grid_down_cnt_,
            std::size_t grid_right_cnt_,
            std::size_t grid_depth_cnt_
        ) const {
            if constexpr (debug_mode) {
                std::size_t cnt { (grid_down_cnt_ * grid_right_cnt_) * grid_depth_cnt_ };
                if (cnt != ELEM_COUNT) {
                    throw std::runtime_error { "Bad element count" };
                }
            }
            return std::array<slot<N, E, ED, PARENT_COUNT>, ELEM_COUNT> {};
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLOT_CONTAINER_SLOT_CONTAINER_STACK_CONTAINER_CREATOR_PACK_H
