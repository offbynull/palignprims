#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_ROW_SLOT_CONTAINER_SLOT_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_ROW_SLOT_CONTAINER_SLOT_H

#include "offbynull/aligner/concepts.h"
#include "offbynull/utils.h"
#include <optional>

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container::slot {
    using offbynull::aligner::concepts::weight;
    using offbynull::utils::packable_optional;

    PACK_STRUCT_START
    /**
     * Element within
     * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container::row_slot_container::row_slot_container.
     * A slot tracks the
     * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtracker::backtracker "backtracking algorithm's"
     * status for some node, holding on to the incoming edge with the highest accumulated weight.
     *
     * Struct is packed when `OBN_PACK_STRUCTS` macro is defined (and platform supports struct packing).
     *
     * @tparam E Graph edge identifier type.
     * @tparam ED Graph edge data type (edge weight).
     */
    template<typename E, weight ED>
    struct slot {
        /** Identifier of backtracking edge. */
        packable_optional<E> backtracking_edge;
        /** Backtracking weight (value is valid only if `backtracking_edge` has a value). */
        ED backtracking_weight;

        /**
         * Construct an
         * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container::slot::slot instance.
         *
         * @param backtracking_edge_ Backtracking edge.
         * @param backtracking_weight_ Backtracking weight.
         */
        slot(E backtracking_edge_, ED backtracking_weight_)
        : backtracking_edge { backtracking_edge_ }
        , backtracking_weight { backtracking_weight_ } {}

        /**
         * Construct an
         * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container::slot::slot instance
         * where edge doesn't exist (equivalent ot default initialization).
         *
         * @param initial_backtracking_weight_ Initial weight, equivalent to 0 for numeric weights.
         */
        slot(std::nullopt_t, ED initial_backtracking_weight_)
        : backtracking_edge { std::nullopt }
        , backtracking_weight { initial_backtracking_weight_ } {}

        bool operator==(const slot&) const = default;
    }
    PACK_STRUCT_STOP;
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_ROW_SLOT_CONTAINER_SLOT_H
