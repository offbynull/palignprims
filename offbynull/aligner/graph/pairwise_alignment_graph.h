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

/**
 * Pairwise sequence alignment graph interface.
 */
namespace offbynull::aligner::graph::pairwise_alignment_graph {
    using offbynull::concepts::one_of;
    using offbynull::concepts::range_of_one_of;
    using offbynull::concepts::bidirectional_range_of_one_of;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::concepts::unqualified_object_type;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::graph::graph::readable_graph;
    using offbynull::utils::compile_time_constant;

    /**
     * An @ref offbynull::aligner::graph::graph::readable_graph extended / constrained to support the grid-like graphs required by pairwise
     * sequence alignment algorithms. A pairwise sequence alignment algorithm maps out the minimum number of changes required to transform
     * one sequence into another, where a "change" is typically either means an addition, a deletion, or a swap. To do this, the algorithm
     * explodes out all possible changes between the two sequences as a grid-like directed graph:
     *
     * @code
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
     * @endcode
     *
     * It's assumed that users are already familiar with sequence alignment algorithms and their applications - A full explanation of
     * sequence alignment is out of scope (if interested, see
     * https://offbynull.com/docs/data/learn/Bioinformatics/output/output.html#H_Sequence%20Alignment).
     *
     * `G` must provide several members. Given a background in graph theory and alignment graphs, most of these should be self-explanatory
     * just from the name and concept restrictions. Note that, although many member variables seem useless, those member variables are ...
     *
     *  * required for certain high-performance algorithms.
     *  * typically computed very quickly (in constant-time) due to the mostly homogenous grid-like nature of alignment graphs.
     *
     * @section Members
     *
     * **[[ type alias `INDEX` ]]**
     *
     * Unsigned integer type used for counting (e.g. `std::uint8_t` will at most allow sequences of length 255-1).
     *
     * **[[ `grid_down_cnt` ]]**
     *
     * Sequence 1's length + 1, which is the number of columns making up the grid. Sequence 1 is often referred to as "down" because it maps
     * to the grid's downward-dimension.
     *
     * **[[ `grid_right_cnt` ]]**
     *
     * Sequence 2's length + 1, which is the number of rows making up the grid. Sequence 2 is often referred to as "right" because it maps
     * to the grid's rightward-dimension.
     *
     * **[[ `grid_depth_cnt` ]]**
     *
     * Number of layers within the grid (grid's depth). Some pairwise alignment algorithms, such as extended gap alignment, require multiple
     * layers.
     *
     * *Note that this is expected to be a compile-time constant (`constexpr`).*
     *
     * **[[ `path_edge_capacity` ]]**
     *
     * Of all paths between root and leaf, the maximum number of edges.
     *
     * **[[ `node_incoming_edge_capacity` ]]**
     *
     * Of all nodes, the maximum in-degree.
     *
     * **[[ `node_outgoing_edge_capacity` ]]**
     *
     * Of all nodes, the maximum out-degree.
     *
     * **[[ `node_to_grid_offset(N n)` ]]**
     *
     * Get grid position of node within graph. If `n` doesn't exist within this graph, the behavior of this function is undefined.
     *
     *  * Param `n`: Node ID.
     *  * Return: `n`'s grid position: Down-position (row), right-position (column), layer (depth).
     *
     * **[[ `grid_offset_to_nodes(INDEX down_idx, INDEX right_idx)` ]]**
     *
     * Get nodes residing at grid position `(down_idx, right_idx)` within graph. Note that there may be multiple nodes returned if grid
     * layout has more than 1 layer. *Should a node have an outgoing edge to another node within the same position but different layer,
     * nodes are returned in topological order.* For example, imagine a graph with a 10x10x3 grid layout (10 down, 10 right, 3 layers deep).
     * In this graph, ...
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
     *  * Param `down_idx`: Down index (row index).
     *  * Param `right_idx`: Right index (colum index).
     *  * Return: Nodes within the layers at coordinate `(down_idx, right_idx)`, returned in topological order. This range may be lazily
     *    evaluated, meaning the behavior of this range becomes undefined once this graph is modified in any way.
     *
     * **[[ `edge_to_element_offsets(E e)` ]]**
     *
     * Get index within sequence 1 (downward sequence) and sequence 2 (rightward sequence) associated with `e`. When `e` is associated with
     * ...
     *
     *  * an index in sequence 1 but not in sequence 2 (e.g. indel), `std::optional { std::pair { idx1, std::nullopt } }` is returned.
     *  * an index in sequence 2 but not in sequence 1 (e.g. indel), `std::optional { std::pair { std::nullopt, idx2 } }` is returned.
     *  * an index in sequence 1 and in sequence 2 (e.g. swap), `std::optional { std::pair { idx1, idx2 } }` is returned.
     *  * neither an index in sequence 1 nor an index in sequence 2 (e.g. freeride), `std::nullopt` is returned.
     *
     * `std::optional { std::pair { std::nullopt, std::nullopt } }` will never be returned.
     *
     * If `e` doesn't exist within this graph, the behavior of this function is undefined.
     *
     *  * Param `e`: Edge ID.
     *  * Return: Index within sequence 1 (downward sequence) and sequence 2 (rightward sequence) associated with `e`, both of which are
     *        optional. See function body documentation above for full breakdown of expectations.
     *
     * @tparam G Type to check.
     */
    template <typename G>
    concept readable_pairwise_alignment_graph =
        unqualified_object_type<G>
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
}

#endif //OFFBYNULL_ALIGNER_GRAPH_PAIRWISE_ALIGNMENT_GRAPH_H
