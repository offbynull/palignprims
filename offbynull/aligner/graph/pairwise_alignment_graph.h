#ifndef OFFBYNULL_ALIGNER_GRAPH_PAIRWISE_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPH_PAIRWISE_ALIGNMENT_GRAPH_H

#include <cstddef>
#include <concepts>
#include <utility>
#include <tuple>
#include <optional>
#include <cstdint>
#include "offbynull/concepts.h"
#include "offbynull/utils.h"
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/concepts.h"

namespace offbynull::aligner::graph::pairwise_alignment_graph {
    using offbynull::concepts::one_of;
    using offbynull::concepts::range_of_one_of;
    using offbynull::concepts::bidirectional_range_of_one_of;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::concepts::unqualified_object_type;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::graph::graph::graph;
    using offbynull::aligner::graph::graph::node;
    using offbynull::aligner::graph::graph::edge;
    using offbynull::aligner::graph::graph::unimplemented_graph;
    using offbynull::utils::compile_time_constant;

    /**
     * An @ref offbynull::aligner::graph::graph::graph extended / constrained to support the grid-like graphs required by pairwise
     * sequence alignment algorithms. A pairwise sequence alignment algorithm maps out the minimum number of changes required to transform
     * one sequence into another, where a "change" is typically either means an addition, a deletion, or a swap. To do this, the algorithm
     * explodes out all possible changes between the two sequences as a grid-like directed graph:
     *
     * ```
     *         "CAT" vs "TAGG"
     *
     *       T     A     G     G
     *    *---->*---->*---->*---->*
     *    |'.   |'.   |'.   |'.   |
     * C  |  '. |  '. |  '. |  '. |
     *    v    vv    vv    vv    vv
     *    *---->*---->*---->*---->*
     *    |'.   |'.   |'.   |'.   |
     * A  |  '. |  '. |  '. |  '. |
     *    v    vv    vv    vv    vv
     *    *---->*---->*---->*---->*
     *    |'.   |'.   |'.   |'.   |
     * T  |  '. |  '. |  '. |  '. |
     *    v    vv    vv    vv    vv
     *    *---->*---->*---->*---->*
     * ```
     *
     * It's assumed that users are already familiar with sequence alignment algorithms and their applications - A full explanation of
     * sequence alignment is out of scope (if interested, see
     * https://offbynull.com/docs/data/learn/Bioinformatics/output/output.html#H_Sequence%20Alignment).
     *
     * `G` must restrict its structure (structure of directed graph) to meet the restrictions of a pairwise alignment graph. Specifically,
     * the graph must ...
     *
     *  * be acyclic.
     *  * have at least one node (not empty).
     *  * have exactly one root node.
     *  * have exactly one leaf node.
     *  * be **grid-based** - nodes are positioned on a grid, where each node has a coordinate/position (X, Y, Z).
     *
     * `G` must provide several members (see @ref offbynull::aligner::graph::pairwise_alignment_graph::pairwise_alignment_graph).
     * Given a background in graph theory and alignment graphs, most of these should be self-explanatory just from the name and concept
     * restrictions. Note that, although many member variables seem useless, those member variables are ...
     *
     *  * required for certain high-performance algorithms.
     *  * typically computed very quickly (in constant-time) due to the mostly homogenous grid-like nature of alignment graphs.
     *
     * @tparam G Type to check.
     */
    template <typename G>
    concept pairwise_alignment_graph =
        unqualified_object_type<G>
        && graph<G>
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
            compile_time_constant<G::grid_depth_cnt> {};  // Enforces that grid_depth must be static compile-time const (constexpr)
            { g.path_edge_capacity } -> std::same_as<const std::size_t&>;
            { g.node_incoming_edge_capacity } -> std::same_as<const std::size_t&>;
            { g.node_outgoing_edge_capacity } -> std::same_as<const std::size_t&>;
            { g.node_to_grid_offset(node) } -> std::same_as<
                std::tuple<
                    typename G::INDEX,
                    typename G::INDEX,
                    std::size_t
                >
            >;
            { g.grid_offset_to_nodes(indexer, indexer) } -> bidirectional_range_of_one_of<typename G::N, const typename G::N&>;
            { g.edge_to_element_offsets(edge) } -> std::same_as<
                std::optional<
                    std::pair<
                        std::optional<typename G::INDEX>,
                        std::optional<typename G::INDEX>
                    >
                >
            >;
        };

    /**
     * Unimplemented @ref offbynull::aligner::graph::pairwise_alignment_graph::pairwise_alignment_graph, intended for
     * documentation.
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
    struct unimplemented_pairwise_alignment_graph : unimplemented_graph<N_, ND_, E_, ED_> {
        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::N */
        using N = N_;
        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::ND */
        using ND = ND_;
        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::E */
        using E = E_;
        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::ED */
        using ED = ED_;
        /** Grid coordinate type. For example, `std::uint8_t` will allow up to 255 nodes on both the down and right axis, meaning it'll
         * support sequences of lengths up to 255-1. */
        using INDEX = INDEX_;

        /**
         * Number of rows within the grid, which maps to downward sequence's length + 1.
         */
        const INDEX grid_down_cnt;
        /**
         * Number of columns within the grid, which maps to rightward sequence's length + 1.
         */
        const INDEX grid_right_cnt;
        /**
         * Number of layers within the grid (grid's depth). Some pairwise alignment algorithms, such as extended gap alignment, require
         * multiple layers.
         *
         * *Note that this is expected to be a compile-time constant (`constexpr`).*
         */
        static constexpr INDEX grid_depth_cnt { 0u };
        /** Maximum number of resident nodes. */
        static constexpr std::size_t resident_nodes_capacity { 0u };
        /** Of all paths between root and leaf, the maximum number of edges. */
        const std::size_t path_edge_capacity;
        /** Of all nodes, the maximum in-degree. */
        const std::size_t node_incoming_edge_capacity;
        /** Of all nodes, the maximum out-degree. */
        const std::size_t node_outgoing_edge_capacity;

        /**
         * Get grid position of node within graph. If `n` doesn't exist within this graph, the behavior of this function is undefined.
         *
         * @param n Node ID.
         * @return `n`'s grid position: Down-position (row), right-position (column), layer (depth).
         */
        std::tuple<INDEX, INDEX, std::size_t> node_to_grid_offset(const typename unimplemented_graph<N_, ND_, E_, ED_>::N& n) const;

        /**
         * Get nodes residing at grid position `(down_idx, right_idx)` within graph. Note that there may be multiple nodes returned if grid
         * layout has more than 1 layer. *Should a node have an outgoing edge to another node within the same position but different layer,
         * nodes are returned in topological order.* For example, imagine a graph with a 10x10x3 grid layout (10 down, 10 right, 3 layers
         * deep). In this graph, ...
         *
         *  * position `(5, 5)` has node A in layer 1 and node B in layer 3.
         *  * node A has an outgoing edge to node B
         *
         * As such, invoking `grid_offset_to_nodes(5, 5)` would first return A and then return B.
         *
         * The behavior of this function is undefined if ...
         *
         *  * `down_idx` extends past the number of rows within this graph's grid layout.
         *  * `right_idx` extends past the number of columns within this graph's grid layout.
         *
         * @param grid_down Down index (row index).
         * @param grid_right Right index (colum index).
         * @return Nodes within the layers at coordinate `(down_idx, right_idx)`, returned in topological order. This range may be lazily
         *     evaluated, meaning the behavior of this range becomes undefined once this graph is modified in any way.
         */
        auto grid_offset_to_nodes(INDEX grid_down, INDEX grid_right) const;

        /**
         * Get index within downward sequence and rightward sequence associated with `e`. When `e` is associated with ...
         *
         *  * an index in downward sequence but not in rightward sequence (e.g. indel),
         *    `std::optional { std::pair { d_idx, std::nullopt } }` is returned.
         *  * an index not in downward sequence but in rightward sequence (e.g. indel),
         *    `std::optional { std::pair { std::nullopt, r_idx } }` is returned.
         *  * an index in downward sequence and in rightward sequence (e.g. swap),
         *    `std::optional { std::pair { d_idx, r_idx } }` is returned.
         *  * neither an index in downward sequence nor an index in rightward sequence (e.g. freeride), `std::nullopt` is returned.
         *
         * `std::optional { std::pair { std::nullopt, std::nullopt } }` will never be returned.
         *
         * If `e` doesn't exist within this graph, the behavior of this function is undefined.
         *
         * @param e Edge ID.
         * @return Index within downward sequence and rightward sequence associated with `e`, both of which are optional. See function body
         *     documentation above for full breakdown of expectations.
         */
        auto edge_to_element_offsets(const typename unimplemented_graph<N_, ND_, E_, ED_>::E& e) const;
    };
}

#endif //OFFBYNULL_ALIGNER_GRAPH_PAIRWISE_ALIGNMENT_GRAPH_H
