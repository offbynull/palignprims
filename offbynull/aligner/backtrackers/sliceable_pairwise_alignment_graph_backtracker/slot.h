#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLOT_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLOT_H

#include <ranges>
#include "offbynull/aligner/concepts.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::slot {
    using offbynull::aligner::concepts::weight;

    template<typename E, weight WEIGHT>
    struct slot {
        std::optional<E> backtracking_edge;
        WEIGHT backtracking_weight;

        slot(E backtracking_edge_, WEIGHT backtracking_weight_)
        : backtracking_edge { backtracking_edge_ }
        , backtracking_weight { backtracking_weight_ } {}

        slot()
        : backtracking_edge { std::nullopt }
        , backtracking_weight {} {}

        bool operator==(const slot&) const = default;
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLOT_H
