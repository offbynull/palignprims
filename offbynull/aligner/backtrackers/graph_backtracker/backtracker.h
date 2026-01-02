#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_BACKTRACKER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_BACKTRACKER_H

#include <cstddef>
#include <functional>
#include <ranges>
#include <utility>
#include <limits>
#include <algorithm>
#include <stdexcept>
#include <type_traits>
#include "offbynull/aligner/graphs/pairwise_extended_gap_alignment_graph.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/edge_weight_accessor.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/edge_weight_accessor_without_explicit_weight.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/backtrackable_node.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/backtrackable_edge.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/backtracker_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/backtracker_heap_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/backtracker_stack_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/ready_queue/ready_queue.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/ready_queue/ready_queue_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/ready_queue/ready_queue_heap_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/ready_queue/ready_queue_stack_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/slot_container/slot.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/slot_container/slot_container.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/slot_container/slot_container_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/slot_container/slot_container_heap_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/slot_container/slot_container_stack_container_creator_pack.h"
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::graph_backtracker::backtracker {
    using offbynull::aligner::graph::graph::graph;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::backtrackers::graph_backtracker::edge_weight_accessor::edge_weight_accessor;
    using offbynull::aligner::backtrackers::graph_backtracker::edge_weight_accessor_without_explicit_weight
        ::edge_weight_accessor_without_explicit_weight;
    using offbynull::aligner::backtrackers::graph_backtracker::backtrackable_node::backtrackable_node;
    using offbynull::aligner::backtrackers::graph_backtracker::backtrackable_edge::backtrackable_edge;
    using offbynull::aligner::backtrackers::graph_backtracker::backtracker_container_creator_pack::backtracker_container_creator_pack;
    using offbynull::aligner::backtrackers::graph_backtracker::backtracker_heap_container_creator_pack
        ::backtracker_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::graph_backtracker::backtracker_stack_container_creator_pack
        ::backtracker_stack_container_creator_pack;
    using offbynull::aligner::backtrackers::graph_backtracker::slot_container::slot::slot;
    using offbynull::aligner::backtrackers::graph_backtracker::slot_container::slot_container::slot_container;
    using offbynull::aligner::backtrackers::graph_backtracker::slot_container::slot_container_container_creator_pack
        ::slot_container_container_creator_pack;
    using offbynull::aligner::backtrackers::graph_backtracker::slot_container::slot_container_heap_container_creator_pack
        ::slot_container_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::graph_backtracker::slot_container::slot_container_stack_container_creator_pack
        ::slot_container_stack_container_creator_pack;
    using offbynull::aligner::backtrackers::graph_backtracker::ready_queue::ready_queue::ready_queue;
    using offbynull::aligner::backtrackers::graph_backtracker::ready_queue::ready_queue_container_creator_pack
        ::ready_queue_container_creator_pack;
    using offbynull::aligner::backtrackers::graph_backtracker::ready_queue::ready_queue_heap_container_creator_pack
        ::ready_queue_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::graph_backtracker::ready_queue::ready_queue_stack_container_creator_pack
        ::ready_queue_stack_container_creator_pack;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::concepts::unqualified_object_type;

    /**
     * Backtracker for @ref offbynull::aligner::graph::graph::graph implementations. A backtracker's purpose is to find the
     * maximally weighted path (path with the highest sum of edge weights) between some directed graph's root node and leaf node, picking an
     * arbitrary one if there are multiple such paths. For a detailed explanation of the backtracking algorithm, see
     * https://offbynull.com/docs/data/learn/Bioinformatics/output/output.html#H_Backtrack%20Algorithm.
     *
     * Note that, although @ref offbynull::aligner::graph::graph::graph doesn't place restrictions on the directed graph's
     * structure, this backtracker implementation assumes that the directed graph it's operating on ...
     *
     *  * is acyclic.
     *  * is not empty.
     *  * has exactly one root node.
     *  * has exactly one leaf node.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam G Graph type.
     * @tparam WEIGHT Graph edge's weight type.
     * @tparam EDGE_WEIGHT_ACCESSOR Graph edge weight extractor type.
     * @tparam CONTAINER_CREATOR_PACK Container factory type.
     */
    template<
        bool debug_mode,
        graph G,
        weight WEIGHT,
        edge_weight_accessor<typename G::E, WEIGHT> EDGE_WEIGHT_ACCESSOR,
        backtracker_container_creator_pack<
            typename G::N,
            typename G::E,
            WEIGHT
        > CONTAINER_CREATOR_PACK = backtracker_heap_container_creator_pack<
            debug_mode,
            typename G::N,
            typename G::E,
            WEIGHT
        >
    >
    requires backtrackable_node<typename G::N> &&
        backtrackable_edge<typename G::E>
    class backtracker {
    public:
        /** `G`'s node identifier type. */
        using N = typename G::N;
        /** `G`'s edge identifier type. */
        using E = typename G::E;
        /** `G`'s edge data type. */
        using ED = typename G::ED;

        /**
         * @ref offbynull::aligner::backtrackers::graph_backtracker::slot_container::slot_container::slot_container container factory type
         * used by this backtracker implementation.
         */
        using SLOT_CONTAINER_CONTAINER_CREATOR_PACK =
            decltype(std::declval<CONTAINER_CREATOR_PACK>().create_slot_container_container_creator_pack());
        /**
         * @ref offbynull::aligner::backtrackers::graph_backtracker::slot_container::slot_container::slot_container type used by this
         * backtracker implementation.
         */
        using SLOT_CONTAINER = slot_container<debug_mode, G, WEIGHT, SLOT_CONTAINER_CONTAINER_CREATOR_PACK>;
        /**
         * @ref offbynull::aligner::backtrackers::graph_backtracker::ready_queue::ready_queue::ready_queue container factory type used by
         * this backtracker implementation.
         */
        using READY_QUEUE_CONTAINER_CREATOR_PACK =
            decltype(std::declval<CONTAINER_CREATOR_PACK>().create_ready_queue_container_creator_pack());
        /**
         * @ref offbynull::aligner::backtrackers::graph_backtracker::ready_queue::ready_queue::ready_queue type used by this backtracker
         * implementation.
         */
        using READY_QUEUE = ready_queue<debug_mode, G, READY_QUEUE_CONTAINER_CREATOR_PACK>;
        /**
         * Path container type used by this backtracker implementation.
         */
        using PATH_CONTAINER = decltype(std::declval<CONTAINER_CREATOR_PACK>().create_path_container());

    private:
        /**
         * Container factory.
         */
        CONTAINER_CREATOR_PACK container_creator_pack;

    public:
        /**
         * Construct an @ref offbynull::aligner::backtrackers::graph_backtracker::backtracker::backtracker instance.
         *
         * @param container_creator_pack_ Container factory.
         */
        backtracker(
            CONTAINER_CREATOR_PACK container_creator_pack_ = {}
        )
        : container_creator_pack { container_creator_pack_ } {}

        /**
         * For each node N within a directed graph, find the incoming edge into N where that edge is a part of the maximally weighted path
         * (path with the highest sum of edge weights) connecting the root node to N. In other words, unless N is the root node, there
         * exists at least one maximally weighted path from the root node to N. This function finds the last edge in that path (one of N's
         * incoming edges), referred to as N's backtracking edge.
         *
         * For example, the directed graph below has the following backtracking edges:
         *
         * ```
         *                     4
         *            .------------------.
         *            |                  |
         *      1     |   1         2    v
         *   A -----> B -----> C ------> E
         *            |        |         ^
         *            |        | 0       |
         *            |        v         |
         *            '------> D --------'
         *                2         2
         * ```
         *
         *  * A has no backtracking edge - A is the root node.
         *  * B has the backtracking edge A⟶B - weight of maximal path from A (root) to B is 1: A⟶B.
         *  * C has the backtracking edge B⟶C - weight of maximal path from A (root) to C is 2: A⟶B⟶C.
         *  * D has the backtracking edge B⟶D - weight of maximal path from A (root) to D is 3: A⟶B⟶D.
         *  * E has the backtracking edge B⟶E or D⟶E - weight of maximal path from A (root) to E is 5: A⟶B⟶E or A⟶B⟶D⟶E.
         *
         * Note that when a node has multiple options for a backtracking edge (as in the case of node E in the above example), one will be
         * arbitrarily chosen.
         *
         * The behavior of this function is undefined if any of the following conditions are met:
         *
         *  * `g` is empty.
         *  * `g` contains more than one root node.
         *  * `g` contains more than one leaf node.
         *  * `g` contains cycles.
          *  * `g` contains edges with non-finite weights.
         *
         * @param g Graph.
         * @param edge_weight_accessor Edge weight accessor for `g` (maps each edge to its weight).
         * @param zero_weight Initial edge weight, equivalent to 0 for numeric weights. Defaults to `WEIGHT`'s default constructor, assuming
         *     it exists.
         * @return For each node N within `g`, N's backtracking edge and the weight for the maximally weighted path from root to N.
         */
        SLOT_CONTAINER populate_weights_and_backtrack_pointers(
            const G& g,
            const EDGE_WEIGHT_ACCESSOR& edge_weight_accessor,
            const WEIGHT zero_weight = {}
        ) {
            // Create "slots" list
            // -------------------
            // The "slots" list tracks number of unprocessed parents for each node within the graph. The list is sorted by the
            // ordering of the node being track (node type must be orderable using std::less or < or whatever stdlib deems) such
            // that the index for a node object can be quickly found.
            auto slots_lazy {
                std::views::common(
                    g.get_nodes()
                    | std::views::transform([&](const auto& n) -> slot<N, E, WEIGHT> {
                        std::size_t in_degree { g.get_in_degree(n) };
                        // COUNT in_degree_narrowed { static_cast<COUNT>(in_degree) };
                        // if constexpr (debug_mode) {
                        //     if (in_degree_narrowed != in_degree) {
                        //         throw std::runtime_error { "Input count narrowing led to information loss" };
                        //     }
                        // }
                        return { n, in_degree, zero_weight };
                    })
                )
            };
            SLOT_CONTAINER slots {
                g,
                slots_lazy.begin(),
                slots_lazy.end(),
                container_creator_pack.create_slot_container_container_creator_pack()
            };
            // Create "ready_idxes" queue
            // --------------------------
            // The "ready_idxes" queue contains indices within "slots" that are ready-to-process (node in that slot has had all
            // parents processed, and so it can be processed). Since root nodes have no parents, they are ready-to-process from
            // the get-go. As such, the "ready_idxes" queue is primed with the "slots" indices for root nodes (of which there
            // should be only one).
            READY_QUEUE ready_idxes {
                container_creator_pack.create_ready_queue_container_creator_pack()
            };
            const N& root_node { g.get_root_node() };
            const auto& [root_slot_idx, root_slot] { slots.find(root_node) };
            ready_idxes.push(root_slot_idx);
            // static_assert(std::numeric_limits<float>::is_iec559, "IEEE 754 required"); // Require for inf and nan?
            root_slot.backtracking_weight = zero_weight;
            // Find max path within graph
            // --------------------------
            // Using the backtracking algorithm, find the path within graph that has the maximum weight. If more than one such
            // path exists, any one of the paths will be returned.
            //
            // The backtracking algorithm is a "dynamic programming" algorithm that walks over each node. A node is walked once
            // all of its parent nodes have already been walked, where the process of "walking a node" is inspecting how much
            // weight has been accumulated within each incoming edge (weight at parent + the edge from parent to the node), and
            // selecting highest one as the edge to backtrack to.
            while (!ready_idxes.empty()) {
                std::size_t idx { ready_idxes.pop() };
                auto& current_slot { slots.at_idx(idx) };
                if constexpr (debug_mode) {
                    for (const auto& edge : g.get_inputs(current_slot.node)) {
                        const auto& src_node { g.get_edge_from(edge) };
                        if (slots.find_ref(src_node).unwalked_parent_cnt != 0zu) {
                            throw std::runtime_error { "Source nodes not fully walked" };
                        }
                    }
                }
                auto incoming_accumulated {
                    std::views::common(
                        g.get_inputs(current_slot.node)
                        | std::views::transform(
                            [&](const auto& edge) -> std::pair<E, WEIGHT> {
                                const auto& src_node { g.get_edge_from(edge) };
                                const slot<N, E, WEIGHT>& src_node_slot { slots.find_ref(src_node) };
                                const auto& edge_weight { edge_weight_accessor(edge) };
                                return { edge, src_node_slot.backtracking_weight + edge_weight };
                            }
                        )
                    )
                };
                auto found {
                    // Ensure range is a common_view (begin() and end() are of same type)
                    std::ranges::max_element(
                        incoming_accumulated.begin(),
                        incoming_accumulated.end(),
                        [](const std::pair<E, WEIGHT>& a, const std::pair<E, WEIGHT>& b) {
                            return a.second < b.second;
                        }
                    )
                };
                if (found != incoming_accumulated.end()) {  // if no incoming nodes found, it's a root node
                    current_slot.backtracking_edge = (*found).first;
                    current_slot.backtracking_weight = (*found).second;
                }
                // For outgoing nodes this node points to, decrement its number of unprocessed parents (this node was one of its
                // parents, and it was processed in this iteration of the loop) then add it to "ready_idxes" if it has no more
                // unprocessed parents.
                for (const auto& edge : g.get_outputs(current_slot.node)) {
                    const auto& dst_node { g.get_edge_to(edge) };
                    const auto& [dst_slot_idx, dst_slot] { slots.find(dst_node) };
                    if constexpr (debug_mode) {
                        if (dst_slot.unwalked_parent_cnt == 0zu) {
                            throw std::runtime_error { "Invalid number of unprocessed parents" };
                        }
                    }
                    dst_slot.unwalked_parent_cnt = dst_slot.unwalked_parent_cnt - 1zu;
                    if (dst_slot.unwalked_parent_cnt == 0zu) {
                        ready_idxes.push(dst_slot_idx);
                    }
                }
            }
            // Return slots container, which can go on to be used for backtracking
            return slots;
        }

        /**
         * Utilize the output of `populate_weights_and_backtrack_pointers(g)` to determine the maximally weighted path (path with the
         * highest sum of edge weights) from `g`'s root node to `g`'s leaf node.
         *
         * For example, the directed graph below has the following backtracking edges:
         *
         * ```
         *                     4
         *            .------------------.
         *            |                  |
         *      1     |   1         2    v
         *   A -----> B -----> C ------> E
         *            |        |         ^
         *            |        | 0       |
         *            |        v         |
         *            '------> D --------'
         *                2         2
         * ```
         *
         *  * A has no backtracking edge - A is the root node.
         *  * B has the backtracking edge A⟶B.
         *  * C has the backtracking edge B⟶C.
         *  * D has the backtracking edge B⟶D.
         *  * E has the backtracking edge D⟶E - Note that E's backtracking edge could also be B⟶E, but for the purposes of this example it's
         *    assumed that `populate_weights_and_backtrack_pointers(g)` selected D⟶E.
         *
         * Walking backwards from the leaf node's backtracking edge (E's backtracking edge), the backtracking edges comprise the maximally
         * weighted path between root node (A) to leaf node (E): D⟶E, B⟶D, and A⟶B. The maximally weighted path is A⟶B⟶D⟶E.
         *
         * The behavior of this function is undefined if any of the following conditions are met:
         *
         *  * `g` is empty.
         *  * `g` contains more than one root node.
         *  * `g` contains more than one leaf node.
         *  * `g` contains cycles.
         *  * `g` contains edges with non-finite weights.
         *  * `end_node` does not exist within `g`.
         *  * `slots` is not the output of `populate_weights_and_backtrack_pointers(g)` (or if g has been modified since `slots` was
         *    generated).
         *
         * @param g Graph.
         * @param slots `populate_weights_and_backtrack_pointers(g)`'s output.
         * @param end_node Identifier of node to backtrack from, which almost always should be `g`'s leaf node.
         * @return Maximally weighted path from `g`'s root node to `end_node`.
         */
        PATH_CONTAINER backtrack(
                const G& g,
                SLOT_CONTAINER& slots,
                const N& end_node
        ) {
            if constexpr (debug_mode) {
                if (!g.has_node(end_node)) {
                    throw std::runtime_error { "End node does not exist" };
                }
            }
            auto next_node { end_node };
            PATH_CONTAINER path {
                container_creator_pack.create_path_container()
            };
            while (true) {
                auto node { next_node };
                if (!g.has_inputs(node)) {
                    break;
                }
                const auto& node_slot { slots.find_ref(node) };
                path.push_back(node_slot.backtracking_edge);
                next_node = g.get_edge_from(node_slot.backtracking_edge);
            }
            // At this point, path is in reverse order (from last to first). Reverse it to get it into the forward order
            // (from first to last).
            std::reverse(path.begin(), path.end());
            return path;
        }

        // TODO: Should the above two functions be private? They aren't used outside this class?

        /**
         * Determine the maximally weighted path (path with the highest sum of edge weights) connecting a directed graph's root node and
         * leaf node.
         *
         * The behavior of this function is undefined if any of the following conditions are met:
         *
         *  * `g` is empty.
         *  * `g` contains more than one root node.
         *  * `g` contains more than one leaf node.
         *  * `g` contains cycles.
         *  * `g` contains edges with non-finite weights.
         *
         * @param g Graph.
         * @param edge_weight_accessor Edge weight accessor for `g` (maps each edge to its weight).
         * @return Maximally weighted path from `g`'s root node to `g`'s leaf node, along with that path's weight.
         */
        std::pair<PATH_CONTAINER, ED> find_max_path(
                const G& g,
                const EDGE_WEIGHT_ACCESSOR& edge_weight_accessor
        ) {
            auto slots {
                populate_weights_and_backtrack_pointers(
                    g,
                    edge_weight_accessor
                )
            };
            const auto& end_node { g.get_leaf_node() };
            const auto& path { backtrack(g, slots, end_node) };
            const auto& weight { slots.find_ref(end_node).backtracking_weight };
            return { path, weight };
        }
    };



    // TODO: Move these helper/factory functions to backtracker_utils.h and nest them inside a class?

    /**
     * Helper function that constructs an @ref offbynull::aligner::backtrackers::graph_backtracker::backtracker::backtracker instance
     * utilizing the heap for storage / computations and invokes `find_max_path(g, edge_weight_accessor_)` on it.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @param g Graph.
     * @param edge_weight_accessor_ Edge weight accessor for `g` (maps each edge to its weight).
     * @return `find_max_path(g, edge_weight_accessor_)` result.
     */
    template<bool debug_mode>
    auto heap_find_max_path(
        const graph auto& g,
        const edge_weight_accessor_without_explicit_weight<typename std::remove_cvref_t<decltype(g)>::E> auto& edge_weight_accessor_
    ) {
        using G = std::remove_cvref_t<decltype(g)>;
        using E = typename G::E;
        using WEIGHT = decltype(edge_weight_accessor_(std::declval<E>()));
        using EDGE_WEIGHT_ACCESSOR = std::remove_cvref_t<decltype(edge_weight_accessor_)>;
        return backtracker<debug_mode, G, WEIGHT, EDGE_WEIGHT_ACCESSOR> {}.find_max_path(g, edge_weight_accessor_);
    }

    /**
     * Helper function that constructs an @ref offbynull::aligner::backtrackers::graph_backtracker::backtracker::backtracker instance
     * utilizing the stack for storage / computations and invokes `find_max_path(g, edge_weight_accessor_)` on it.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam max_slot_container_elems Maximum number of
     *     @ref offbynull::aligner::backtrackers::graph_backtracker::slot_container::slot_container::slot_container slots allowed on the
     *     stack.
     * @tparam max_ready_queue_elems Maximum number of
     *     @ref offbynull::aligner::backtrackers::graph_backtracker::ready_queue::ready_queue::ready_queue queue elements allowed on the
     *     stack.
     * @tparam max_path_elems Maximum number of edges allowed on the stack (in a container holding a path).
     * @param g Graph.
     * @param edge_weight_accessor_ Edge weight accessor for `g` (maps each edge to its weight).
     * @return `find_max_path(g, edge_weight_accessor_)` result.
     */
    template<
        bool debug_mode,
        std::size_t max_slot_container_elems,
        std::size_t max_ready_queue_elems,
        std::size_t max_path_elems
    >
    auto stack_find_max_path(
        const graph auto& g,
        const edge_weight_accessor_without_explicit_weight<typename std::remove_cvref_t<decltype(g)>::E> auto& edge_weight_accessor_
    ) {
        using G = std::remove_cvref_t<decltype(g)>;
        using N = typename G::N;
        using E = typename G::E;
        using WEIGHT = decltype(edge_weight_accessor_(std::declval<E>()));
        using EDGE_WEIGHT_ACCESSOR = std::remove_cvref_t<decltype(edge_weight_accessor_)>;
        return backtracker<
            debug_mode,
            G,
            WEIGHT,
            EDGE_WEIGHT_ACCESSOR,
            backtracker_stack_container_creator_pack<
                debug_mode,
                N,
                E,
                WEIGHT,
                max_slot_container_elems,
                max_ready_queue_elems,
                max_path_elems
            >
        > {}.find_max_path(g, edge_weight_accessor_);
    }
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_BACKTRACKER_H
