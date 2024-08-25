#ifndef OFFBYNULL_ALIGNER_GRAPH_PAIRWISE_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPH_PAIRWISE_ALIGNMENT_GRAPH_H

#include <cstddef>
#include <concepts>
#include <utility>
#include <tuple>
#include <optional>
#include "offbynull/concepts.h"
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/concepts.h"

namespace offbynull::aligner::graph::pairwise_alignment_graph {
    using offbynull::concepts::one_of;
    using offbynull::concepts::range_of_one_of;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::concepts::unqualified_value_type;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::graph::graph::readable_graph;

    template <typename G>
    concept readable_pairwise_alignment_graph =
        unqualified_value_type<G>
        && readable_graph<G>
        && widenable_to_size_t<typename G::INDEX>
        && weight<typename G::ED>
        && requires(
            G g,
            typename G::N node,
            typename G::E edge,
            typename G::INDEX indexer
        ) {
            { g.grid_down_cnt } -> std::same_as<const typename G::INDEX&>;
            { g.grid_right_cnt } -> std::same_as<const typename G::INDEX&>;
            { g.grid_depth_cnt } -> std::same_as<const typename G::INDEX&>;
            { g.path_edge_capacity } -> std::same_as<const std::size_t&>;
            { g.node_to_grid_offsets(node) } -> std::same_as<
                std::tuple<
                    typename G::INDEX,
                    typename G::INDEX,
                    std::size_t
                >
            >;
            { g.edge_to_element_offsets(edge) } -> std::same_as<
                std::optional<
                    std::pair<
                        std::optional<typename G::INDEX>,
                        std::optional<typename G::INDEX>
                    >
                >
            >;
        };
}

#endif //OFFBYNULL_ALIGNER_GRAPH_PAIRWISE_ALIGNMENT_GRAPH_H
