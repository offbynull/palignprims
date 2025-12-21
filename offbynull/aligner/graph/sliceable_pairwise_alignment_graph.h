#ifndef OFFBYNULL_ALIGNER_GRAPH_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPH_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H

#include <cstddef>
#include <concepts>
#include <ranges>
#include <tuple>
#include <stdexcept>
#include "offbynull/concepts.h"
#include "offbynull/helpers/join_bidirectional_view.h"
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"

namespace offbynull::aligner::graph::sliceable_pairwise_alignment_graph {
    using offbynull::concepts::range_of_one_of;
    using offbynull::concepts::bidirectional_range_of_one_of;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::concepts::unqualified_object_type;
    using offbynull::helpers::join_bidirectional_view::join_bidirectional_view_adaptor;
    using offbynull::aligner::graph::pairwise_alignment_graph::pairwise_alignment_graph;
    using offbynull::aligner::graph::pairwise_alignment_graph::unimplemented_pairwise_alignment_graph;
    using offbynull::aligner::graph::graph::node;
    using offbynull::aligner::graph::graph::edge;

    /**
     * An @ref offbynull::aligner::graph::pairwise_alignment_graph::pairwise_alignment_graph extended to support the
     * divide-and-conquer pairwise sequence alignment algorithm. The divide-and-conquer algorithm reduces memory usage at the expense of
     * increased computation (some calculations are repeated multiple times). The general idea is to recursively subdivide the alignment
     * graph, re-walking ever smaller portions of the graph, until the entire alignment path is retrieved.
     *
     * Each subdivision is walked row-by-row, where a row is colloquially referred to as a slice. In a typical global pairwise alignment
     * graph, each node within a slice has incoming edges coming from nodes in ...
     *
     *  * the same slice.
     *  * the slice directly above it.
     *
     * ```
     *         global alignment
     *         "CAT" vs "TAGG"
     *
     *       T     A     G     G
     *    *---->*---->*---->*---->*    slice 0
     *    |'.   |'.   |'.   |'.   |
     * C  |  '. |  '. |  '. |  '. |
     *    v    vv    vv    vv    vv
     *    *---->*---->*---->*---->*    slice 1
     *    |'.   |'.   |'.   |'.   |
     * A  |  '. |  '. |  '. |  '. |
     *    v    vv    vv    vv    vv
     *    *---->*---->*---->*---->*    slice 2
     *    |'.   |'.   |'.   |'.   |
     * T  |  '. |  '. |  '. |  '. |
     *    v    vv    vv    vv    vv
     *    *---->*---->*---->*---->*    slice 3
     * ```
     *
     * Because the only dependencies are on the current slice and the slice above it, the entire process is much more memory efficient.
     *
     * For more complex types of pairwise alignment graphs, each node in a slice may also have ...
     *
     *  * incoming edges from nodes that are potentially outside of the same slice and the slice above the current slice.
     *  * outgoing edges to nodes that are potentially outside of the same slice and the slice above the current slice.
     *
     * These nodes, referred to as resident nodes, need to be always kept in memory regardless of what the current slice is. Resident nodes
     * are typically few-and-far between. For example, a local pairwise alignment graph will have ...
     *
     *  * an edge from the root node to every non-root node, meaning the root node is a resident node.
     *  * an edge from every non-leaf node to the leaf node, meaning the leaf node is a resident node.
     *
     * Aside from the terminology described above, it's assumed that users are already familiar with the divide-and-conquer algorithm - A
     * full explanation of is out of scope (if interested, see
     * https://offbynull.com/docs/data/learn/Bioinformatics/output/output.html#H_Divide-and-Conquer%20Algorithm).
     *
     * `G` must provide several members (see
     * @ref offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph). Given a
     * background in graph theory and alignment graphs, most of these should be self-explanatory just from the name and concept
     * restrictions. Note that, although many member variables / functions seem useless, those member variables are required for certain
     * high-performance algorithms.
     *
     * @tparam G Type to check.
     */
    template <typename G>
    concept sliceable_pairwise_alignment_graph =
        unqualified_object_type<G>
        && pairwise_alignment_graph<G>
        && requires(
            G g,
            typename G::N node,
            typename G::INDEX grid_down,
            typename G::INDEX grid_right
        ) {
            { g.resident_nodes_capacity } -> std::same_as<const std::size_t&>;
            { g.row_nodes(grid_down) } -> bidirectional_range_of_one_of<typename G::N, const typename G::N&>;
            { g.row_nodes(grid_down, node, node) } -> bidirectional_range_of_one_of<typename G::N, const typename G::N&>;
            { g.is_reachable(node, node) } -> std::same_as<bool>;
            { g.resident_nodes() } -> range_of_one_of<typename G::N, const typename G::N&>;
            { g.outputs_to_residents(node) } -> range_of_one_of<typename G::E, const typename G::E&>;
            { g.inputs_from_residents(node) } -> range_of_one_of<typename G::E, const typename G::E&>;
        };

