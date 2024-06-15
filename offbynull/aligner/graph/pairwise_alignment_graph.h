#ifndef OFFBYNULL_ALIGNER_GRAPH_PAIRWISE_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPH_PAIRWISE_ALIGNMENT_GRAPH_H

#include <concepts>
#include <utility>
#include <functional>
#include "offbynull/concepts.h"
#include "offbynull/aligner/graph/graph.h"

namespace offbynull::aligner::graph::pairwise_alignment_graph {
    using offbynull::concepts::one_of;
    using offbynull::concepts::range_of_one_of;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::aligner::graph::graph::readable_graph;

    template <typename G>
    concept readable_parwise_alignment_graph =
        readable_graph<G>
        && widenable_to_size_t<typename G::INDEX>
        && requires(
            G g,
            typename G::N node,
            typename G::E edge
        ) {
            { g.node_to_grid_offsets(node) } -> std::same_as<std::pair<typename G::INDEX, typename G::INDEX>>;
            { g.edge_to_element_offsets(g.edge) } -> std::same_as<
                std::optional<
                    std::pair<
                        std::optional<typename G::INDEX>,
                        std::optional<typename G::INDEX>
                    >
                >
            >;
        }
        && requires(
            G g,
            typename G::INDEX indexer
        ) {
            { g.down_node_cnt } -> std::same_as<const typename G::INDEX&>;
            { g.right_node_cnt } -> std::same_as<const typename G::INDEX&>;
            { G::node_count(indexer, indexer) } -> std::same_as<typename G::INDEX>;
            { G::longest_path_edge_count(indexer, indexer) } -> std::same_as<typename G::INDEX>;
        };
}

#endif //OFFBYNULL_ALIGNER_GRAPH_PAIRWISE_ALIGNMENT_GRAPH_H
