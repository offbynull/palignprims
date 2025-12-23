#ifndef OFFBYNULL_ALIGNER_GRAPH_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPH_GRAPH_H

#include <cstddef>
#include <tuple>
#include <concepts>
#include <ranges>
#include <type_traits>
#include "offbynull/concepts.h"

namespace offbynull::aligner::graph::graph {
    using offbynull::concepts::bidirectional_range_of_non_cvref;
    using offbynull::concepts::unqualified_object_type;
    using offbynull::concepts::same_as_non_cvref;
    using offbynull::concepts::tuple_with_3_non_cvref_children;
    using offbynull::concepts::tuple_with_4_non_cvref_children;

    /**
     * Concept that's satisfied if `N` is an object type (e.g. not a reference, not CV-qualified) and has the traits of an
     * @ref offbynull::aligner::graph::graph::graph's node:
     *
     *  * Must be copyable / moveable.
     *  * Must be comparable using equality and inequality (`==` and `!=` operators).
     *
     * @tparam N Type to check (non-cvref).
     */
    template<typename N>
    concept node = unqualified_object_type<N> && std::copyable<N> && std::equality_comparable<N>;

    /**
     * Concept that's satisfied if `E` is an object type (not a reference, not CV-qualified) and has the traits of an
     * @ref offbynull::aligner::graph::graph::graph's edge:
     *
     *  * Must be copyable / moveable.
     *  * Must be comparable using equality and inequality (`==` and `!=` operators).
     *
     * @tparam E Type to check (non-cvref).
     */
    template<typename E>
    concept edge = unqualified_object_type<E> && std::copyable<E> && std::equality_comparable<E>;

    /**
     * Concept that's satisfied if `T` is a range (not a reference, not CV-qualified) of tuples, where each tuple contains the edge, from
     * node, to node, and edge data (`tuple<E, N, N, ED>`). Const, volatile, and references are removed from each item before checking
     * types.
     *
     * @tparam T Type of check (non-cvref).
     * @tparam N Node type (non-cvref).
     * @tparam E Edge type (non-cvref).
     * @tparam ED Edge data type (non-cvref).
     */
    template<typename T, typename N, typename E, typename ED>
    concept full_input_output_range = std::ranges::bidirectional_range<T>
        && node<N>
        && edge<E>
        && unqualified_object_type<ED>
        && tuple_with_4_non_cvref_children<
            std::remove_cvref_t<std::ranges::range_reference_t<T>>,
            E,
            N,
            N,
            ED
        >;

    /**
     * Concept that's satisfied if `G` is an object type (e.g. not a reference, not CV-qualified) and has the traits of a directed graph. A
     * directed graph consists of a set of nodes and directed edges, where each node / edge can have a some piece of data associated with it
     * (e.g. an edge weight).
     *
     * `G` implementations have the option of exposing node / edge data in one of two ways:
     *
     *  1. If data being exposed belongs to some object (e.g. the `G` instance itself), a const references to the data is returned and the
     *     data's lifetime must be treated as if being bound to the lifetime of the `G` instance.
     *  2. If data being exposed is generated on-the-fly when accessed, a copy of the data is returned.
     *
     * `G` must implement several members (see @ref offbynull::aligner::graph::graph::unimplemented_graph). Given a background in graph
     * theory, most of these members should be self-explanatory just from the name and concept restrictions.
     *
     * @tparam G Type to check.
     */
    template <typename G>
    concept graph =
        unqualified_object_type<G>
        && requires(G g, typename G::N n, typename G::E e) {
            typename G::N;
            typename G::ND;
            typename G::E;
            typename G::ED;
            // { typename G::N {} } -> node;  // Commented out because it was decided that node IDs don't have to be default constructible
            // { typename G::E {} } -> edge;  // Commented out because it was decided that edge IDs don't have to be default constructible
            { g.get_node_data(n) } -> same_as_non_cvref<typename G::ND>;
            { g.get_edge_data(e) } -> same_as_non_cvref<typename G::ED>;
            { g.get_edge_from(e) } -> same_as_non_cvref<typename G::N>;
            { g.get_edge_to(e) } -> same_as_non_cvref<typename G::N>;
            { g.get_edge(e) } -> tuple_with_3_non_cvref_children<typename G::N, typename G::N, typename G::ED>;
            { g.get_root_nodes() } -> bidirectional_range_of_non_cvref<typename G::N>;
            { g.get_root_node() } -> same_as_non_cvref<typename G::N>;
            { g.get_leaf_nodes() } -> bidirectional_range_of_non_cvref<typename G::N>;
            { g.get_leaf_node() } -> same_as_non_cvref<typename G::N>;
            { g.get_nodes() } -> bidirectional_range_of_non_cvref<typename G::N>;
            { g.get_edges() } -> bidirectional_range_of_non_cvref<typename G::E>;
            { g.has_node(n) } -> std::same_as<bool>;
            { g.has_edge(e) } -> std::same_as<bool>;
            { g.get_outputs_full(n) } -> full_input_output_range<typename G::N, typename G::E, typename G::ED>;
            { g.get_inputs_full(n) } -> full_input_output_range<typename G::N, typename G::E, typename G::ED>;
            { g.get_outputs(n) } -> bidirectional_range_of_non_cvref<typename G::E>;
            { g.get_inputs(n) } -> bidirectional_range_of_non_cvref<typename G::E>;
            { g.has_outputs(n) } -> std::same_as<bool>;
            { g.has_inputs(n) } -> std::same_as<bool>;
            { g.get_out_degree(n) } -> std::same_as<std::size_t>;
            { g.get_in_degree(n) } -> std::same_as<std::size_t>;
        };