    /**
     * Unimplemented @ref offbynull::aligner::graph::sliceable_pairwise_alignment_graph::sliceable_pairwise_alignment_graph,
     * intended for documentation.
     *
     * @tparam N_ Node identifier type, used to lookup nodes.
     * @tparam ND_ Node data type, used to associated data with nodes.
     * @tparam E_ Edge identifier type, used to lookup edges.
     * @tparam ED_ Edge data type, used to associated data with edges.
     * @tparam INDEX_ Node coordinate type.
     */
    template<
        node N_,
        unqualified_object_type ND_,
        edge E_,
        unqualified_object_type ED_,
        widenable_to_size_t INDEX_
    >
    struct unimplemented_sliceable_pairwise_alignment_graph : unimplemented_pairwise_alignment_graph<N_, ND_, E_, ED_, INDEX_> {
        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::N */
        using N = N_;
        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::ND */
        using ND = ND_;
        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::E */
        using E = E_;
        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::ED */
        using ED = ED_;
        /** @copydoc offbynull::aligner::graph::pairwise_alignment_graph::unimplemented_pairwise_alignment_graph::INDEX */
        using INDEX = INDEX_;

        /** Maximum number of resident nodes. */
        static constexpr std::size_t resident_nodes_capacity { 0zu };

        /**
         * List nodes in a slice (row). The behavior of this function is undefined if `grid_down` extends past the number of slices (rows)
         * within this graph's grid layout.
         *
         * @param grid_down Slice (row) to iterator over.
         * @return Nodes within `grid_down` slice. This range may be lazily evaluated, meaning the behavior of this range becomes undefined
         *     once this graph is modified in any way.
         */
        auto row_nodes(INDEX grid_down) const;

        /**
         * List nodes in a slice (row) as if this graph's root node and leaf node are actually `root_node` and `leaf_node` respectively.
         * That means, for a node to be returned, it must be reachable from `root_node` and reachable to `leaf_node`.
         *
         * The behavior of this function is undefined if ...
         *
         *  * `grid_down` extends past the number of slices (rows) within this graph.
         *  * `n1` or `n2` doesn't exist within this graph.
         *  * `n2` isn't reachable from `n1`.
         *
         * @param grid_down Slice (row) to iterator over.
         * @param root_node Root node override. When iterating nodes, it'll be as if graph's root node is actually this node.
         * @param leaf_node Leaf node override. When iterating nodes, it'll be as if graph's leaf node is actually this node.
         * @return Nodes within `grid_down` slice. This range may be lazily evaluated, meaning the behavior of this range becomes undefined
         *     once this graph is modified in any way.
         */
        auto row_nodes(INDEX grid_down, const N& root_node, const N& leaf_node) const;

        /**
         * Test if `n2` is reachable from `n1`.
         *
         * @return `true` if `n2` is reachable from `n1`, `false` otherwise.
         */
        bool is_reachable(const N& n1, const N& n2) const;

        /**
         * List all resident nodes.
         *
         * @return Resident nodes. This range may be lazily evaluated, meaning the behavior of this range becomes undefined once this graph
         *     is modified in any way.
         */
        auto resident_nodes() const;

