#ifndef OFFBYNULL_ALIGNER_BACKTRACK_SLICED_WALKER_H
#define OFFBYNULL_ALIGNER_BACKTRACK_SLICED_WALKER_H

#include <functional>
#include <ranges>
#include <algorithm>
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/backtrack/container_creator.h"
#include "offbynull/aligner/backtrack/container_creators.h"
#include "offbynull/aligner/backtrack/ready_queue.h"
#include "offbynull/aligner/backtrack/slot_container.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/concepts.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::backtrack::sliced_walker {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_parwise_alignment_graph;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::backtrack::ready_queue::ready_queue;
    using offbynull::aligner::backtrack::container_creator::container_creator;
    using offbynull::aligner::backtrack::container_creators::vector_container_creator;
    using offbynull::concepts::range_of_type;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::utils::max_element;

    template<typename N, weight WEIGHT>
    struct slot {
        N node;
        WEIGHT backtracking_weight;

        slot(N node_, WEIGHT backtracking_weight_)
        : node{node_}
        , backtracking_weight{backtracking_weight_} {}

        slot(N node_)
        : slot{node_, {}} {}
    };

    template<typename N, weight WEIGHT>
    struct slots_comparator {
        bool operator()(const slot<N, WEIGHT>& lhs, const slot<N, WEIGHT>& rhs) const noexcept {
            return lhs.node < rhs.node;
        }

        bool operator()(const slot<N, WEIGHT>& lhs, const N& rhs) const noexcept {
            return lhs.node < rhs;
        }

        bool operator()(const N& lhs, const slot<N, WEIGHT>& rhs) const noexcept {
            return lhs < rhs.node;
        }
    };

    template<
        readable_sliceable_parwise_alignment_graph G,
        weight WEIGHT,
        container_creator SLICE_SLOT_ALLOCATOR=vector_container_creator<slot<typename G::N, WEIGHT>>,
        container_creator RESIDENT_SLOT_ALLOCATOR=vector_container_creator<slot<typename G::N, WEIGHT>>,
        bool error_check = true
    >
    requires requires(typename G::N n)
    {
        {n < n} -> std::same_as<bool>;
    }
    class sliced_walker {
    private:
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using INDEX = typename G::INDEX;
        using SLICE_SLOT_CONTAINER=decltype(std::declval<SLICE_SLOT_ALLOCATOR>().create_empty(0zu));
        using RESIDENT_SLOT_CONTAINER=decltype(std::declval<RESIDENT_SLOT_ALLOCATOR>().create_empty(0zu));
        G& graph;
        std::function<WEIGHT(const E&)> get_edge_weight_func;
        RESIDENT_SLOT_CONTAINER resident_slots;
        SLICE_SLOT_CONTAINER lower_slots;
        SLICE_SLOT_CONTAINER upper_slots;
        INDEX grid_down;
        slot<N, WEIGHT>* active_slot_ptr;
        slot<N, WEIGHT>* next_slot_ptr;

        static std::optional<std::reference_wrapper<slot<N, WEIGHT>>> find_within_slots(auto& slots, const auto& node) {
            auto it { std::lower_bound(slots.begin(), slots.end(), node, slots_comparator<N, WEIGHT>{}) };
            if (it != slots.end() && (*it).node == node) {
                return { *it };
            }
            return { std::nullopt };
        }

        bool is_resident(const N& node) {
            return find_within_slots(resident_slots, node).has_value();
        }

        slot<N, WEIGHT>& find_slot(const N& node) {
            using FOUND_SLOT_REF = std::optional<std::reference_wrapper<slot<N, WEIGHT>>>;
            FOUND_SLOT_REF found_resident_slot_ref { find_within_slots(resident_slots, node) };
            FOUND_SLOT_REF found_prev_slot_ref { find_within_slots(lower_slots, node) };
            FOUND_SLOT_REF found_next_slot_ref { find_within_slots(upper_slots, node) };
            auto found_slot_range {
                std::array<FOUND_SLOT_REF, 3> {
                    found_resident_slot_ref,
                    found_prev_slot_ref,
                    found_next_slot_ref
                }
                | std::views::filter([](const auto& v) { return v.has_value(); })
                | std::views::take(1u)
                | std::views::transform([](const auto& v) { return *v; })
            };
            if constexpr (error_check) {
                if (found_slot_range.begin() == found_slot_range.end()) {
                    throw std::runtime_error("Node not found");
                }
            }
            slot<N, WEIGHT>& found_slot {
                (
                    *(found_slot_range.begin())
                ).get()
            };
            return found_slot;
        }

    public:
        sliced_walker(
            G& graph_,
            std::function<WEIGHT(const E&)> get_edge_weight_func_,
            SLICE_SLOT_ALLOCATOR slice_slot_container_creator = {},
            RESIDENT_SLOT_ALLOCATOR resident_slot_container_creator = {}
        )
        : graph{graph_}
        , get_edge_weight_func{get_edge_weight_func_}
        , resident_slots{resident_slot_container_creator.create_empty(std::nullopt)}
        , lower_slots{slice_slot_container_creator.create_empty(std::nullopt)}
        , upper_slots{slice_slot_container_creator.create_empty(std::nullopt)}
        , grid_down{0u}
        , active_slot_ptr{nullptr}
        , next_slot_ptr{nullptr} {
            auto&& _resident_slots { graph.resident_nodes() };
            std::ranges::copy(_resident_slots.cbegin(), _resident_slots.cend(), std::back_inserter(resident_slots));
            std::ranges::sort(resident_slots.begin(), resident_slots.end(), slots_comparator<N, WEIGHT>{});
            auto&& _upper_slots { graph.slice_nodes(0u) };
            std::ranges::copy(_upper_slots.cbegin(), _upper_slots.cend(), std::back_inserter(upper_slots));
            std::ranges::sort(upper_slots.begin(), upper_slots.end(), slots_comparator<N, WEIGHT>{});
            next_slot_ptr = &find_slot(graph.get_root_node());
        }

        slot<N, WEIGHT> active_slot() {
            return *this->active_slot_ptr;
        }

        const SLICE_SLOT_CONTAINER& active_slots() {
            return upper_slots;
        }

        bool next() {
            if (next_slot_ptr == nullptr) {
                return true;
            }
            active_slot_ptr = next_slot_ptr;
            // Compute only if node is not a resident. A resident node's backtracking weight + backtracking edge should
            // be computed as its inputs are walked over one-by-one by this function (see block below this one).
            if (!is_resident(active_slot_ptr->node)) {
                auto incoming_accumulated {
                    std::views::common(
                        graph.get_inputs(active_slot_ptr->node)
                        | std::views::transform(
                            [&](const auto& edge) noexcept -> std::pair<E, WEIGHT> {
                                const N& n_from { graph.get_edge_from(edge) };
                                const WEIGHT& edge_weight { get_edge_weight_func(edge) };
                                slot<N, WEIGHT>& n_from_slot { find_slot(n_from) };
                                return { edge, n_from_slot.backtracking_weight + edge_weight };
                            }
                        )
                    )
                };
                auto found {
                    // Can't use std::max_element / std::ranges::max_element because it requires a forward iterator
                    // Ensure range is a common_view (begin() and end() are of same type)
                    max_element(
                        incoming_accumulated.begin(),
                        incoming_accumulated.end(),
                        [](const std::pair<E, WEIGHT>& a, const std::pair<E, WEIGHT>& b) noexcept {
                            return a.second < b.second;
                        }
                    )
                };
                if (found != incoming_accumulated.end()) {  // if no incoming nodes found, it's a root node
                    // active_slot.backtracking_edge = (*found).first;
                    active_slot_ptr->backtracking_weight = (*found).second;
                }
            }

            // Update resident node weights
            for (const E& edge : graph.outputs_to_residents(active_slot_ptr->node)) {
                const N& resident_node { graph.get_edge_to(edge) };
                std::optional<std::reference_wrapper<slot<N, WEIGHT>>> resident_slot_maybe {
                    find_within_slots(resident_slots, resident_node)
                };
                if constexpr (error_check) {
                    if (!resident_slot_maybe.has_value()) {
                        throw std::runtime_error("This should never happen");
                    }
                }
                slot<N, WEIGHT>& resident_slot { (*resident_slot_maybe).get() };
                const WEIGHT& edge_weight { get_edge_weight_func(edge) };
                const WEIGHT& new_weight { active_slot_ptr->backtracking_weight + edge_weight };
                if (new_weight > resident_slot.backtracking_weight) {
                    resident_slot.backtracking_weight = new_weight;
                }
            }

            // Move to next node / next slice
            if (graph.slice_last_node(grid_down) != active_slot_ptr->node) {
                next_slot_ptr = &find_slot(graph.slice_next_node(active_slot_ptr->node));
            } else {
                if (grid_down == graph.grid_down_cnt - 1u) {
                    next_slot_ptr = nullptr;
                    return true;
                }
                grid_down++;
                lower_slots.clear();
                std::ranges::copy(upper_slots.begin(), upper_slots.end(), std::back_inserter(lower_slots));
                auto&& _upper_slots { graph.slice_nodes(grid_down) };
                upper_slots.clear();
                std::ranges::copy(_upper_slots.begin(), _upper_slots.end(), std::back_inserter(upper_slots));
                std::ranges::sort(upper_slots.begin(), upper_slots.end(), slots_comparator<N, WEIGHT>{});
                next_slot_ptr = &find_slot(graph.slice_first_node(grid_down));
                active_slot_ptr = &find_slot(graph.slice_last_node(grid_down - 1u)); // need to update this because slots entries have moved around
            }

            return false;
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACK_SLICED_WALKER_H
