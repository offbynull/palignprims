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
            typename G::INDEX n_down
        ) {
            { g.max_slice_nodes_count() } -> std::same_as<std::size_t>;
            { g.slice_nodes(n_down) } -> range_of_one_of<typename G::N, const typename G::N&>;
            { g.first_node_in_slice(n_down) } -> one_of<typename G::N, const typename G::N&>;  // first node to walk to
            { g.last_node_in_slice(n_down) } -> one_of<typename G::N, const typename G::N&>;  // last node to walk to
            { g.next_node_in_slice(node) } -> one_of<typename G::N, const typename G::N&>;  // next node to walk to
            { g.prev_node_in_slice(node) } -> one_of<typename G::N, const typename G::N&>;  // prev node to walk to
            { g.max_resident_nodes_count() } -> std::same_as<std::size_t>;
            { g.resident_nodes() } -> range_of_one_of<typename G::N, const typename G::N&>;
            { g.outputs_to_residents(node) } -> range_of_one_of<typename G::E, const typename G::E&>;  // children in slice
            { g.inputs_from_residents(node) } -> range_of_one_of<typename G::E, const typename G::E&>;  // children in slice
        };
}

#endif //OFFBYNULL_ALIGNER_GRAPH_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
