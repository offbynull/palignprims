#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKABLE_NODE_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKABLE_NODE_H

#include <concepts>
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtrackable_node {
    using offbynull::concepts::unqualified_object_type;

    /**
     * Concept that's satisfied if `N` has the traits of graph node identifier (as expected by the
     * @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker::backtracker "backtracking algorithm") and
     * is an unqualified object type.
     *
     * @tparam N Type to check.
     */
    template<typename N>
    concept backtrackable_node = unqualified_object_type<N> && std::regular<N>;
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKABLE_NODE_H
