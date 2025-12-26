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
    using offbynull::concepts::bidirectional_range_of_non_cvref;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::concepts::unqualified_object_type;
    using offbynull::helpers::join_bidirectional_view::join_bidirectional_view_adaptor;
    using offbynull::aligner::graph::pairwise_alignment_graph::pairwise_alignment_graph;
    using offbynull::aligner::graph::pairwise_alignment_graph::unimplemented_pairwise_alignment_graph;
    using offbynull::aligner::graph::graph::node;
    using offbynull::aligner::graph::graph::edge;

    /**
     * An @ref offbynull::aligner::graph::pairwise_alignment_graph::pairwise_alignment_graph extended to support the
     * divide-and-conquer pairwise sequence alignment algorithm. A full description of the algorithm, as applied to global alignment, is
     * available at https://offbynull.com/docs/data/learn/Bioinformatics/output/output.html#H_Divide-and-Conquer%20Algorithm (it's assumed
     * the user is already familiar with this algorithm).
     *
     * The general idea behind the divide-and-conquer algorithm is to recursively subdivide the alignment graph, bidirectionally walking
     * ever smaller portions of the graph until the entire alignment path is retrieved. The algorithm, as described in the link above, is
     * only applicable to pairwise global alignment. To support other types of pairwise alignments (e.g., local, overlap, ...), the
     * algorithm has been modified.
     *
     * The first modification (which has nothing to do with supporting other types of pairwise alignments) is that walking and subdivision
     * is expected to happen by row, not by column. For example, ...
     *
     *  * the pairwise alignment graph is walked row-by-row as opposed to column-by-column
     *  * the pairwise alignment graph is split on a row as opposed to being split on a column.
     *
     * The second modification is the introduction of resident nodes to support other types of pairwise alignments (e.g., local, overlap,
     * ...). To understand what resident nodes are and how they work, begin by recalling that the base of any pairwise alignment graph is a
     * global alignment:
     *
     * ```
     *         global alignment
     *         "CAT" vs "TAGG"
     *
     *       T     A     G     G
     *    *---->*---->*---->*---->*    row 0
     *    |'.   |'.   |'.   |'.   |
     * C  |  '. |  '. |  '. |  '. |
     *    v    vv    vv    vv    vv
     *    *---->*---->*---->*---->*    row 1
     *    |'.   |'.   |'.   |'.   |
     * A  |  '. |  '. |  '. |  '. |
     *    v    vv    vv    vv    vv
     *    *---->*---->*---->*---->*    row 2
     *    |'.   |'.   |'.   |'.   |
     * T  |  '. |  '. |  '. |  '. |
     *    v    vv    vv    vv    vv
     *    *---->*---->*---->*---->*    row 3
     * ```
     *
     * In a global pairwise alignment graph, each node within a row has an outgoing edge to ...
     *
     *  * the neighboring node in the next column (assuming not already at the last column).
     *  * the neighboring node in the next row (assuming not already at the last row).
     *  * the neighboring node diagonally across in the next row+column (assuming not already at the leaf node).
     *
     * For the divide-and-conquer algorithm to walk such a graph, only 2 rows need to be retained in memory. That is, the parents
     * (dependencies) of the current node being walked are within the current node's row and the adjacent row directly above it.
     *
     * Other types of pairwise alignments extend global pairwise alignment by adding edges that connect nodes in non-adjacent rows. For
     * example, a pairwise fitting alignment connects the root node to nodes in left-hand column, and connects nodes in the right-hand
     * column to the leaf node.
     *
     * ```
     *          fitting alignment
     *           "CAT" vs "TAGG"
     *
     *           T     A     G     G
     * ,-,-,---*---->*---->*---->*---->*------.    row 0
     * | | |   |'.   |'.   |'.   |'.   |      |
     * | | | C |  '. |  '. |  '. |  '. |      |
     * | | |   v    vv    vv    vv    vv      |
     * | | '-->*---->*---->*---->*---->*----. |    row 1
     * | |     |'.   |'.   |'.   |'.   |    | |
     * | |   A |  '. |  '. |  '. |  '. |    | |
     * | |     v    vv    vv    vv    vv    | |
     * | '---->*---->*---->*---->*---->*--. | |    row 2
     * |       |'.   |'.   |'.   |'.   |  | | |
     * |     T |  '. |  '. |  '. |  '. |  | | |
     * |       v    vv    vv    vv    vv  | | |
     * '------>*---->*---->*---->*---->*<-'-'-'    row 3
     * ```
     *
     * To support edges that connect nodes in non-adjacent rows, some nodes connecting these non-adjacent edges are retained in memory for
     * the entirety of the algorithm's run (regardless of the row being walked, outside of the 2 rows actively retained in memory). Each of
     * these nodes, referred to as a resident node, has its ...
     *
     *  * weight updated as each parent is walked, even if its parent lives farther away than the previous row.
     *  * weight accessible by each child, even if its child lives farther away than the next row.
     *
     * The general rule of thumb is that resident nodes should be few-and-far between, where each one is either source or destination of a
     * large number of edges in non-adjacent rows. For example, consider the structure of a local pairwise alignment graph:
     *
     *  * Root has outgoing edges to every other node (freerides).
     *  * Leaf has incoming edge from every other node (freerides)
     *  * All nodes other than root and leaf have an edge from root and an edge to leaf (freerides).
     *
     * In a local pairwise alignment graph, it makes sense for the root and leaf to become resident nodes. Whenever a node in the middle
     * (not root or leaf) is walked, it ...
     *
     *  * queries root for its weight + weight from the freeride edge connecting it.
     *  * updates leaf with its weight + weight from the freeride edge connecting it (only updating leaf if leaf doesn't already contain a
     *    higher weight).
     *
     * In other types of pairwise alignment graphs, the divide-and-conquer algorithm may result in needing so many resident nodes that it
     * kills the memory efficiency provided by the algorithm. In such cases, the graph should avoid satisfying this concept (such that the
     * divide-and-conquer algorithm won't support it).
     *
     * `G` must provide several members (see
     * @ref offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph). Given a
     * background in graph theory and alignment graphs, most of these should be self-explanatory just from the name and concept
     * restrictions. Note that, although many member variables / functions seem useless, those member variables are required for
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
            typename G::N_INDEX grid_down,
            typename G::N_INDEX grid_right
        ) {
            { g.resident_nodes_capacity } -> std::same_as<const std::size_t&>;
            // TODO: The way things work right now, the backtracker for this grap type keeps 2 rows in memory as it walks the graph. But, do
            //       you need 2 rows? Can you structure this to keep just a portion of the 2 rows needed to continue the walk?
            { g.row_nodes(grid_down) } -> bidirectional_range_of_non_cvref<typename G::N>;
            { g.row_nodes(grid_down, node, node) } -> bidirectional_range_of_non_cvref<typename G::N>;
            { g.is_reachable(node, node) } -> std::same_as<bool>;
            { g.resident_nodes() } -> bidirectional_range_of_non_cvref<typename G::N>;
            { g.outputs_to_residents(node) } -> bidirectional_range_of_non_cvref<typename G::E>;
            { g.inputs_from_residents(node) } -> bidirectional_range_of_non_cvref<typename G::E>;
        };

    /**
     * Unimplemented @ref offbynull::aligner::graph::sliceable_pairwise_alignment_graph::sliceable_pairwise_alignment_graph,
     * intended for documentation.
     *
     * @tparam N_ Node identifier type, used to lookup nodes.
     * @tparam ND_ Node data type, used to associateddata with nodes.
     * @tparam E_ Edge identifier type, used to lookup edges.
     * @tparam ED_ Edge data type, used to associate data with edges.
     * @tparam N_INDEX_ Node coordinate type.
     */
    template<
        node N_,
        unqualified_object_type ND_,
        edge E_,
        unqualified_object_type ED_,
        widenable_to_size_t N_INDEX_
    >
    struct unimplemented_sliceable_pairwise_alignment_graph : unimplemented_pairwise_alignment_graph<N_, ND_, E_, ED_, N_INDEX_> {
        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::N */
        using N = N_;
        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::ND */
        using ND = ND_;
        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::E */
        using E = E_;
        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::ED */
        using ED = ED_;
        /** @copydoc offbynull::aligner::graph::pairwise_alignment_graph::unimplemented_pairwise_alignment_graph::N_INDEX */
        using N_INDEX = N_INDEX_;

        /** Maximum number of resident nodes. */
        static constexpr std::size_t resident_nodes_capacity { 0zu };

        /**
         * List nodes in a slice (row). The behavior of this function is undefined if `grid_down` extends past the number of slices (rows)
         * within this graph's grid layout.
         *
         * @param grid_down Slice (row) to iterate over.
         * @return Nodes within `grid_down` slice. This range may be lazily evaluated, meaning the behavior of this range becomes undefined
         *     once this graph is modified in any way.
         */
        bidirectional_range_of_non_cvref<N> auto row_nodes(N_INDEX grid_down) const;

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
         * @param grid_down Slice (row) to iterate over.
         * @param root_node Root node identifier override. When iterating nodes, it'll be as if graph's root node is actually this node.
         * @param leaf_node Leaf node identifier override. When iterating nodes, it'll be as if graph's leaf node is actually this node.
         * @return Nodes within `grid_down` slice. This range may be lazily evaluated, meaning the behavior of this range becomes undefined
         *     once this graph is modified in any way.
         */
        bidirectional_range_of_non_cvref<N> auto row_nodes(N_INDEX grid_down, const N& root_node, const N& leaf_node) const;

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
        bidirectional_range_of_non_cvref<N> auto resident_nodes() const;

        /**
         * List outgoing edges from `n` to resident nodes. If `n` doesn't exist within this graph, the behavior of this function is
         * undefined.
         *
         * @param n Node ID.
         * @return Edges from `n` to resident nodes. This range may be lazily evaluated, meaning the behavior of this range becomes
         *     undefined once this graph is modified in any way.
         */
        bidirectional_range_of_non_cvref<E> auto outputs_to_residents(const N& n) const;

        /**
         * List incoming edges from resident nodes to `n`. If `n` doesn't exist within this graph, the behavior of this function is
         * undefined.
         *
         * @param n Node ID.
         * @return Edges from resident nodes to `n`. This range may be lazily evaluated, meaning the behavior of this range becomes
         *     undefined once this graph is modified in any way.
         */
        bidirectional_range_of_non_cvref<E> auto inputs_from_residents(const N& n) const;
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
     * @param grid_down Slice (row) to iterate over.
     * @param root_node Root node identifier override. When iterating nodes, it'll be as if `g`'s root node is actually this node.
     * @param leaf_node Leaf node identifier override. When iterating nodes, it'll be as if `g`'s leaf node is actually this node.
     * @return Range of nodes. This range may be lazily evaluated, meaning the behavior of this range becomes undefined once `g` is modified
     *     in any way.
     */
    template<
        bool debug_mode,
        sliceable_pairwise_alignment_graph G
    >
    bidirectional_range_of_non_cvref<typename G::N> auto generic_row_nodes(
        const G& g,
        typename G::N_INDEX grid_down,
        const typename G::N& root_node,
        const typename G::N& leaf_node
    ) {
        using N_INDEX = typename G::N_INDEX;
        using N = typename G::N;

        // constexpr N_INDEX I0 { static_cast<N_INDEX>(0zu) };
        constexpr N_INDEX I1 { static_cast<N_INDEX>(1zu) };

        if constexpr (debug_mode) {
            if (!g.has_node(root_node) || !g.has_node(leaf_node)) {
                throw std::runtime_error { "Bad root / leaf node " };
            }
        }

        N_INDEX right_first { std::get<1zu>(g.node_to_grid_offset(root_node)) };
        N_INDEX right_last { std::get<1zu>(g.node_to_grid_offset(leaf_node)) };
        return
            std::views::iota(
                right_first,
                static_cast<N_INDEX>(right_last + I1)  // Cast to prevent narrowing warning
            )
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
     * @param grid_down Slice (row) to iterate over.
     * @return Nodes within `grid_down` slice. This range may be lazily evaluated, meaning the behavior of this range becomes undefined once
     *     `g` is modified in any way.
     */
    template<
        bool debug_mode,
        sliceable_pairwise_alignment_graph G
    >
    bidirectional_range_of_non_cvref<typename G::N> auto generic_row_nodes(
        const G& g,
        typename G::N_INDEX grid_down
    ) {
        return generic_row_nodes<debug_mode, G>(g, grid_down, g.get_root_node(), g.get_leaf_node());
    }
}

#endif //OFFBYNULL_ALIGNER_GRAPH_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
