#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLOT_CONTAINER_SLOT_CONTAINER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLOT_CONTAINER_SLOT_CONTAINER_H

#include <cstddef>
#include <iterator>
#include <utility>
#include <limits>
#include <stdexcept>
#include <ranges>
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/slot_container/slot.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/slot_container/slot_container_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/slot_container/slot_container_heap_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtrackable_node.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtrackable_edge.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/concepts.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container::slot_container {
    using offbynull::aligner::concepts::weight;
    using offbynull::concepts::unqualified_object_type;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::aligner::graph::pairwise_alignment_graph::pairwise_alignment_graph;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container::slot::slot;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container::slot_container_container_creator_pack
        ::slot_container_container_creator_pack;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container::slot_container_heap_container_creator_pack
        ::slot_container_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtrackable_node::backtrackable_node;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtrackable_edge::backtrackable_edge;
    using offbynull::utils::check_multiplication_nonoverflow_throwable;
    using offbynull::concepts::input_iterator_of_non_cvref;

    /**
     * Container of @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container::slot::slot "slots", used by
     * @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker::backtracker to track the backtracking state
     * of each node within a graph.
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
        slot_container_container_creator_pack<
            typename G::N,
            typename G::E,
            typename G::ED,
            PARENT_COUNT,
            SLOT_INDEX
        > CONTAINER_CREATOR_PACK = slot_container_heap_container_creator_pack<
            debug_mode,
            typename G::N,
            typename G::E,
            typename G::ED,
            PARENT_COUNT,
            SLOT_INDEX
        >
    >
    class slot_container {
    private:
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using N_INDEX = typename G::N_INDEX;

        using SLOT_CONTAINER = decltype(std::declval<CONTAINER_CREATOR_PACK>().create_slot_container(0zu, 0zu, 0zu, std::declval<ED>()));

        const G& g;
        SLOT_CONTAINER slots;

    public:
        // Concepts for params have been commented out because THEY FAIL when you pass in a std::views::common(...)'s
        // iterator. Apparently the iterator doesn't contain ::value_type? Or the iterator that's passed in isn't
        // default constructible (there doesn't seem to be any requirement that an iterator be default constructible,
        // but the iterator is a concat_bidirectional_view which is being wrapped in a std::views::common() and there's some weird
        // concepts checking to make sure things are default constructible / ::value_type isn't making it through?
        //
        // https://www.reddit.com/r/cpp_questions/comments/1d5z7sh/bizarre_requirement_of_stdinput_iterator/
        /**
         * Construct an
         * @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container::slot_container::slot_container
         * instance.
         *
         * The behavior of this function is undefined if any of the following conditions are met:
         *
         *  * `begin` and `end` aren't from the same owner.
         *  * `begin > end`.
         *  * slots returned by `begin` / `end` point to nodes in some graph other than `g_`.
         *
         * @param g_ Graph.
         * @param begin Start iterator containing slots for `g_` (elements must be initialized to each node within `g_`, where no node's
         *     parents have been walked yet).
         * @param end End iterator containing slots for `g_` (elements must be initialized to each node within `g_`, where no node's parents
         *     have been walked yet).
         * @param zero_weight Initial edge weight, equivalent to 0 for numeric weights.
         * @param container_creator_pack Container factory.
         */
        slot_container(
            const G& g_,
            input_iterator_of_non_cvref<slot<N, E, ED, PARENT_COUNT>> auto begin,
            std::sentinel_for<decltype(begin)> auto end,
            ED zero_weight,
            CONTAINER_CREATOR_PACK container_creator_pack = {}
        )
        : g { g_ }
        , slots {
            container_creator_pack.create_slot_container(
                g.grid_down_cnt,
                g.grid_right_cnt,
                g.grid_depth_cnt,
                zero_weight
            )
        } {
            if constexpr (debug_mode) {
                check_multiplication_nonoverflow_throwable<SLOT_INDEX>(g.grid_down_cnt, g.grid_right_cnt, g.grid_depth_cnt);
                if (std::numeric_limits<SLOT_INDEX>::max() < g.grid_down_cnt * g.grid_right_cnt * g.grid_depth_cnt) {
                    throw std::runtime_error { "SLOT_INDEX not wide enough to support grid_down_cnt * grid_right_cnt * grid_depth_cnt" };
                }
                if (std::numeric_limits<PARENT_COUNT>::max() < g.node_incoming_edge_capacity) {
                    throw std::runtime_error { "PARENT_COUNT not wide enough to support node_incoming_edge_capacity" };
                }
            }
            auto it { begin };
            while (it != end) {
                const auto& slot { *it };
                const auto& [down_offset, right_offset, depth] { g.node_to_grid_offset(slot.node) };
                std::size_t idx { (g.grid_depth_cnt * ((down_offset * g.grid_right_cnt) + right_offset)) + depth };
                slots[idx] = slot;
                ++it;
            }
        }

        /**
         * Get index of slot assigned to some node.
         *
         * If no slot exists for `node`, the behavior of this function is undefined.
         *
         * @param node Identifier of node to find.
         * @return Index of slot assigned to `node`.
         */
        SLOT_INDEX find_idx(const N& node) const {
            const auto& [down_offset, right_offset, depth] { g.node_to_grid_offset(node) };
            SLOT_INDEX ret {
                static_cast<SLOT_INDEX>(
                    (g.grid_depth_cnt * ((down_offset * g.grid_right_cnt) + right_offset)) + depth
                )  // Cast to prevent narrowing warning
            };
            if constexpr (debug_mode) {
                if (ret >= slots.size()) {
                    throw std::runtime_error { "Out of bounds" };
                }
            }
            return ret;
        }

        /**
         * Get reference to slot assigned to some node.
         *
         * If no slot exists for `node` (e.g., `node` does not belong to this container's graph), the behavior of this function is
         * undefined.
         *
         * @param node Identifier of node to find.
         * @return Reference to slot assigned to `node`.
         */
        slot<N, E, ED, PARENT_COUNT>& find_ref(const N& node) {
            std::size_t idx { find_idx(node) };
            slot<N, E, ED, PARENT_COUNT>& slot { slots[idx] };
            return slot;
        }

        /**
         * Get reference to slot assigned to some node.
         *
         * If no slot exists for `node` (e.g., `node` does not belong to this container's graph), the behavior of this function is
         * undefined.
         *
         * @param node Identifier of node to find.
         * @return Reference to slot assigned to `node`.
         */
        const slot<N, E, ED, PARENT_COUNT>& find_ref(const N& node) const {
            std::size_t idx { find_idx(node) };
            const slot<N, E, ED, PARENT_COUNT>& slot { slots[idx] };
            return slot;
        }

        /**
         * Get reference to slot at some index.
         *
         * If `idx`is out of bounds, the behaviour of this function undefined.
         *
         * @param idx Index of slot
         * @return Reference to slot at `idx`.
         */
        slot<N, E, ED, PARENT_COUNT>& at_idx(const SLOT_INDEX idx) {
            if constexpr (debug_mode) {
                if (idx >= slots.size()) {
                    throw std::runtime_error { "Out of bounds" };
                }
            }
            return slots[idx];
        }

        /**
         * Get reference to slot at some index.
         *
         * If `idx` is out of bounds, the behaviour of this function undefined.
         *
         * @param idx Index of slot
         * @return Reference to slot at `idx`.
         */
        const slot<N, E, ED, PARENT_COUNT>& at_idx(const SLOT_INDEX idx) const {
            if constexpr (debug_mode) {
                if (idx >= slots.size()) {
                    throw std::runtime_error { "Out of bounds" };
                }
            }
            return slots[idx];
        }

        /**
         * Get index of and reference to slot assigned to some node.
         *
         * If no slot exists for `node` (e.g., `node` does not belong to this container's graph), the behavior of this function is
         * undefined.
         *
         * @param node Identifier of node to find.
         * @return Index of and reference to slot assigned to `node`.
         */
        std::pair<SLOT_INDEX, slot<N, E, ED, PARENT_COUNT>&> find(const N& node) {
            SLOT_INDEX idx { find_idx(node) };
            slot<N, E, ED, PARENT_COUNT>& slot { slots[idx] };
            return { idx, slot };
        }

        /**
         * Get index of and reference to slot assigned to some node.
         *
         * If no slot exists for `node` (e.g., `node` does not belong to this container's graph), the behavior of this function is
         * undefined.
         *
         * @param node Identifier of node to find.
         * @return Index of and reference to slot assigned to `node`.
         */
        std::pair<SLOT_INDEX, const slot<N, E, ED, PARENT_COUNT>&> find(const N& node) const {
            SLOT_INDEX idx { find_idx(node) };
            const slot<N, E, ED, PARENT_COUNT>& slot { slots[idx] };
            return { idx, slot };
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLOT_CONTAINER_SLOT_CONTAINER_H