    /**
     * Unimplemented @ref offbynull::aligner::graph::graph::graph, intended for documentation.
     *
     * @tparam N_ Node identifier type, used to lookup nodes.
     * @tparam ND_ Node data type, used to associated data with nodes.
     * @tparam E_ Edge identifier type, used to lookup edges.
     * @tparam ED_ Edge data type, used to associated data with edges.
     */
    template<
        node N_,
        unqualified_object_type ND_,
        edge E_,
        unqualified_object_type ED_
    >
    struct unimplemented_graph {
        /** Node identifier type, used to lookup nodes. */
        using N = N_;
        /** Node data type, used to associated data with nodes. */
        using ND = ND_;
        /** Edge identifier type, used to lookup edges. */
        using E = E_;
        /** Edge data type, used to associated data with edges. */
        using ED = ED_;

        /**
         * Get data associated with node.
         *
         * If `n` doesn't exist within this graph, the behavior of this function is undefined.
         *
         * @param n Node ID.
         * @return `n`'s data.
         */
        const ND& get_node_data(const N& n) const;

        /**
         * Get data associated with edge.
         *
         * If `e` doesn't exist within this graph, the behavior of this function is undefined.
         *
         * @param e Edge ID.
         * @return `e`'s data.
         */
        const ED& get_edge_data(const E& e) const;

        /**
         * Get source node of edge. If `e` doesn't exist within this graph, the behavior of this function is undefined.
         *
         * @param e Edge ID.
         * @return ID of `e`'s source node.
         */
        const N& get_edge_from(const E& e) const;

        /**
         * Get destination node of edge. If `e` doesn't exist within this graph, the behavior of this function is undefined.
         *
         * @param e Edge ID.
         * @return ID of `e`'s destination node.
         */
        const N& get_edge_to(const E& e) const;

        /**
         * Get source node, destination node, and data associated with an edge. If `e` doesn't exist within this graph, the behavior of this
         * function is undefined.
         *
         * @param e Edge ID.
         * @return ID of `e`'s source node, ID of `e`'s destination node, and `e`'s data.
         */
        std::tuple<const N&, const N&, const ED&> get_edge(const E& e) const;

        /**
         * Get root nodes.
         *
         * @return IDs of root nodes. This range may be lazily evaluated, meaning the behavior of this range becomes undefined once this
         *     graph is modified in any way.
         */
        bidirectional_range_of_non_cvref<N> auto get_root_nodes() const;

        /**
         * Get root node. If this graph doesn't contain exactly 1 root node, the behavior of this function is undefined.
         *
         * @return ID of root node.
         */
        const N& get_root_node() const;

        /**
         * Get leaf nodes.
         *
         * @return IDs of leaf nodes. This range may be lazily evaluated, meaning the behavior of this range becomes undefined once this
         *     graph is modified in any way.
         */
        bidirectional_range_of_non_cvref<N> auto get_leaf_nodes() const;

