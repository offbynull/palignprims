#ifndef OFFBYNULL_ALIGNER_GRAPH_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPH_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H

#include <cstddef>
#include <concepts>
#include <ranges>
#include <tuple>
#include <stdexcept>
#include "offbynull/concepts.h"
#include "offbynull/helpers/join_bidirectional_view.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"

/**
 * Pairwise sequence alignment graph interface, structured for use by the divide-and-conquer pairwise sequence alignment algorithm.
 */
namespace offbynull::aligner::graph::sliceable_pairwise_alignment_graph {
    using offbynull::concepts::range_of_one_of;
    using offbynull::concepts::bidirectional_range_of_one_of;
    using offbynull::concepts::unqualified_object_type;
    using offbynull::helpers::join_bidirectional_view::join_bidirectional_view_adaptor;
    using offbynull::aligner::graph::pairwise_alignment_graph::readable_pairwise_alignment_graph;

    /**
     * An @ref offbynull::aligner::graph::pairwise_alignment_graph::readable_pairwise_alignment_graph extended to support the
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
     * @code
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
     * @endcode
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
     * `G` must provide several members. Given a background in graph theory and alignment graphs, most of these should be self-explanatory
     * just from the name and concept restrictions. Note that, although many member variables / functions seem useless, those member
     * variables are required for certain high-performance algorithms.
     *
     * @section Members
     *
     * **[[ `resident_nodes_capacity` ]]**
     *
     * Maximum number of resident nodes.
     *
     * **[[ `resident_nodes()` ]]**
     *
     * List all resident nodes.
     *
     *  * Return: Resident nodes. This range may be lazily evaluated, meaning the behavior of this range becomes undefined once this graph
     *        is modified in any way.
     *
     * **[[ `row_nodes(INDEX grid_down)` ]]**
     *
     * List nodes in a slice (row). The behavior of this function is undefined if `grid_down` extends past the number of slices (rows)
     * within this graph's grid layout.
     *
     *  * Param `grid_down`: Slice (row) to iterator over.
     *  * Return: Nodes within `grid_down` slice. This range may be lazily evaluated, meaning the behavior of this range becomes undefined
     *        once this graph is modified in any way.
     *
     * **[[ `row_nodes(INDEX grid_down, N root_node, N leaf_node)` ]]**
     *
     * List nodes in a slice (row) as if this graph's root node and leaf node are actually `root_node` and `leaf_node` respectively. That
     * means, for a node to be returned, it must be reachable from `root_node` and reachable to `leaf_node`.
     *
     * The behavior of this function is undefined if ...
     *
     *  * `grid_down` extends past the number of slices (rows) within this graph.
     *  * `n1` or `n2` doesn't exist within this graph.
     *  * `n2` isn't reachable from `n1`.
     *
     *  * Param `grid_down`: Slice (row) to iterator over.
     *  * Param `root_node`: Root node override. When iterating nodes, it'll be as if graph's root node is actually this node.
     *  * Param `leaf_node`: Leaf node override. When iterating nodes, it'll be as if graph's leaf node is actually this node.
     *  * Return: Nodes within `grid_down` slice. This range may be lazily evaluated, meaning the behavior of this range becomes undefined
     *        once this graph is modified in any way.
     *
     * **[[ `is_reachable(N n1, N n2)` ]]**
     *
     * Test if `n2` is reachable from `n1`.
     *
     * * Return: `true` if `n2` is reachable from `n1`, `false` otherwise.
     *
     * **[[ `outputs_to_residents(N n)` ]]**
     *
     * List outgoing edges from `n` to resident nodes. If `n` doesn't exist within this graph, the behavior of this function is undefined.
     *
     *  * Param `n`: Node ID.
     *  * Return: Edges from `n` to resident nodes. This range may be lazily evaluated, meaning the behavior of this range becomes undefined
     *        once this graph is modified in any way.
     *
     * **[[ `inputs_from_residents(N n)` ]]**
     *
     * List incoming edges from resident nodes to `n`. If `n` doesn't exist within this graph, the behavior of this function is undefined.
     *
     *  * Param `n`: Node ID.
     *  * Return: Edges from resident nodes to `n`. This range may be lazily evaluated, meaning the behavior of this range becomes undefined
     *        once this graph is modified in any way.
     *
     * @tparam G Type to check.
     */
    template <typename G>
    concept readable_sliceable_pairwise_alignment_graph =
        unqualified_object_type<G>
        && readable_pairwise_alignment_graph<G>
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




    // Reference implementation for row_nodes()
    // ----------------------------------------
    // You can either ...
    // 1. have the readable_sliceable_pairwise_alignment_graph's row_nodes() function call into this function
    // 2. write a custom implementation for the readable_sliceable_pairwise_alignment_graph's row_nodes() (typically more optimized than)
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
     * @throws std::runtime_error If `debug_mode` and either `root_node` and / or `leaf_node` are missing from the graph.
     */
    template<
        bool debug_mode,
        readable_sliceable_pairwise_alignment_graph G
    >
    std::ranges::bidirectional_range auto generic_row_nodes(
        const G& g,
        typename G::INDEX grid_down,
        const typename G::N& root_node,
        const typename G::N& leaf_node
    ) {
        using INDEX = typename G::INDEX;
        using N = typename G::N;

        if constexpr (debug_mode) {
            if (!g.has_node(root_node) || !g.has_node(leaf_node)) {
                throw std::runtime_error { "Bad root / leaf node " };
            }
        }

        INDEX right_first { std::get<1>(g.node_to_grid_offset(root_node)) };
        INDEX right_last { std::get<1>(g.node_to_grid_offset(leaf_node)) };
        return
            std::views::iota(right_first, right_last + 1u)
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
        readable_sliceable_pairwise_alignment_graph G
    >
    std::ranges::bidirectional_range auto generic_row_nodes(
        const G& g,
        typename G::INDEX grid_down
    ) {
        return generic_row_nodes<debug_mode, G>(g, grid_down, g.get_root_node(), g.get_leaf_node());
    }
}

#endif //OFFBYNULL_ALIGNER_GRAPH_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