        /**
         * List outgoing edges from `n` to resident nodes. If `n` doesn't exist within this graph, the behavior of this function is
         * undefined.
         *
         * @param n Node ID.
         * @return Edges from `n` to resident nodes. This range may be lazily evaluated, meaning the behavior of this range becomes
         *     undefined once this graph is modified in any way.
         */
        auto outputs_to_residents(const N& n) const;

        /**
         * List incoming edges from resident nodes to `n`. If `n` doesn't exist within this graph, the behavior of this function is
         * undefined.
         *
         * @param n Node ID.
         * @return Edges from resident nodes to `n`. This range may be lazily evaluated, meaning the behavior of this range becomes
         *     undefined once this graph is modified in any way.
         */
        auto inputs_from_residents(const N& n) const;
    };


    // Reference implementation for row_nodes()
    // ----------------------------------------
    // You can either ...
    // 1. have the sliceable_pairwise_alignment_graph's row_nodes() function call into this function
    // 2. write a custom implementation for the sliceable_pairwise_alignment_graph's row_nodes() (typically more optimized than)
    //    and test against this to ensure things are working correctly.
    /**
     * List nodes in a slice (row) as if this `g`'s root node and leaf node are actually `root_node` and `leaf_node` respectively. That
     * means, for a node to be returned, it must be reachable from `root_node` and reachable to `leaf_node`.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam G Graph type.
     * @param g Graph.
     * @param grid_down Slice (row) to iterator over.
     * @param root_node Root node override. When iterating nodes, it'll be as if `g`'s root node is actually this node.
     * @param leaf_node Leaf node override. When iterating nodes, it'll be as if `g`'s leaf node is actually this node.
     * @return Range of nodes. This range may be lazily evaluated, meaning the behavior of this range becomes undefined once `g` is modified
     *     in any way.
     */
    template<
        bool debug_mode,
        sliceable_pairwise_alignment_graph G
    >
    std::ranges::bidirectional_range auto generic_row_nodes(
        const G& g,
        typename G::INDEX grid_down,
        const typename G::N& root_node,
        const typename G::N& leaf_node
    ) {
        using INDEX = typename G::INDEX;
        using N = typename G::N;

        constexpr INDEX I0 { static_cast<INDEX>(0zu) };
        constexpr INDEX I1 { static_cast<INDEX>(1zu) };

        if constexpr (debug_mode) {
            if (!g.has_node(root_node) || !g.has_node(leaf_node)) {
                throw std::runtime_error { "Bad root / leaf node " };
            }
        }

        INDEX right_first { std::get<1zu>(g.node_to_grid_offset(root_node)) };
        INDEX right_last { std::get<1zu>(g.node_to_grid_offset(leaf_node)) };
        return
            std::views::iota(right_first, right_last + I1)
            | std::views::transform([&g, grid_down](const auto& grid_right) {
                return g.grid_offset_to_nodes(grid_down, grid_right);
            })
            | join_bidirectional_view_adaptor {}
            | std::views::filter([&g, root_node, leaf_node](const N& node) {
                return g.is_reachable(root_node, node) && g.is_reachable(node, leaf_node);
            });
    }

    /**
     * List nodes in a slice (row). The behavior of this function is undefined if `grid_down` extends past the number of slices (rows)
     * within `g`'s grid layout.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam G Graph type.
     * @param g Graph.
     * @param grid_down Slice (row) to iterator over.
     * @return Nodes within `grid_down` slice. This range may be lazily evaluated, meaning the behavior of this range becomes undefined once
     *     `g` is modified in any way.
     */
    template<
        bool debug_mode,
        sliceable_pairwise_alignment_graph G
    >
    std::ranges::bidirectional_range auto generic_row_nodes(
        const G& g,
        typename G::INDEX grid_down
    ) {
        return generic_row_nodes<debug_mode, G>(g, grid_down, g.get_root_node(), g.get_leaf_node());
    }
}

#endif //OFFBYNULL_ALIGNER_GRAPH_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
