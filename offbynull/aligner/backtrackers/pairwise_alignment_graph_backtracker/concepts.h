#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_CONCEPTS_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_CONCEPTS_H

#include <concepts>
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::concepts {
    using offbynull::concepts::unqualified_value_type;



    template<typename N>
    concept backtrackable_node = unqualified_value_type<N> && std::regular<N>;

    template<typename E>
    concept backtrackable_edge = unqualified_value_type<E> && std::regular<E>;
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_CONCEPTS_H
