#ifndef OFFBYNULL_ALIGNER_BACKTRACK_SLICED_WALKER_H
#define OFFBYNULL_ALIGNER_BACKTRACK_SLICED_WALKER_H

#include <cstddef>
#include <functional>
#include <ranges>
#include <algorithm>
#include "offbynull/aligner/graphs/pairwise_extended_gap_alignment_graph.h"
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
    using offbynull::aligner::backtrack::container_creators::static_vector_container_creator;
    using offbynull::concepts::range_of_type;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::utils::max_element;

    template<typename N, weight WEIGHT>
    struct slot {
        N node;
        WEIGHT backtracking_weight;

        slot(N node_)
        : node{node_}
        , backtracking_weight{} {}
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
        std::size_t slice_max_cnt,
        widenable_to_size_t COUNT,
        weight WEIGHT,
        bool error_check = true
    >
    requires requires(typename G::N n)
    {
        {n < n} -> std::same_as<bool>;
    }
    class sliced_forward_walker {
    private:
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using INDEX = typename G::INDEX;
        using SLOT_ALLOCATOR=static_vector_container_creator<slot<N, WEIGHT>, slice_max_cnt, error_check>;
        using SLOT_CONTAINER=decltype(std::declval<SLOT_ALLOCATOR>().create_empty(slice_max_cnt));
        SLOT_CONTAINER resident_slots;
        SLOT_CONTAINER prev_slots;
        SLOT_CONTAINER next_slots;
        INDEX n_down;
        slot<N, WEIGHT>* active_slot_ptr;

        static std::optional<std::reference_wrapper<slot<N, WEIGHT>>> find_within_slots(auto& slots, const auto& node) {
            auto it { std::lower_bound(slots.begin(), slots.end(), node, slots_comparator<N, WEIGHT>{}) };
            if (it != slots.end() && (*it).node == node) {
                return { *it };
            }
            return { std::nullopt };
        }

        slot<N, WEIGHT>& find_slot(const N& node) {
            using FOUND_SLOT_REF = std::optional<std::reference_wrapper<slot<N, WEIGHT>>>;
            FOUND_SLOT_REF found_resident_slot_ref { find_within_slots(resident_slots, node) };
            FOUND_SLOT_REF found_prev_slot_ref { find_within_slots(prev_slots, node) };
            FOUND_SLOT_REF found_next_slot_ref { find_within_slots(next_slots, node) };
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
        sliced_forward_walker(
            G& g,
            SLOT_ALLOCATOR container_creator = {}
        )
        : resident_slots{container_creator.create_empty(g.max_resident_nodes_count())}
        , next_slots{container_creator.create_empty(g.max_slice_nodes_count())}
        , prev_slots{container_creator.create_empty(g.max_slice_nodes_count())}
        , n_down{0u}
        , active_slot_ptr{&find_slot(g.get_root_node())} {
            const auto& _resident_slots { g.resident_nodes() };
            std::ranges::copy(_resident_slots.begin(), _resident_slots.end(), std::back_inserter(resident_slots));
            std::ranges::sort(resident_slots.begin(), resident_slots.end(), slots_comparator<N, WEIGHT>{});
            const auto & _next_slots { g.slice_nodes(0u) };
            std::ranges::copy(_next_slots.begin(), _next_slots.end(), std::back_inserter(next_slots));
            std::ranges::sort(next_slots.begin(), next_slots.end(), slots_comparator<N, WEIGHT>{});
            // std::ranges::sort(prev_slots.begin(), prev_slots.end(), slots_comparator<N, WEIGHT>{});
        }

        bool next(
            G& g,
            std::function<WEIGHT(const E&)> get_edge_weight_func
        ) {
            if (n_down == g.down_node_cnt) {
                return true;
            }
            auto incoming_accumulated {
                std::views::common(
                    g.get_inputs(active_slot_ptr->node)
                    | std::views::transform(
                        [&](const auto& edge) noexcept -> std::pair<E, WEIGHT> {
                            const N& n_from { g.get_edge_from(edge) };
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

            // Update resident nodes
            for (const E& edge : g.outputs_to_residents(active_slot_ptr->node)) {
                const N& resident_node { g.get_edge_to(edge) };
                std::optional<std::reference_wrapper<slot<N, WEIGHT>>> resident_slot_maybe {
                    find_within_slots(resident_slots, active_slot_ptr->node)
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
            if (g.last_node_in_slice(n_down) == active_slot_ptr->node) {
                active_slot_ptr = &find_slot(g.next_node_in_slice(active_slot_ptr->node, n_down));
            } else {
                n_down++;
                if (n_down == g.down_node_cnt) {
                    return true;
                }
                prev_slots.clear();
                std::ranges::copy(next_slots.begin(), next_slots.end(), std::back_inserter(prev_slots));
                const auto & _next_slots { g.slice_nodes(n_down) };
                std::ranges::copy(_next_slots.begin(), _next_slots.end(), std::back_inserter(next_slots));
                std::ranges::sort(next_slots.begin(), next_slots.end(), slots_comparator<N, WEIGHT>{});
                active_slot_ptr = &find_slot(g.first_node_in_slice(n_down));
            }
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACK_SLICED_WALKER_H
