#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_ROW_SLOT_CONTAINER_ROW_SLOT_CONTAINER_HEAP_CONTAINER_CREATOR_PACK_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_ROW_SLOT_CONTAINER_ROW_SLOT_CONTAINER_HEAP_CONTAINER_CREATOR_PACK_H

#include <vector>
#include <cstddef>
#include <ranges>
#include <optional>
#include "offbynull/aligner/concepts.h"
#include "offbynull/utils.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/row_slot_container/slot.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtrackable_edge.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/row_slot_container/unimplemented_row_slot_container_container_creator_pack.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container
        ::row_slot_container_heap_container_creator_pack {
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container::slot::slot;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtrackable_edge::backtrackable_edge;
    using offbynull::aligner::concepts::weight;
    using offbynull::utils::copy_to_vector;

    /**
     * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container::row_slot_container_container_creator_pack::row_slot_container_container_creator_pack
     * that allocates its containers on the heap.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam E Graph edge identifier type.
     * @tparam ED Graph edge data type (edge weight).
     */
    template<
        bool debug_mode,
        backtrackable_edge E,
        weight ED
    >
    struct row_slot_container_heap_container_creator_pack {
        /**
         * @copydoc offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container::unimplemented_row_slot_container_container_creator_pack::unimplemented_row_slot_container_container_creator_pack
         */
        std::vector<slot<E, ED>> create_slot_container(std::size_t grid_right_cnt, std::size_t grid_depth_cnt, ED zero_weight) const {
            std::size_t cnt { grid_right_cnt * grid_depth_cnt };
            return copy_to_vector(
                std::views::iota(0zu, cnt)
                | std::views::transform([=](const auto&) { return slot<E, ED> { std::nullopt, zero_weight }; })
            );
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_ROW_SLOT_CONTAINER_ROW_SLOT_CONTAINER_HEAP_CONTAINER_CREATOR_PACK_H
