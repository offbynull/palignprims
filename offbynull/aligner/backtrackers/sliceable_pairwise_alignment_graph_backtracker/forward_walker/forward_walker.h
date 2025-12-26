#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_FORWARD_WALKER_FORWARD_WALKER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_FORWARD_WALKER_FORWARD_WALKER_H

#include <functional>
#include <ranges>
#include <optional>
#include <utility>
#include <stdexcept>
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtrackable_node.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtrackable_edge.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/row_slot_container/slot.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/row_slot_container/row_slot_container.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/row_slot_container/row_slot_container_pair.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/row_slot_container/row_slot_container_stack_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_slot_container/resident_slot_container.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_slot_container/resident_slot_with_node.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_slot_container/resident_slot.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_slot_container/resident_slot_container_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_slot_container/resident_slot_container_heap_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_slot_container/resident_slot_container_stack_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/forward_walker/forward_walker_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/forward_walker/forward_walker_heap_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/forward_walker/row_entry.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker::forward_walker {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::sliceable_pairwise_alignment_graph;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker
        ::forward_walker_container_creator_pack::forward_walker_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker
        ::forward_walker_heap_container_creator_pack::forward_walker_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker::row_entry::row_entry;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtrackable_node::backtrackable_node;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtrackable_edge::backtrackable_edge;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container::slot::slot;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container::row_slot_container
        ::row_slot_container;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container
        ::row_slot_container_container_creator_pack::row_slot_container_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container
        ::row_slot_container_heap_container_creator_pack::row_slot_container_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container
        ::row_slot_container_stack_container_creator_pack::row_slot_container_stack_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container
        ::row_slot_container_pair::row_slot_container_pair;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container
        ::resident_slot_with_node::resident_slot_with_node;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container::resident_slot
        ::resident_slot;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container
        ::resident_slot_container::resident_slot_container;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container
        ::resident_slot_container_container_creator_pack::resident_slot_container_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container
        ::resident_slot_container_heap_container_creator_pack::resident_slot_container_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container
        ::resident_slot_container_stack_container_creator_pack::resident_slot_container_stack_container_creator_pack;
    using offbynull::aligner::concepts::weight;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::concepts::unqualified_object_type;

    /**
     * Forward walker for @ref offbynull::aligner::graph::sliceable_pairwise_alignment_graph::sliceable_pairwise_alignment_graph
     * implementations. A forward walker walks the graph from the root node to some destination node, calculating the weight of the
     * maximally-weighted path between those two nodes (path with the highest sum of edge weights).
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam G Graph type.
     * @tparam CONTAINER_CREATOR_PACK Container factory type.
     */
    template<
        bool debug_mode,
        sliceable_pairwise_alignment_graph G,
        forward_walker_container_creator_pack<
            typename G::N,
            typename G::E,
            typename G::ED
        > CONTAINER_CREATOR_PACK = forward_walker_heap_container_creator_pack<
            debug_mode,
            typename G::N,
            typename G::E,
            typename G::ED,
            true
        >
    >
    class forward_walker {
    private:
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using N_INDEX = typename G::N_INDEX;

        static constexpr N_INDEX I0 { static_cast<N_INDEX>(0zu) };
        static constexpr N_INDEX I1 { static_cast<N_INDEX>(1zu) };

        using ROW_SLOT_CONTAINER_CONTAINER_CREATOR_PACK =
            decltype(std::declval<CONTAINER_CREATOR_PACK>().create_row_slot_container_container_creator_pack());
        using RESIDENT_SLOT_CONTAINER_CONTAINER_CREATOR_PACK =
            decltype(std::declval<CONTAINER_CREATOR_PACK>().create_resident_slot_container_container_creator_pack());

        const G& g;
        resident_slot_container<debug_mode, G, RESIDENT_SLOT_CONTAINER_CONTAINER_CREATOR_PACK> resident_slots;
        row_slot_container_pair<debug_mode, G, ROW_SLOT_CONTAINER_CONTAINER_CREATOR_PACK> row_slots;
        decltype(g.row_nodes(0zu)) row;
        decltype(row.begin()) row_it;
        row_entry<N, E, ED> row_entry_;

    public:
        /**
         * Create an @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker::forward_walker
         * instance targeted at nodes within a specific row of the graph. For example, consider a scenario where `target_row` is 5. For each
         * node N in `g`'s 5th row, the returned object will have the weight of the maximally-weighted path between the root node and N.
         *
         * The behavior of this function is undefined if `target_row` is past the final row within `g`.
         *
         * @param g_ Graph.
         * @param target_row Row within `g`.
         * @param container_creator_pack_ Container factory.
         * @return @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker::forward_walker
         *     instance primed to `g`'s `target_row` row.
         */
        static forward_walker create_and_initialize(
            const G& g_,
            const N_INDEX target_row,
            CONTAINER_CREATOR_PACK container_creator_pack_ = {}
        ) {
            if constexpr (debug_mode) {
                if (target_row >= g_.grid_down_cnt) {
                    throw std::runtime_error { "Slice too far down" };
                }
            }
            forward_walker ret {
                g_,
                container_creator_pack_
            };
            while (ret.row_slots.down_offset() != target_row || ret.row_it != ret.row.end()) {
                ret.step_forward();
            }
            return ret;
        }

        /**
         * For the maximally-weighted path from the root node to `node` (path with the highest sum of edge weights), get the final edge of
         * that path (last edge before reaching `node`) as well as the weight of that path.
         *
         * The behavior of this function is undefined if ...
         *
         *  * `node` doesn't exist within `g`.
         *  * `node` is neither a node in the row targeted nor a resident node before the row targeted.
         *
         * @param node Idntifier of node within graph. This node must be either a non-resident node within the row targeted or a resident
         *     node leading up to the row targeted.
         * @return Identifier of final edge within path and overall path weight (of maximally-weighted path between the root node and
         *     `node`).
         */
        slot<E, ED>& find(const N& node) {
            auto found_resident { resident_slots.find(node) };
            if (found_resident.has_value()) {
                return (*found_resident).get().slot_;
            }
            auto found_row { row_slots.find(node) };
            if (found_row.has_value()) {
                return *found_row;
            }
            if constexpr (debug_mode) {
                throw std::runtime_error { "Node not found" };
            }
            std::unreachable();
        }

    private:
        forward_walker(
            const G& g_,
            CONTAINER_CREATOR_PACK container_creator_pack_ = {}
        )
        : g { g_ }
        , resident_slots { g, container_creator_pack_.create_resident_slot_container_container_creator_pack() }
        , row_slots { g, container_creator_pack_.create_row_slot_container_container_creator_pack() }
        , row { g.row_nodes(I0) }
        , row_it { row.begin() }
        , row_entry_ {} {
            row_entry_.node = *row_it;
            row_entry_.slot_ptr = &find(row_entry_.node); // should be equivalent to g.get_root_node()
        }

        void step_forward() {
            if (row_it != row.end()) {
                row_entry_.node = *row_it;
                row_entry_.slot_ptr = &find(row_entry_.node);
            } else {
                if (row_slots.down_offset() == g.grid_down_cnt - I1) {
                    return;
                }
                row_slots.move_down();
                row = g.row_nodes(row_slots.down_offset());
                row_it = row.begin();
                row_entry_.node = *row_it;
                row_entry_.slot_ptr = &find(row_entry_.node);
            }
            // Compute only if node is not a resident. A resident node's backtracking weight + backtracking edge should
            // be computed as its inputs are walked over one-by-one by this function (see block below this one).
            if (!resident_slots.find(row_entry_.node).has_value()) {
                auto incoming_accumulated {
                    std::views::common(
                        g.get_inputs(row_entry_.node)
                        | std::views::transform(
                            [&](const auto& edge) -> std::pair<E, ED> {
                                const N& n_from { g.get_edge_from(edge) };
                                const ED& edge_weight { g.get_edge_data(edge) };
                                slot<E, ED>& n_from_slot { find(n_from) };
                                return {
                                    edge,
                                    static_cast<ED>(n_from_slot.backtracking_weight + edge_weight)  // Cast to prevent narrowing warning
                                };
                            }
                        )
                    )
                };
                auto found {
                    std::ranges::max_element(
                        incoming_accumulated.begin(),
                        incoming_accumulated.end(),
                        [](const std::pair<E, ED>& a, const std::pair<E, ED>& b) {
                            return a.second < b.second;
                        }
                    )
                };
                if (found != incoming_accumulated.end()) {  // if no incoming nodes found, it's a root node
                    row_entry_.slot_ptr->backtracking_edge = { (*found).first };
                    row_entry_.slot_ptr->backtracking_weight = (*found).second;
                }
            }

            // Update resident node weights
            for (const E& edge : g.outputs_to_residents(row_entry_.node)) {
                const N& resident_node { g.get_edge_to(edge) };
                std::optional<std::reference_wrapper<resident_slot<E, ED>>> resident_slot_maybe {
                    resident_slots.find(resident_node)
                };
                if constexpr (debug_mode) {
                    if (!resident_slot_maybe.has_value()) {
                        throw std::runtime_error { "This should never happen" };
                    }
                }
                resident_slot<E, ED>& resident_slot_ { (*resident_slot_maybe).get() };
                const ED& edge_weight { g.get_edge_data(edge) };
                if (!resident_slot_.initialized) {
                    resident_slot_.slot_.backtracking_edge = { edge };
                    resident_slot_.slot_.backtracking_weight = edge_weight;
                    resident_slot_.initialized = true;
                } else {
                    ED new_weight {
                        static_cast<ED>(row_entry_.slot_ptr->backtracking_weight + edge_weight)  // Cast to prevent narrowing warning
                    };
                    if (new_weight > resident_slot_.slot_.backtracking_weight) {
                        resident_slot_.slot_.backtracking_edge = { edge };
                        resident_slot_.slot_.backtracking_weight = new_weight;
                    }
                }
            }

            // Move to next node
            ++row_it;
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_FORWARD_WALKER_FORWARD_WALKER_H
