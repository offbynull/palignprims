#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_ROW_SLOT_CONTAINER_ROW_SLOT_CONTAINER_STACK_CONTAINER_CREATOR_PACK_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_ROW_SLOT_CONTAINER_ROW_SLOT_CONTAINER_STACK_CONTAINER_CREATOR_PACK_H

#include <cstddef>
#include <stdexcept>
#include "offbynull/utils.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/row_slot_container/slot.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtrackable_edge.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/row_slot_container/unimplemented_row_slot_container_container_creator_pack.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container
        ::row_slot_container_stack_container_creator_pack {
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container::slot::slot;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtrackable_edge::backtrackable_edge;
    using offbynull::aligner::concepts::weight;
    using offbynull::utils::static_vector_typer;

    /**
     * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container::row_slot_container_container_creator_pack::row_slot_container_container_creator_pack
     * that allocates its containers on the stack.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam E Graph edge type.
     * @tparam ED Graph edge data type (edge weight).
     * @tparam grid_right_cnt Expected right dimension of the underlying sliceable pairwise alignment graph instance.
     * @tparam grid_depth_cnt Expected depth dimension of the underlying sliceable pairwise alignment graph instance.
     */
    template<
        bool debug_mode,
        backtrackable_edge E,
        weight ED,
        std::size_t grid_right_cnt,
        std::size_t grid_depth_cnt
    >
    struct row_slot_container_stack_container_creator_pack {
    private:
        static constexpr std::size_t max_elem_cnt { grid_right_cnt * grid_depth_cnt };

    public:
        /** `create_slot_container()` return type. */
        using SEGMENT_CONTAINER_TYPE = typename static_vector_typer<
            debug_mode,
            slot<E, ED>,
            max_elem_cnt
        >::type;

        /**
         * @copydoc offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container::unimplemented_row_slot_container_container_creator_pack::unimplemented_row_slot_container_container_creator_pack
         */
        SEGMENT_CONTAINER_TYPE create_slot_container(std::size_t grid_right_cnt_, std::size_t grid_depth_cnt_) const {
            std::size_t cnt { grid_right_cnt_ * grid_depth_cnt_ };
            if constexpr (debug_mode) {
                if (cnt > max_elem_cnt) {
                    throw std::runtime_error { "Bad element count" };
                }
            }
            return SEGMENT_CONTAINER_TYPE(cnt);
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_ROW_SLOT_CONTAINER_ROW_SLOT_CONTAINER_STACK_CONTAINER_CREATOR_PACK_H
