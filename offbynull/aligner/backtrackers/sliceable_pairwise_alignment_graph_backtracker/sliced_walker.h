#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLICED_WALKER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLICED_WALKER_H

#include <functional>
#include <ranges>
#include <algorithm>
#include "offbynull/aligner/concepts.h"
#include "offbynull/helpers/container_creators.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/slot.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/slice_slot_container.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_slot_container.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::sliced_walker {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::slot::slot;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::slice_slot_container::slice_slot_container;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container::node_searchable_slot;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container::resident_slot_container;
    using offbynull::aligner::concepts::weight;
    using offbynull::helpers::container_creators::container_creator;
    using offbynull::helpers::container_creators::vector_container_creator;
    using offbynull::concepts::range_of_type;
    using offbynull::concepts::widenable_to_size_t;

    template<typename N, typename E, weight WEIGHT>
    struct slice_entry {
        N node;
        slot<E, WEIGHT>* slot_ptr;

        slice_entry()
        : node{}
        , slot_ptr{nullptr} {}
    };

    template<
        readable_sliceable_pairwise_alignment_graph G,
        container_creator SLICE_SLOT_CONTAINER_CREATOR,
        bool error_check
    >
    struct slice_slot_container_pair {
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using INDEX = typename G::INDEX;

        slice_slot_container<G, SLICE_SLOT_CONTAINER_CREATOR, error_check> slots1;
        slice_slot_container<G, SLICE_SLOT_CONTAINER_CREATOR, error_check> slots2;
        slice_slot_container<G, SLICE_SLOT_CONTAINER_CREATOR, error_check>* previous_slots;  // row above current row
        slice_slot_container<G, SLICE_SLOT_CONTAINER_CREATOR, error_check>* current_slots;  // current row
        INDEX grid_down_offset;

        slice_slot_container_pair(
            G& graph,
            SLICE_SLOT_CONTAINER_CREATOR slice_slot_container_creator
        )
        : slots1{graph, slice_slot_container_creator}
        , slots2{graph, slice_slot_container_creator}
        , previous_slots{&slots1}
        , current_slots{&slots2}
        , grid_down_offset{0u} {}

        void move_down() {
            grid_down_offset++;

            auto* old_previous_slots { previous_slots };
            previous_slots = current_slots;
            current_slots = old_previous_slots;

            current_slots->reset(grid_down_offset);
        }

        std::optional<std::reference_wrapper<slot<E, ED>>> find(const N& node) {
            auto found_lower { current_slots->find(node) };
            if (found_lower.has_value()) {
                return { found_lower };
            }
            auto found_upper { previous_slots->find(node) };
            if (found_upper.has_value()) {
                return { found_upper };
            }
            return { std::nullopt };
        }
    };

    template<
        readable_sliceable_pairwise_alignment_graph G,
        container_creator SLICE_SLOT_CONTAINER_CREATOR=vector_container_creator<
            slot<
                typename G::E,
                typename G::ED
            >
        >,
        container_creator RESIDENT_SLOT_CONTAINER_CREATOR=vector_container_creator<
            node_searchable_slot<
                typename G::N,
                typename G::E,
                typename G::ED
            >
        >,
        bool error_check = true
    >
    class sliced_walker {
    private:
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using INDEX = typename G::INDEX;
        G& graph;
        resident_slot_container<G, RESIDENT_SLOT_CONTAINER_CREATOR, error_check> resident_slots;
        slice_slot_container_pair<G, SLICE_SLOT_CONTAINER_CREATOR, error_check> slice_slots;
        slice_entry<N, E, ED> current_slice_entry;
        slice_entry<N, E, ED> next_slice_entry;

    public:
        sliced_walker(
            G& graph_,
            SLICE_SLOT_CONTAINER_CREATOR slice_slot_container_creator = {},
            RESIDENT_SLOT_CONTAINER_CREATOR resident_slot_container_creator = {}
        )
        : graph{graph_}
        , resident_slots{graph, resident_slot_container_creator}
        , slice_slots{graph, slice_slot_container_creator}
        , current_slice_entry{}
        , next_slice_entry{}  {
            auto&& _resident_slots { graph.resident_nodes() };
            // previous_slots.reset(0u);  // Should be implicit
            next_slice_entry.node = graph.slice_first_node(0u);
            next_slice_entry.slot_ptr = &find(next_slice_entry.node); // should be equivalent to graph.get_root_node()
        }

        slot<E, ED>& find(const N& node) {
            auto found_resident { resident_slots.find(node) };
            if (found_resident.has_value()) {
                return *found_resident;
            }
            auto found_slice { slice_slots.find(node) };
            if (found_slice.has_value()) {
                return *found_slice;
            }
            if constexpr (error_check) {
                throw std::runtime_error("Node not found");
            }
            std::unreachable();
        }

        bool next() {
            if (next_slice_entry.slot_ptr == nullptr) {
                return true;
            }
            current_slice_entry.node = next_slice_entry.node;
            current_slice_entry.slot_ptr = next_slice_entry.slot_ptr;
            // Compute only if node is not a resident. A resident node's backtracking weight + backtracking edge should
            // be computed as its inputs are walked over one-by-one by this function (see block below this one).
            if (!resident_slots.find(current_slice_entry.node).has_value()) {
                auto incoming_accumulated {
                    std::views::common(
                        graph.get_inputs(current_slice_entry.node)
                        | std::views::transform(
                            [&](const auto& edge) noexcept -> std::pair<E, ED> {
                                const N& n_from { graph.get_edge_from(edge) };
                                const ED& edge_weight { graph.get_edge_data(edge) };
                                slot<E, ED>& n_from_slot { find(n_from) };
                                return { edge, n_from_slot.backtracking_weight + edge_weight };
                            }
                        )
                    )
                };
                auto found {
                    std::ranges::max_element(
                        incoming_accumulated.begin(),
                        incoming_accumulated.end(),
                        [](const std::pair<E, ED>& a, const std::pair<E, ED>& b) noexcept {
                            return a.second < b.second;
                        }
                    )
                };
                if (found != incoming_accumulated.end()) {  // if no incoming nodes found, it's a root node
                    current_slice_entry.slot_ptr->backtracking_edge = (*found).first;
                    current_slice_entry.slot_ptr->backtracking_weight = (*found).second;
                }
            }

            // Update resident node weights
            for (const E& edge : graph.outputs_to_residents(current_slice_entry.node)) {
                const N& resident_node { graph.get_edge_to(edge) };
                std::optional<std::reference_wrapper<slot<E, ED>>> resident_slot_maybe {
                    resident_slots.find(resident_node)
                };
                if constexpr (error_check) {
                    if (!resident_slot_maybe.has_value()) {
                        throw std::runtime_error("This should never happen");
                    }
                }
                slot<E, ED>& resident_slot { (*resident_slot_maybe).get() };
                const ED& edge_weight { graph.get_edge_data(edge) };
                const ED& new_weight { current_slice_entry.slot_ptr->backtracking_weight + edge_weight };
                if (new_weight > resident_slot.backtracking_weight) {
                    resident_slot.backtracking_weight = new_weight;
                }
            }

            // Move to next node / next slice
            bool at_last_node_in_slice {
                current_slice_entry.node == graph.slice_last_node(slice_slots.grid_down_offset)
            };
            if (!at_last_node_in_slice) {
                next_slice_entry.node = graph.slice_next_node(current_slice_entry.node);
                next_slice_entry.slot_ptr = &find(next_slice_entry.node);
            } else {
                if (slice_slots.grid_down_offset == graph.grid_down_cnt - 1u) {
                    next_slice_entry.slot_ptr = nullptr;
                    return true;
                }
                slice_slots.move_down();
                next_slice_entry.node = graph.slice_first_node(slice_slots.grid_down_offset);
                next_slice_entry.slot_ptr = &find(next_slice_entry.node);
                current_slice_entry.node = graph.slice_last_node(slice_slots.grid_down_offset - 1u); // need to update this because slots entries have moved around
                current_slice_entry.slot_ptr = &find(current_slice_entry.node);
            }

            return false;
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLICED_WALKER_H
