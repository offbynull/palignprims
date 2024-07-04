#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_CONCEPTS_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_CONCEPTS_H

#include <concepts>

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::concepts {
    template<typename N>
    concept backtrackable_node =
        std::regular<N> &&
        requires(N n) {
            {n < n} -> std::same_as<bool>;
        };

    template<typename E>
    concept backtrackable_edge = std::regular<E>;
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_CONCEPTS_H
