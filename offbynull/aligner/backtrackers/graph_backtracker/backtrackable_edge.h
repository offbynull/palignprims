#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_BACKTRACKABLE_EDGE_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_BACKTRACKABLE_EDGE_H

#include <concepts>
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::graph_backtracker::backtrackable_edge {
    using offbynull::concepts::unqualified_object_type;

    /**
     * Concept that's satisfied if `E` has the traits of graph edge (as expected by the
     * @link offbynull::aligner:backtrackers::graph_backtracker::backtracker::backtracker backtracking algorithm @endlink) and is an
     * unqualified object type.
     *
     * @tparam E Type to check.
     */
    template<typename E>
    concept backtrackable_edge =
        unqualified_object_type<E>
        && std::regular<E>;
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_BACKTRACKABLE_EDGE_H
