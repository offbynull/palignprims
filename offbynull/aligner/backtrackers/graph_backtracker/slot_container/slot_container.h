#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_SLOT_CONTAINER_SLOT_CONTAINER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_SLOT_CONTAINER_SLOT_CONTAINER_H

#include <cstddef>
#include <ranges>
#include <vector>
#include <iterator>
#include <algorithm>
#include <limits>
#include <utility>
#include <stdexcept>
#include "offbynull/aligner/backtrackers/graph_backtracker/backtrackable_node.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/backtrackable_edge.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/concepts.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/slot_container/slot.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/slot_container/slot_comparator.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/slot_container/slot_container_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/slot_container/slot_container_heap_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/slot_container/slot_container_stack_container_creator_pack.h"

namespace offbynull::aligner::backtrackers::graph_backtracker::slot_container::slot_container {
    using offbynull::aligner::concepts::weight;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::concepts::input_iterator_of_non_cvref;
    using offbynull::aligner::graph::graph::graph;
    using offbynull::aligner::backtrackers::graph_backtracker::backtrackable_node::backtrackable_node;
    using offbynull::aligner::backtrackers::graph_backtracker::backtrackable_edge::backtrackable_edge;
    using offbynull::aligner::backtrackers::graph_backtracker::slot_container::slot::slot;
    using offbynull::aligner::backtrackers::graph_backtracker::slot_container::slot_comparator::slot_comparator;
    using offbynull::aligner::backtrackers::graph_backtracker::slot_container::slot_container_container_creator_pack
        ::slot_container_container_creator_pack;
    using offbynull::aligner::backtrackers::graph_backtracker::slot_container::slot_container_heap_container_creator_pack
        ::slot_container_heap_container_creator_pack;

    /**
     * Container of @ref offbynull::aligner::backtrackers::graph_backtracker::slot_container::slot::slot "slots", used by
     * @ref offbynull::aligner::backtrackers::graph_backtracker::backtracker::backtracker to track the backtracking state of each node
     * within a graph.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam G Graph type.
     * @tparam WEIGHT Graph edge's weight type.
     * @tparam CONTAINER_CREATOR_PACK Container factory type.
     */
    template<
        bool debug_mode,
        graph G,
        weight WEIGHT,
        slot_container_container_creator_pack<
            typename G::N,
            typename G::E,
            WEIGHT
        > CONTAINER_CREATOR_PACK = slot_container_heap_container_creator_pack<
            debug_mode,
            typename G::N,
            typename G::E,
            WEIGHT
        >
    >
    class slot_container {
    private:
        using N = typename G::N;
        using E = typename G::E;

        using SLOT_CONTAINER = decltype(
            std::declval<CONTAINER_CREATOR_PACK>().create_slot_container(
                std::declval<std::vector<slot<N, E, WEIGHT>>>().begin(),
                std::declval<std::vector<slot<N, E, WEIGHT>>>().end()
            )
        );

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
         * Construct an @ref offbynull::aligner::backtrackers::graph_backtracker::slot_container::slot_container::slot_container instance.
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
         * @param container_creator_pack Container factory.
         */
        slot_container(
            const G& g_,
            input_iterator_of_non_cvref<slot<N, E, WEIGHT>> auto begin,
            std::sentinel_for<decltype(begin)> auto end,
            CONTAINER_CREATOR_PACK container_creator_pack = {}
        )
        : g { g_ }
        , slots { container_creator_pack.create_slot_container(begin, end) } {
            std::ranges::sort(
                slots.begin(),
                slots.end(),
                slot_comparator<N, E, WEIGHT> {}
            );
        }

        // TODO: Instead of passing start and end iterators in the constructor above, directly initialize from g_?

        /**
         * Get index of slot assigned to some node.
         *
         * If no slot exists for `node`, the behavior of this function is undefined.
         *
         * @param node Identifier of node to find.
         * @return Index of slot assigned to `node`.
         */
        std::size_t find_idx(const N& node) {
            auto it { std::lower_bound(slots.begin(), slots.end(), node, slot_comparator<N, E, WEIGHT> {}) };
            if constexpr (debug_mode) {
                if (it == slots.end() || (*it).node != node) {
                    throw std::runtime_error { "Not found" };
                }
            }
            return it - slots.begin();
        }

        /**
         * Get reference to slot assigned to some node.
         *
         * If no slot exists for `node`, the behavior of this function is undefined.
         *
         * @param node Identifier of node to find.
         * @return Reference to slot assigned to `node`.
         */
        slot<N, E, WEIGHT>& find_ref(const N& node) {
            auto it { std::lower_bound(slots.begin(), slots.end(), node, slot_comparator<N, E, WEIGHT> {}) };
            if constexpr (debug_mode) {
                if (it == slots.end() || (*it).node != node) {
                    throw std::runtime_error { "Not found" };
                }
            }
            return *it;
        }

        /**
         * Get reference to slot at some index.
         *
         * If `idx` is out of bounds, the behaviour of this function undefined.
         *
         * @param idx Index of slot
         * @return Reference to slot at `idx`.
         */
        slot<N, E, WEIGHT>& at_idx(const std::size_t idx) {
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
         * If no slot exists for `node`, the behavior of this function is undefined.
         *
         * @param node Identifier of node to find.
         * @return Index of and reference to slot assigned to `node`.
         */
        std::pair<std::size_t, slot<N, E, WEIGHT>&> find(const N& node) {
            auto it { std::lower_bound(slots.begin(), slots.end(), node, slot_comparator<N, E, WEIGHT> {}) };
            if constexpr (debug_mode) {
                if (it == slots.end() || (*it).node != node) {
                    throw std::runtime_error { "Not found" };
                }
            }
            auto dist_from_beginning { std::ranges::distance(slots.begin(), it) };
            // It may be that dist_from_beginning is signed, in which case the widenable_to_size_t test below fails (it tests for
            // unsignedness in addition testing to see if widenable). If it is signed, the test checkd the max value of both types. If
            // max_value(decltype(dist_from_beginning)) < max_value(size_t), it's safe to do a static_cast to size_t because
            // dist_from_beginning should never be a negative value.
            if constexpr (debug_mode && !widenable_to_size_t<decltype(dist_from_beginning)>) {
                if constexpr (std::numeric_limits<decltype(dist_from_beginning)>::max() > std::numeric_limits<std::size_t>::max()) {
                    if (dist_from_beginning > std::numeric_limits<std::size_t>::max()) {
                        throw std::runtime_error { "Narrowing of distance will result in information loss" };
                    }
                }
            }
            std::size_t idx { static_cast<std::size_t>(dist_from_beginning) };
            slot<N, E, WEIGHT>& slot { *it };
            return { idx, slot };
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_SLOT_CONTAINER_SLOT_CONTAINER_H
