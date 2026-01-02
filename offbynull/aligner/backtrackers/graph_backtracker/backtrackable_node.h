#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_BACKTRACKABLE_NODE_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_BACKTRACKABLE_NODE_H

#include <concepts>
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::graph_backtracker::backtrackable_node {
    using offbynull::concepts::unqualified_object_type;

    /**
     * Concept that's satisfied if `N` has the traits of graph node identifier (as expected by the
     * @ref offbynull::aligner::backtrackers::graph_backtracker::backtracker::backtracker "backtracking algorithm") and is an unqualified
     * object type.
     *
     * @tparam N Type to check.
     */
    template<typename N>
    concept backtrackable_node =
        unqualified_object_type<N>
        && std::regular<N>
        && requires(N n) {
            { n < n } -> std::same_as<bool>;  // Needed for sorting within slot_container - binary search
        };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_BACKTRACKABLE_NODE_H
