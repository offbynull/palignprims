#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BIDI_WALKER_BIDI_WALKER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BIDI_WALKER_BIDI_WALKER_H

#include <utility>
#include <ranges>
#include <cmath>
#include <type_traits>
#include <stdexcept>
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtrackable_node.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtrackable_edge.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/bidi_walker/bidi_walker_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/bidi_walker/bidi_walker_heap_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/forward_walker/forward_walker.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/row_slot_container/slot.h"
#include "offbynull/aligner/graphs/reversed_sliceable_pairwise_alignment_graph.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker::bidi_walker {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::sliceable_pairwise_alignment_graph;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtrackable_node::backtrackable_node;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtrackable_edge::backtrackable_edge;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker
        ::bidi_walker_container_creator_pack::bidi_walker_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker
        ::bidi_walker_heap_container_creator_pack::bidi_walker_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker::forward_walker::forward_walker;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container::slot::slot;
    using offbynull::aligner::graphs::reversed_sliceable_pairwise_alignment_graph::reversed_sliceable_pairwise_alignment_graph;
    using offbynull::concepts::bidirectional_range_of_exact;

    /**
     * Bidirectional walker for
     * @ref offbynull::aligner::graph::sliceable_pairwise_alignment_graph::sliceable_pairwise_alignment_graph implementations. A
     * bidirectional walker walks the graph in two directions, converging on some node N:
     *
     *  * Walk from the root node to N.
     *  * Walk from the leaf node to N (e.g., as if walking the graph in reverse - each edge's direction is flipped).
     *
     * For each direction, it calculates the weight of the maximally-weighted path between those two nodes (path with the highest sum of
     * edge weights).
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam G Graph type.
     * @tparam CONTAINER_CREATOR_PACK Container factory type.
     */
    template<
        bool debug_mode,
        sliceable_pairwise_alignment_graph G,
        bidi_walker_container_creator_pack<
            typename G::N,
            typename G::E,
            typename G::ED
        > CONTAINER_CREATOR_PACK = bidi_walker_heap_container_creator_pack<
            debug_mode,
            typename G::N,
            typename G::E,
            typename G::ED,
            true
        >
    >
    requires backtrackable_node<typename G::N> &&
        backtrackable_edge<typename G::E>
    class bidi_walker {
    public:
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using INDEX = typename G::INDEX;

    private:
        using FORWARD_WALKER_CONTAINER_CREATOR_PACK =
            decltype(std::declval<CONTAINER_CREATOR_PACK>().create_forward_walker_container_creator_pack());
        using BACKWARD_WALKER_CONTAINER_CREATOR_PACK =
            decltype(std::declval<CONTAINER_CREATOR_PACK>().create_backward_walker_container_creator_pack());

        static constexpr INDEX I0 { static_cast<INDEX>(0zu) };
        static constexpr INDEX I1 { static_cast<INDEX>(1zu) };

        const G& g;
        const INDEX target_row;
        const reversed_sliceable_pairwise_alignment_graph<debug_mode, G> reversed_g;
        forward_walker<debug_mode, G, FORWARD_WALKER_CONTAINER_CREATOR_PACK> forward_walker_;
        forward_walker<debug_mode, std::remove_const_t<decltype(reversed_g)>, BACKWARD_WALKER_CONTAINER_CREATOR_PACK> backward_walker;

    public:
        /**
         * Create an
         * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker::bidi_walker::bidi_walker
         * instance targeted at nodes within a specific row of the graph. For example, consider a scenario where `target_row` is 5. For each
         * node N in `g`'s 5th row, the returned object will have the ...
         *
         *  * weight of the maximally-weighted path between the root node and N.
         *  * weight of the maximally-weighted path between the leaf node and N (as if walking the graph in reverse).
         *
         * The behavior of this function is undefined if `target_row` is past the final row within `g`.
         *
         * @param g_ Graph.
         * @param target_row Row within `g`.
         * @param container_creator_pack Container factory.
         * @return @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker::forward_walker
         *     instance primed to `g`'s `target_row` row.
         */
        static bidi_walker create_and_initialize(
            const G& g_,
            const INDEX target_row,
            CONTAINER_CREATOR_PACK container_creator_pack = {}
        ) {
            if constexpr (debug_mode) {
                if (target_row >= g_.grid_down_cnt) {
                    throw std::runtime_error { "Slice too far down" };
                }
            }
            bidi_walker ret {
                g_,
                target_row,
                container_creator_pack
            };
            return ret;
        }

        /**
         * Result of
         * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker::bidi_walker::bidi_walker::find.
         */
        struct find_result {
            /** Forward walk final path edge and overall path weight (of maximally-weighted path between the root node and `node`). */
            const slot<E, ED>& forward_slot;
            /** Reverse walk final path edge and overall path weight (of maximally-weighted path between the left node and `node`). */
            const slot<E, ED>& backward_slot;
        };

        /**
         * Get the final edge as well as the weight of the overall path, for both the path from the root node to `node` (forward walk) and
         * the path from the leaf node to `node` (reverse walk).
         *
         * The behavior of this function is undefined if ...
         *
         *  * `node` doesn't exist within `g`.
         *  * `node` is neither a node in the row targeted nor a resident node before the row targeted.
         *
         * @param node Node within graph. This node must be either a non-resident node within the row targeted or a resident node leading up
         *     to the row targeted.
         * @return Final edge within path and overall path weight (of maximally-weighted path) to `node`, for both directions.
         */
        find_result find(const N& node) {
            const auto& forward_slot { forward_walker_.find(node) };
            const auto& backward_slot { backward_walker.find(node) };
            return { forward_slot, backward_slot };
        }

        /**
         * A node accompanied by the
         * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker::bidi_walker::bidi_walker::find_result
         * instance for that node.
         */
        struct list_entry {
            /**
             * Node.
             */
            N node;
            /**
             * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker::bidi_walker::bidi_walker::find_result
             * for `node`.
             */
            find_result slots;
        };

        /**
         * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker::bidi_walker::bidi_walker::find
         * invoked on all nodes within the target row.
         *
         * @return Range, where each element contains the node and result of
         *     @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker::bidi_walker::bidi_walker::find
         *     for that node.
         */
        bidirectional_range_of_exact<list_entry> auto list() {
            return g.row_nodes(target_row)
                | std::views::transform([&](const N& n) {
                    return list_entry { n, find(n) };
                });
        }

        /**
         * Modified version of
         * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker::bidi_walker::bidi_walker::find_result
         * where each member are is a value object (copy) rather than a reference. This is useful for avoiding dangling references.
         */
        struct find_result_copy {
            /** Forward walk final path edge and overall path weight (of maximally-weighted path between the root node and `node`). */
            const slot<E, ED> forward_slot;
            /** Reverse walk final path edge and overall path weight (of maximally-weighted path between the left node and `node`). */
            const slot<E, ED> backward_slot;
        };

        /**
         * Determine the final edge as well as the weight of the overall path, for both the path from the root node to `node` (forward walk)
         * and the path from the leaf node to `node` (reverse walk).
         *
         * The behavior of this function is undefined if `node` doesn't exist within `g`.
         *
         * @param g Graph.
         * @param node Node within `g`.
         * @return Final edge within path and overall path weight (of maximally-weighted path) to `node`, for both directions.
         */
        static find_result_copy converge(
            const G& g,
            const N& node
        ) {
            const auto& [down, right, depth] { g.node_to_grid_offset(node) };
            bidi_walker bidi_walker_ { bidi_walker::create_and_initialize(g, down) };
            find_result found { bidi_walker_.find(node) };
            return find_result_copy { found.forward_slot, found.backward_slot };
        }

        /**
         * Equivalent to invoking `converge(g, node)` and summing the forward direction's overall path weight with the reverse direction's
         * overall path weight.
         *
         * The behavior of this function is undefined if `node` doesn't exist within `g`.
         *
         * @param g Graph.
         * @param node Node within `g`.
         * @return Overall path weight (of maximally-weighted path) to `node`, for both directions, summed.
         */
        static ED converge_weight(
            const G& g,
            const N& node
        ) {
            find_result_copy slots { converge(g, node) };
            return slots.forward_slot.backtracking_weight + slots.backward_slot.backtracking_weight;
        }

        /**
         * Test if `node` sits on any of the maximally-weighted path between `g`'s root node and leaf node. That is, there may be
         * multiple maximally-weighted paths between `g`'s root node and leaf node. As long as `node` sits within one of them, this function
         * returns `true`.
         *
         * The behavior of this function is undefined if ...
         *
         *  * `node` doesn't exist within `g`.
         *  * `g` contains edges with non-finite weights.
         *  * `max_path_weight_comparison_tolerance` is not a finite value.
         *  * `max_path_weight` is not a finite value.
         *
         * @param g Graph.
         * @param node Node within `g`.
         * @param max_path_weight Weight of the maximally-weighted path between `g`'s root node and leaf node (must be finite). If there are
         *     multiple, they all should be the same weight.
         * @param max_path_weight_comparison_tolerance Tolerance used when testing for weight for equality. This may need to be non-zero
         *     when the type used for edge weights is a floating point type (must be finite). It helps mitigate floating point rounding
         *     errors when `g` is large / has large magnitude differences across `g`'s edge weights. The value this should be set to depends
         *     on multiple factors (e.g., which floating point type is used, expected graph size, expected magnitudes, etc..).
         * @return `true` if `node` sits on any of the maximally-weighted path between `g`'s root node and leaf node, `false` otherwise.
         */
        static bool is_node_on_max_path(
            const G& g,
            const N& node,
            const ED max_path_weight,
            const ED max_path_weight_comparison_tolerance
        ) {
            if constexpr (debug_mode) {
                if (!std::isfinite(max_path_weight)) {
                    throw std::runtime_error { "Max weight not finite" };
                }
                if (!std::isfinite(max_path_weight_comparison_tolerance)) {
                    throw std::runtime_error { "Tolerance not finite" };
                }
            }

            const auto& [down, right, depth] { g.node_to_grid_offset(node) };

            bidi_walker bidi_walker_ { bidi_walker::create_and_initialize(g, down) };
            for (const auto& entry : bidi_walker_.list()) {
                ED node_converged_weight { entry.slots.forward_slot.backtracking_weight + entry.slots.backward_slot.backtracking_weight };
                if (std::abs(node_converged_weight -  max_path_weight) <= max_path_weight_comparison_tolerance) {
                    return true;
                }
            }
            return false;
        }

    private:
        bidi_walker(
            const G& g_,
            const INDEX target_row_,
            CONTAINER_CREATOR_PACK container_creator_pack_ = {}
        )
        : g { g_ }
        , target_row { target_row_ }
        , reversed_g { g }
        , forward_walker_ {
            decltype(forward_walker_)::create_and_initialize(
                g,
                target_row,
                container_creator_pack_.create_forward_walker_container_creator_pack()
            )
        }
        , backward_walker {
            decltype(backward_walker)::create_and_initialize(
                reversed_g,
                g.grid_down_cnt - I1 - target_row,
                container_creator_pack_.create_backward_walker_container_creator_pack()
            )
        } {}
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BIDI_WALKER_BIDI_WALKER_H