        /**
         * Get leaf node. If this graph doesn't contain exactly 1 leaf node, the behavior of this function is undefined.
         *
         * @return ID of leaf node.
         */
        const N& get_leaf_node() const;

        /**
         * List all nodes.
         *
         * @return Node IDs. This range may be lazily evaluated, meaning the behavior of this range becomes undefined once this graph is
         *     modified in any way.
         */
        bidirectional_range_of_non_cvref<N> auto get_nodes() const;

        /**
         * List all edges.
         *
         * @return Edge IDs. This range may be lazily evaluated, meaning the behavior of this range becomes undefined once this graph is
         *     modified in any way.
         */
        bidirectional_range_of_non_cvref<E> auto get_edges() const;

        /**
         * Test if node exists.
         *
         * @param n Node ID.
         * @return `true` if `n` exists in graph, `false` otherwise.
         */
        bool has_node(const N& n) const;

        /**
         * Test if edge exists.
         *
         * @param e Edge ID.
         * @return `true` if `e` exists in graph, `false` otherwise.
         */
        bool has_edge(const E& e) const;

        /**
         * List out `n`'s outgoing edges, including each edge's source, destination, and data. If `n` doesn't exist within this graph, the
         * behavior of this function is undefined.
         *
         * @param n Node ID.
         * @return Edges where `n` is the source node. Each returned edge includes the edge's ID, the edge's source  and destination (node
         *     IDs), and the data associated with that edge. This range may be lazily evaluated, meaning the behavior of this range becomes
         *     undefined once this graph is modified in any way.
         */
        full_input_output_range<N, E, ED> auto get_outputs_full(const N& n) const;

        /**
         * List out `n`'s incoming edges, including each edge's source, destination, and data. If `n` doesn't exist within this graph, the
         * behavior of this function is undefined.
         *
         * @param n Node ID.
         * @return Edges where `n` is the destination node. Each returned edge includes the edge's ID, the edge's source  and destination
         *     (node IDs), and the data associated with that edge. This range may be lazily evaluated, meaning the behavior of this range
         *     becomes undefined once this graph is modified in any way.
         */
        full_input_output_range<N, E, ED> auto get_inputs_full(const N& n) const;

        /**
         * List out `n`'s outgoing edges. If `n` doesn't exist within this graph, the behavior of this function is undefined.
         *
         * @param n Node ID.
         * @return IDs of edges where `n` is the source node. This range may be lazily evaluated, meaning the behavior of this range becomes
         *    undefined once this graph is modified in any way.
         */
        bidirectional_range_of_non_cvref<E> auto get_outputs(const N& n) const;

        /**
         * List out `n`'s incoming edges. If `n` doesn't exist within this graph, the behavior of this function is undefined.
         *
         * @param n Node ID.
         * @return IDs of edges where `n` is the destination node. This range may be lazily evaluated, meaning the behavior of this range
         *     becomes undefined once this graph is modified in any way.
         */
        bidirectional_range_of_non_cvref<E> auto get_inputs(const N& n) const;

        /**
         * Test if `n` has any outgoing edges. If `n` doesn't exist within this graph, the behavior of this function is undefined.
         *
         * @param n Node ID.
         * @return `true` if `n` has outgoing edges, `false` otherwise.
         */
        bool has_outputs(const N& n) const;

        /**
         * Test if `n` has any incoming edges. If `n` doesn't exist within this graph, the behavior of this function is undefined.
         *
         * @param n Node ID.
         * @return `true` if `n` has incoming edges, `false` otherwise.
         */
        bool has_inputs(const N& n) const;

        /**
         * Get number edges going out from `n`. If `n` doesn't exist within this graph, the behavior of this function is undefined.
         *
         * @param n Node ID.
         * @return Out-degree of `n`.
         */
        std::size_t get_out_degree(const N& n) const;

        /**
         * Get number edges coming in to `n`. If `n` doesn't exist within this graph, the behavior of this function is undefined.
         *
         * @param n Node ID.
         * @return In-degree of `n`.
         */
        std::size_t get_in_degree(const N& n) const;
    };
}

#endif //OFFBYNULL_ALIGNER_GRAPH_GRAPH_H
