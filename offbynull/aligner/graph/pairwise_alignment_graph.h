#ifndef OFFBYNULL_ALIGNER_GRAPH_PAIRWISE_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPH_PAIRWISE_ALIGNMENT_GRAPH_H

#include <cstddef>
#include <concepts>
#include <type_traits>
#include <utility>
#include <functional>
#include "offbynull/concepts.h"
#include "offbynull/aligner/graph/graph.h"

namespace offbynull::aligner::graph::pairwise_alignment_graph {
    using offbynull::concepts::one_of;
    using offbynull::concepts::range_of_one_of;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::aligner::graph::graph::readable_graph;

    // You can use unimplemented types as requires params -- the compiler will check to see if it has the same traits
    struct unimplemented_sequence {
        std::size_t size() const;
        const std::tuple<>& operator[](const std::size_t idx) const;  // Falsely returning std::tuple<>, when really this would be whatever internal type this thing returns
    };

    template <typename G>
    concept readable_parwise_alignment_graph =
        readable_graph<G>
        && widenable_to_size_t<typename G::INDEX>
        && requires(
            G g,
            unimplemented_sequence v,
            unimplemented_sequence w,
            std::function<
                double(
                    const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(v[0u])>>>&,
                    const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(w[0u])>>>&
                )
            > weight_lookup,
            std::function<void(typename G::ED&, double weight)> weight_setter
        ) {
            { g.assign_weights(v, w, weight_lookup, weight_setter) } -> std::same_as<void>;
        }
        && requires(
            G g,
            typename G::E e,
            unimplemented_sequence v,
            unimplemented_sequence w
        ) {
            { G::edge_to_elements(e, v, w) } -> std::same_as<
                std::optional<
                    std::pair<
                        std::optional<std::reference_wrapper<const std::decay_t<decltype(v[0])>>>,
                        std::optional<std::reference_wrapper<const std::decay_t<decltype(w[0])>>>
                    >
                >
            >;
        }
        && requires(
            G g,
            typename G::INDEX indexer
        ) {
            { G::node_count(indexer, indexer) } -> std::same_as<typename G::INDEX>;
            { G::edge_count(indexer, indexer) } -> std::same_as<typename G::INDEX>;
            { G::longest_path_edge_count(indexer, indexer) } -> std::same_as<typename G::INDEX>;
        };
}

#endif //OFFBYNULL_ALIGNER_GRAPH_PAIRWISE_ALIGNMENT_GRAPH_H
