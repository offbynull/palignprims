#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKER_H

#include <cstddef>
#include <cstdint>
#include <ranges>
#include <utility>
#include <functional>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtrackable_node.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtrackable_edge.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtracker_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtracker_heap_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtracker_stack_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/ready_queue/ready_queue.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/slot_container/slot.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/slot_container/slot_container.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/concepts.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker {
    using offbynull::aligner::graph::pairwise_alignment_graph::pairwise_alignment_graph;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtrackable_node::backtrackable_node;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtrackable_edge::backtrackable_edge;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker_container_creator_pack
        ::backtracker_container_creator_pack;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker_heap_container_creator_pack
        ::backtracker_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker_stack_container_creator_pack
        ::backtracker_stack_container_creator_pack;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container::slot::slot;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container::slot_container::slot_container;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::ready_queue::ready_queue::ready_queue;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::concepts::backtracking_result;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::concepts::backtracking_result_without_explicit_weight;
    using offbynull::concepts::widenable_to_size_t;

    /**
     * Backtracker for @ref offbynull::aligner::graph::pairwise_alignment_graph::pairwise_alignment_graph implementations. A
     * backtracker's purpose is to find the maximally-weighted path (path with the highest sum of edge weights) between some directed
     * graph's root node and leaf node, picking an arbitrary one if there are multiple such paths. For a detailed explanation of the
     * backtracking algorithm, see
     * https://offbynull.com/docs/data/learn/Bioinformatics/output/output.html#H_Backtrack%20Algorithm.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam G Graph type.
     * @tparam PARENT_COUNT Graph node incoming edge counter type. Must be wide enough to hold the maximum number of incoming edges across
     *      all nodes in the underlying pairwise alignment graph instance (e.g., across all nodes in any global pairwise alignment graph, a
     *      node can have at most 3 incoming edges).
     * @tparam SLOT_INDEX Slot indexer type. Must be wide enough to hold the value `grid_down_cnt * grid_right_cnt * grid_depth_cnt`
     *     (variables being multiplied are the dimensions of the `G` instance).
     * @tparam CONTAINER_CREATOR_PACK Container factory type.
     */
    template<
        bool debug_mode,
        pairwise_alignment_graph G,
        widenable_to_size_t PARENT_COUNT,
        widenable_to_size_t SLOT_INDEX,
        backtracker_container_creator_pack<
            typename G::N,
            typename G::E,
            typename G::ED,
            PARENT_COUNT,
            SLOT_INDEX
        > CONTAINER_CREATOR_PACK = backtracker_heap_container_creator_pack<
            debug_mode,
            typename G::N,
            typename G::E,
            typename G::ED,
            PARENT_COUNT,
            SLOT_INDEX,
            true
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
        /** `G`'s grid coordinate type. For example, `std::uint8_t` will allow up to 255 nodes on both the down and right axis. */
        using N_INDEX = typename G::N_INDEX;

        /**
         * @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container::slot_container::slot_container
         * container factory type used by this backtracker implementation.
         */
        using SLOT_CONTAINER_CONTAINER_CREATOR_PACK =
            decltype(std::declval<CONTAINER_CREATOR_PACK>().create_slot_container_container_creator_pack());
        /**
         * @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container::slot_container::slot_container type
         * used by this backtracker implementation.
         */
        using SLOT_CONTAINER = slot_container<debug_mode, G, PARENT_COUNT, SLOT_INDEX, SLOT_CONTAINER_CONTAINER_CREATOR_PACK>;
        /**
         * @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::ready_queue::ready_queue::ready_queue container
         * factory type used by this backtracker implementation.
         */
        using READY_QUEUE_CONTAINER_CREATOR_PACK =
            decltype(std::declval<CONTAINER_CREATOR_PACK>().create_ready_queue_container_creator_pack());
        /**
         * @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::ready_queue::ready_queue::ready_queue type used by
         * this backtracker implementation.
         */
        using READY_QUEUE = ready_queue<debug_mode, G, SLOT_INDEX, READY_QUEUE_CONTAINER_CREATOR_PACK>;
        /**
         * Path container type used by this backtracker implementation.
         */
        using PATH_CONTAINER = decltype(std::declval<CONTAINER_CREATOR_PACK>().create_path_container(0zu));

    private:
        static constexpr PARENT_COUNT PC0 { static_cast<PARENT_COUNT>(0zu) };
        static constexpr PARENT_COUNT PC1 { static_cast<PARENT_COUNT>(1zu) };

        /**
         * Initial edge weight (e.g., 0).
         */
        ED zero_weight;

        /**
         * Container factory.
         */
        CONTAINER_CREATOR_PACK container_creator_pack;

    public:
        /**
         * Construct an @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker::backtracker instance.
         *
         * @param zero_weight_ Initial weight, equivalent to 0 for numeric weights. Defaults to `ED`'s default constructor, assuming it
         *     exists.
         * @param container_creator_pack_ Container factory.
         */
        backtracker(
            ED zero_weight_ = {},
            CONTAINER_CREATOR_PACK container_creator_pack_ = {}
        )
        : zero_weight { zero_weight_ }
        , container_creator_pack { container_creator_pack_ } {}

        /**
         * Determine the maximally-weighted path (path with the highest sum of edge weights) connecting a pairwise alignment graph's root
         * node and leaf node.
         *
         * If `g` contains edges with non-finite weights, the behavior of this function is undefined.
         *
         * @param g Graph.
         * @return Maximally weighted path from `g`'s root node to `g`'s leaf node, along with that path's weight.
         */
        backtracking_result<ED> auto find_max_path(
            const G& g
        ) {
            auto slots { populate_weights_and_backtrack_pointers(g) };
            auto path { backtrack(g, slots) };  // NOTE: Don't use const auto& or auto&& - can't end up as const due to std::move() below
            const auto& leaf_node { g.get_leaf_node() };
            const auto& weight { slots.find_ref(leaf_node).backtracking_weight };
            return std::make_pair(
                std::move(path),  // For some reason this range can't be copied, but it can be moved?
                weight
            );  // NOTE: No dangling issues - make_pair() stores values, not refs.
        }

    private:
        SLOT_CONTAINER populate_weights_and_backtrack_pointers(
            const G& g
        ) {
            // Create "slots" list
            // -------------------
            // The "slots" list tracks number of unprocessed parents for each node within the graph. The list is sorted by the
            // ordering of the node being track (node type must be orderable using std::less or < or whatever stdlib deems) such
            // that the index for a node object can be quickly found.
            auto slots_lazy {
                std::views::common(
                    g.get_nodes()
                    | std::views::transform([&](const auto& n) -> slot<N, E, ED, PARENT_COUNT> {
                        std::size_t in_degree { g.get_in_degree(n) };
                        if constexpr (debug_mode) {
                            if (in_degree > std::numeric_limits<PARENT_COUNT>::max()) {
                                throw std::runtime_error { "Type not wide enough" };
                            }
                        }
                        return { n, static_cast<PARENT_COUNT>(in_degree), zero_weight };
                    })
                )
            };
            SLOT_CONTAINER slots {
                g,
                slots_lazy.begin(),
                slots_lazy.end(),
                zero_weight
            };
            // Create "ready_idxes" queue
            // --------------------------
            // The "ready_idxes" queue contains indicies within "slots" that are ready-to-process (node in that slot has had all
            // parents processed, and so it can be processed). Since root nodes have no parents, they are ready-to-process from
            // the get-go. As such, the "ready_idxes" queue is primed with the "slots" indices for root nodes (of which there
            // should be only one).
            READY_QUEUE ready_idxes { g };
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
                SLOT_INDEX idx { ready_idxes.pop() };
                auto& current_slot { slots.at_idx(idx) };
                if constexpr (debug_mode) {
                    for (const auto& edge : g.get_inputs(current_slot.node)) {
                        const auto& src_node { g.get_edge_from(edge) };
                        if (slots.find_ref(src_node).unwalked_parent_cnt != PC0) {
                            throw std::runtime_error { "Source nodes not fully walked" };
                        }
                    }
                }
                auto incoming_accumulated {
                    std::views::common(
                        g.get_inputs(current_slot.node)
                        | std::views::transform(
                            [&](const auto& edge) -> std::pair<E, ED> {
                                const auto& src_node { g.get_edge_from(edge) };
                                const slot<N, E, ED, PARENT_COUNT>& src_node_slot { slots.find_ref(src_node) };
                                const auto& edge_weight { g.get_edge_data(edge) };
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
                        [](const std::pair<E, ED>& a, const std::pair<E, ED>& b) {
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
                        if (dst_slot.unwalked_parent_cnt == PC0) {
                            throw std::runtime_error { "Invalid number of unprocessed parents" };
                        }
                    }
                    dst_slot.unwalked_parent_cnt = static_cast<PARENT_COUNT>(dst_slot.unwalked_parent_cnt - PC1);
                    if (dst_slot.unwalked_parent_cnt == PC0) {
                        ready_idxes.push(dst_slot_idx);
                    }
                }
            }
            // Return slots container, which can go on to be used for backtracking
            return slots;
        }

        auto backtrack(
            const G& g,
            const SLOT_CONTAINER& slots
        ) {
            auto next_node { g.get_leaf_node() };
            PATH_CONTAINER path {
                container_creator_pack.create_path_container(
                    g.path_edge_capacity
                )
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
            // At this point, path is in reverse order (from last to first). Reverse it (view) to get it into forward order.
            return std::move(path) | std::views::reverse;
        }
    };


    // TODO: Move these helper/factory functions to backtracker_utils.h and nest them inside a class?

    /**
     * Helper function that constructs an
     * @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker::backtracker instance utilizing the heap for
     * storage / computations and invokes `find_max_path(g)` on it.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam PARENT_COUNT Graph node incoming edge counter type. Must be wide enough to hold the maximum number of incoming edges across
     *      all nodes in the underlying pairwise alignment graph instance (e.g., across all nodes in any global pairwise alignment graph, a
     *      node can have at most 3 incoming edges).
     * @tparam SLOT_INDEX Slot indexer type. Must be wide enough to hold the value `grid_down_cnt * grid_right_cnt * grid_depth_cnt`
     *     (variables being multiplied are the dimensions of `g`).
     * @tparam minimize_allocations `true` to force underlying
     *     @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::ready_queue::ready_queue::ready_queue` to prime with
     *     `grid_down_cnt * grid_right_cnt * grid_depth_cnt` reserved elements (variables being multiplied are the dimensions of `g`),
     *     thereby removing/reducing the need for adhoc reallocations.
     * @param g Graph.
     * @return `find_max_path(g)` result.
     */
    template<
        bool debug_mode,
        widenable_to_size_t PARENT_COUNT,
        widenable_to_size_t SLOT_INDEX,
        bool minimize_allocations
    >
    backtracking_result_without_explicit_weight auto heap_find_max_path(
        const pairwise_alignment_graph auto& g
    ) {
        using G = std::remove_cvref_t<decltype(g)>;
        return backtracker<
            debug_mode,
            G,
            PARENT_COUNT,
            SLOT_INDEX,
            backtracker_heap_container_creator_pack<
                debug_mode,
                typename G::N,
                typename G::E,
                typename G::ED,
                PARENT_COUNT,
                SLOT_INDEX,
                minimize_allocations
            >
        > {}.find_max_path(g);
    }

    /**
     * Helper function that constructs an
     * @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker::backtracker instance utilizing the stack
     * for storage / computations and invokes `find_max_path(g)` on it.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam PARENT_COUNT Graph node incoming edge counter type. Must be wide enough to hold the maximum number of incoming edges across
     *      all nodes in the underlying pairwise alignment graph instance (e.g., across all nodes in any global pairwise alignment graph, a
     *      node can have at most 3 incoming edges).
     * @tparam SLOT_INDEX Slot indexer type. Must be wide enough to hold the value `grid_down_cnt * grid_right_cnt * grid_depth_cnt`
     *     (variables being multiplied are the dimensions of `g`).
     * @tparam grid_down_cnt `g`'s down dimension of the underlying pairwise alignment graph instance.
     * @tparam grid_right_cnt `g`'s right dimension of the underlying pairwise alignment graph instance.
     * @tparam grid_depth_cnt `g'`s depth dimension of the underlying pairwise alignment graph instance.
     * @tparam path_edge_capacity Of all paths between root and leaf within `g`, the maximum number of edges.
     * @param g Graph.
     * @return `find_max_path(g)` result.
     */
    template<
        bool debug_mode,
        widenable_to_size_t PARENT_COUNT,
        widenable_to_size_t SLOT_INDEX,
        std::size_t grid_down_cnt,
        std::size_t grid_right_cnt,
        std::size_t grid_depth_cnt,
        std::size_t path_edge_capacity
    >
    backtracking_result_without_explicit_weight auto stack_find_max_path(
        const pairwise_alignment_graph auto& g
    ) {
        using G = std::remove_cvref_t<decltype(g)>;
        using N = typename G::N;
        using E = typename G::E;
        using ED = typename G::ED;
        if constexpr (debug_mode) {
            if (g.grid_down_cnt != grid_down_cnt
                || g.grid_right_cnt != grid_right_cnt
                || g.grid_depth_cnt != grid_depth_cnt
                || g.path_edge_capacity != path_edge_capacity) {
                throw std::runtime_error { "Unexpected graph dimensions" };
            }
        }
        return backtracker<
            debug_mode,
            G,
            PARENT_COUNT,
            SLOT_INDEX,
            backtracker_stack_container_creator_pack<
                debug_mode,
                N,
                E,
                ED,
                PARENT_COUNT,
                SLOT_INDEX,
                grid_down_cnt,
                grid_right_cnt,
                grid_depth_cnt,
                path_edge_capacity
            >
        > {}.find_max_path(g);
    }
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKER_H
