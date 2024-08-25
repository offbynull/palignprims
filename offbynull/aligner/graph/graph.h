#ifndef OFFBYNULL_ALIGNER_GRAPH_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPH_GRAPH_H

#include <cstddef>
#include <tuple>
#include <concepts>
#include "offbynull/concepts.h"

namespace offbynull::aligner::graph::graph {
    using offbynull::concepts::one_of;
    using offbynull::concepts::range_of_one_of;
    using offbynull::concepts::unqualified_value_type;

    template<typename N>
    concept node = unqualified_value_type<N> && std::regular<N>;

    template<typename E>
    concept edge = unqualified_value_type<E> && std::regular<E>;

    template <typename G>
    concept readable_graph =
        unqualified_value_type<G>
        && requires(G g, typename G::N n, typename G::E e) {
            typename G::N;
            typename G::ND;
            typename G::E;
            typename G::ED;
            { typename G::N {} } -> node;
            { typename G::E {} } -> edge;
            { g.get_node_data(n) } -> one_of<typename G::ND, const typename G::ND&>;
            { g.get_edge_data(e) } -> one_of<typename G::ED, const typename G::ED&>;
            { g.get_edge_from(e) } -> one_of<typename G::N, const typename G::N&>;
            { g.get_edge_to(e) } -> one_of<typename G::N, const typename G::N&>;
            { g.get_edge(e) } -> one_of<
                std::tuple<typename G::N, typename G::N, typename G::ED>,
                std::tuple<typename G::N, typename G::N, const typename G::ED&>,
                std::tuple<const typename G::N&, const typename G::N&, typename G::ED>,
                std::tuple<const typename G::N&, const typename G::N&, const typename G::ED&>
            >;
            { g.get_root_nodes() } -> range_of_one_of<typename G::N, const typename G::N&>;
            { g.get_root_node() } -> one_of<typename G::N, const typename G::N&>;
            { g.get_leaf_nodes() } -> range_of_one_of<typename G::N, const typename G::N&>;
            { g.get_leaf_node() } -> one_of<typename G::N, const typename G::N&>;
            { g.get_nodes() } -> range_of_one_of<typename G::N, const typename G::N&>;
            { g.get_edges() } -> range_of_one_of<typename G::E, const typename G::E&>;
            { g.has_node(n) } -> std::same_as<bool>;
            { g.has_edge(e) } -> std::same_as<bool>;
            { g.get_outputs_full(n) } -> range_of_one_of<
                std::tuple<typename G::E, typename G::N, typename G::N, typename G::ED>,
                std::tuple<typename G::E, typename G::N, typename G::N, const typename G::ED&>,
                std::tuple<const typename G::E&, const typename G::N&, const typename G::N&, typename G::ED>,
                std::tuple<const typename G::E&, const typename G::N&, const typename G::N&, const typename G::ED&>
            >;
            { g.get_inputs_full(n) } -> range_of_one_of<
                std::tuple<typename G::E, typename G::N, typename G::N, typename G::ED>,
                std::tuple<typename G::E, typename G::N, typename G::N, const typename G::ED&>,
                std::tuple<const typename G::E&, const typename G::N&, const typename G::N&, typename G::ED>,
                std::tuple<const typename G::E&, const typename G::N&, const typename G::N&, const typename G::ED&>
            >;
            { g.get_outputs(n) } -> range_of_one_of<typename G::E, const typename G::E&>;
            { g.get_inputs(n) } -> range_of_one_of<typename G::E, const typename G::E&>;
            { g.has_outputs(n) } -> std::same_as<bool>;
            { g.has_inputs(n) } -> std::same_as<bool>;
            { g.get_out_degree(n) } -> std::same_as<std::size_t>;
            { g.get_in_degree(n) } -> std::same_as<std::size_t>;
        };
}

#endif //OFFBYNULL_ALIGNER_GRAPH_GRAPH_H
