#ifndef OFFBYNULL_ALIGNER_GRAPH_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPH_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H

#include "offbynull/concepts.h"

namespace offbynull::aligner::graph::sliceable_pairwise_alignment_graph {
    using offbynull::concepts::one_of;
    using offbynull::concepts::range_of_type;
    using offbynull::concepts::range_of_one_of;
    using offbynull::concepts::widenable_to_size_t;

    template <typename G>
    concept readable_sliceable_pairwise_alignment_graph =
        requires(
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
