#ifndef OFFBYNULL_ALIGNER_GRAPH_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPH_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H

#include "offbynull/concepts.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"

namespace offbynull::aligner::graph::sliceable_pairwise_alignment_graph {
    using offbynull::concepts::one_of;
    using offbynull::concepts::range_of_type;
    using offbynull::concepts::range_of_one_of;
    using offbynull::concepts::input_iterator_of_type;
    using offbynull::concepts::random_access_range_of_type;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::aligner::graph::pairwise_alignment_graph::readable_parwise_alignment_graph;

    template <typename G>
    concept readable_sliceable_parwise_alignment_graph =
        readable_parwise_alignment_graph<G>
        && requires(
            G g,
            typename G::N node,
            typename G::INDEX grid_down,
            typename G::INDEX grid_right,
            typename G::INDEX override_grid_right_cnt
        ) {
            { G::slice_nodes_capacity(grid_down, grid_right) } -> std::same_as<std::size_t>;
            { g.slice_nodes(grid_down) } -> range_of_one_of<typename G::N, const typename G::N&>;
            { g.slice_nodes(grid_down, override_grid_right_cnt) } -> range_of_one_of<typename G::N, const typename G::N&>;
            { g.slice_first_node(grid_down) } -> one_of<typename G::N, const typename G::N&>;  // first node to walk to
            { g.slice_first_node(grid_down, grid_right) } -> one_of<typename G::N, const typename G::N&>;  // first node to walk to
            { g.slice_last_node(grid_down) } -> one_of<typename G::N, const typename G::N&>;  // last node to walk to
            { g.slice_last_node(grid_down, grid_right) } -> one_of<typename G::N, const typename G::N&>;  // last node to walk to
            { g.slice_next_node(node) } -> one_of<typename G::N, const typename G::N&>;  // next node to walk to
            { g.slice_prev_node(node) } -> one_of<typename G::N, const typename G::N&>;  // prev node to walk to
            { G::resident_nodes_capacity(grid_down, grid_right) } -> std::same_as<std::size_t>;
            { g.resident_nodes() } -> range_of_one_of<typename G::N, const typename G::N&>;
            { g.outputs_to_residents(node) } -> range_of_one_of<typename G::E, const typename G::E&>;  // children in slice
            { g.inputs_from_residents(node) } -> range_of_one_of<typename G::E, const typename G::E&>;  // children in slice
        };
}

#endif //OFFBYNULL_ALIGNER_GRAPH_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
