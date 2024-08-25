#ifndef OFFBYNULL_ALIGNER_GRAPH_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPH_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H

#include <cstddef>
#include <concepts>
#include "offbynull/concepts.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"

namespace offbynull::aligner::graph::sliceable_pairwise_alignment_graph {
    using offbynull::concepts::range_of_one_of;
    using offbynull::concepts::unqualified_value_type;
    using offbynull::aligner::graph::pairwise_alignment_graph::readable_pairwise_alignment_graph;

    template <typename G>
    concept readable_sliceable_pairwise_alignment_graph =
        unqualified_value_type<G>
        && readable_pairwise_alignment_graph<G>
        && requires(
            G g,
            typename G::N node,
            typename G::INDEX grid_down,
            typename G::INDEX grid_right,
            typename G::INDEX override_grid_right_cnt
        ) {
            { g.resident_nodes_capacity } -> std::same_as<const std::size_t&>;
            { g.slice_nodes(grid_down) } -> range_of_one_of<typename G::N, const typename G::N&>;
            { g.slice_nodes(grid_down, node, node) } -> range_of_one_of<typename G::N, const typename G::N&>;
            { g.is_reachable(node, node) } -> std::same_as<bool>;
            { g.resident_nodes() } -> range_of_one_of<typename G::N, const typename G::N&>;
            { g.outputs_to_residents(node) } -> range_of_one_of<typename G::E, const typename G::E&>;  // children in slice
            { g.inputs_from_residents(node) } -> range_of_one_of<typename G::E, const typename G::E&>;  // children in slice
        };
}

#endif //OFFBYNULL_ALIGNER_GRAPH_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
