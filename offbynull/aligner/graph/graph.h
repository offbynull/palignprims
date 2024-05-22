#ifndef OFFBYNULL_ALIGNER_GRAPH_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPH_GRAPH_H

#include <concepts>
#include <iterator>
#include <ranges>

namespace offbynull::aligner::graph::graph {
    template<typename T, typename ... Ts>
    concept one_of = (std::same_as<T, Ts> || ...);

    template<typename T, typename... Ts>
    concept range_of_one_of = std::ranges::range<T> && (std::same_as<std::ranges::range_value_t<T>, Ts> || ...);

    template <typename G>
    concept readable_graph =
        requires(G g, G::N n, G::E e) {
            typename G::N;
            typename G::ND;
            typename G::E;
            typename G::ED;
            { g.get_node_data(n) } -> std::same_as<typename G::ND&>;
            { g.get_edge_data(e) } -> std::same_as<typename G::ED&>;
            { g.get_edge_from(e) } -> one_of<typename G::N, const typename G::N&>;
            { g.get_edge_to(e) } -> one_of<typename G::N, const typename G::N&>;
            { g.get_edge(e) } -> one_of<
                std::tuple<typename G::N, typename G::N, typename G::ED&>,
                std::tuple<const typename G::N&, const typename G::N&, typename G::ED&>
            >;
            { g.get_root_nodes() } -> range_of_one_of<typename G::N, const typename G::N&>;
            { g.get_root_node() } -> one_of<typename G::N, const typename G::N&>;
            { g.get_leaf_nodes() } -> range_of_one_of<typename G::N, const typename G::N&>;
            { g.get_nodes() } -> range_of_one_of<typename G::N, const typename G::N&>;
            { g.get_edges() } -> range_of_one_of<typename G::E, const typename G::E&>;
            { g.has_node(n) } -> std::same_as<bool>;
            { g.has_edge(e) } -> std::same_as<bool>;
            { g.get_outputs_full(n) } -> range_of_one_of<
                std::tuple<typename G::E, typename G::N, typename G::N, typename G::ED&>,
                std::tuple<const typename G::E&, const typename G::N&, const typename G::N&, typename G::ED&>
            >;
            { g.get_inputs_full(n) } -> range_of_one_of<
                std::tuple<typename G::E, typename G::N, typename G::N, typename G::ED&>,
                std::tuple<const typename G::E&, const typename G::N&, const typename G::N&, typename G::ED&>
            >;
            { g.get_output_full(n) } -> one_of<
                std::tuple<typename G::E, typename G::N, typename G::N, typename G::ED&>,
                std::tuple<const typename G::E&, const typename G::N&, const typename G::N&, typename G::ED&>
            >;
            { g.get_input_full(n) } -> one_of<
                std::tuple<typename G::E, typename G::N, typename G::N, typename G::ED&>,
                std::tuple<const typename G::E&, const typename G::N&, const typename G::N&, typename G::ED&>
            >;
            { g.get_outputs(n) } -> range_of_one_of<typename G::E, const typename G::E&>;
            { g.get_inputs(n) } -> range_of_one_of<typename G::E, const typename G::E&>;
            { g.get_output(n) } -> one_of<typename G::E, const typename G::E&>;
            { g.get_input(n) } -> one_of<typename G::E, const typename G::E&>;
            { g.has_outputs(n) } -> std::same_as<bool>;
            { g.has_inputs(n) } -> std::same_as<bool>;
            { g.get_out_degree(n) } -> std::same_as<size_t>;
            { g.get_in_degree(n) } -> std::same_as<size_t>;
        };
}

#endif //OFFBYNULL_ALIGNER_GRAPH_GRAPH_H
