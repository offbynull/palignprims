#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_FORWARD_WALKER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_FORWARD_WALKER_H

#include <cstddef>
#include <functional>
#include <ranges>
#include <optional>
#include <utility>
#include <stdexcept>
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/slot.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/slice_slot_container.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_slot_container.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::slot::slot;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::slice_slot_container::slice_slot_container;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::slice_slot_container
        ::slice_slot_container_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::slice_slot_container
        ::slice_slot_container_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::slice_slot_container
        ::slice_slot_container_stack_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container
        ::resident_slot_with_node;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container::resident_slot;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container
        ::resident_slot_container;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container
        ::resident_slot_container_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container
        ::resident_slot_container_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container
        ::resident_slot_container_stack_container_creator_pack;
    using offbynull::aligner::concepts::weight;
    using offbynull::concepts::range_of_type;
    using offbynull::concepts::widenable_to_size_t;




    template<
        typename T,
        typename N,
        typename E,
        typename ED
    >
    concept forward_walker_container_creator_pack =
        weight<ED>
        && requires(const T t) {
            { t.create_slice_slot_container_container_creator_pack() } -> slice_slot_container_container_creator_pack<E, ED>;
            { t.create_resident_slot_container_container_creator_pack() } -> resident_slot_container_container_creator_pack<N, E, ED>;
        };

    template<
        bool debug_mode,
        typename N,
        typename E,
        weight ED
    >
    struct forward_walker_heap_container_creator_pack {
        slice_slot_container_heap_container_creator_pack<
            debug_mode,
            E,
            ED
        > create_slice_slot_container_container_creator_pack() const {
            return {};
        }

        resident_slot_container_heap_container_creator_pack<
            debug_mode,
            N,
            E,
            ED
        > create_resident_slot_container_container_creator_pack() const {
            return {};
        }
    };

    template<
        bool debug_mode,
        typename N,
        typename E,
        weight ED,
        std::size_t grid_right_cnt,
        std::size_t grid_depth_cnt,
        std::size_t resident_nodes_capacity
    >
    struct forward_walker_stack_container_creator_pack {
        slice_slot_container_stack_container_creator_pack<
            debug_mode,
            E,
            ED,
            grid_right_cnt,
            grid_depth_cnt
        > create_slice_slot_container_container_creator_pack() const {
            return {};
        }

        resident_slot_container_stack_container_creator_pack<
            debug_mode,
            N,
            E,
            ED,
            resident_nodes_capacity
        > create_resident_slot_container_container_creator_pack() const {
            return {};
        }
    };




    template<typename N, typename E, weight WEIGHT>
    struct slice_entry {
        N node;
        slot<E, WEIGHT>* slot_ptr;

        slice_entry()
        : node {}
        , slot_ptr { nullptr } {}
    };

    template<
        bool debug_mode,
        readable_sliceable_pairwise_alignment_graph G,
        slice_slot_container_container_creator_pack<
            typename G::E,
            typename G::ED
        > SLICE_SLOT_CONTAINER_CONTAINER_CREATOR_PACK
    >
    struct slice_slot_container_pair {
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using INDEX = typename G::INDEX;

        slice_slot_container<debug_mode, G, SLICE_SLOT_CONTAINER_CONTAINER_CREATOR_PACK> slots1;
        slice_slot_container<debug_mode, G, SLICE_SLOT_CONTAINER_CONTAINER_CREATOR_PACK> slots2;
        slice_slot_container<debug_mode, G, SLICE_SLOT_CONTAINER_CONTAINER_CREATOR_PACK>* previous_slots;  // row above current row
        slice_slot_container<debug_mode, G, SLICE_SLOT_CONTAINER_CONTAINER_CREATOR_PACK>* current_slots;  // current row
        INDEX grid_down_offset;

        slice_slot_container_pair(
            const G& g,
            SLICE_SLOT_CONTAINER_CONTAINER_CREATOR_PACK container_creator_pack = {}
        )
        : slots1 { g, container_creator_pack }
        , slots2 { g, container_creator_pack }
        , previous_slots { &slots1 }
        , current_slots { &slots2 }
        , grid_down_offset { 0u } {}

        // Custom copy/move/copy assignment/move assigned because this class has raw pointer types as members. The default copy/assignment
        // will do a SHALLOW copy of these pointers, meaning they won't be pointing into the copy'd element_container (they'll instead be
        // pointing into the original element_container).
        slice_slot_container_pair(const slice_slot_container_pair& other)
        : slots1 { other.slots1 }
        , slots2 { other.slots2 }
        , previous_slots { other.previous_slots == &other.slots1 ? &slots1 : &slots2 }
        , current_slots { other.current_slots == &other.slots1 ? &slots1 : &slots2 }
        , grid_down_offset { other.grid_down_offset } {}

        slice_slot_container_pair(slice_slot_container_pair&& other)
        : slots1 { std::move(other.slots1) }
        , slots2 { std::move(other.slots2) }
        , previous_slots { other.previous_slots == &other.slots1 ? &slots1 : &slots2 }
        , current_slots { other.current_slots == &other.slots1 ? &slots1 : &slots2 }
        , grid_down_offset { std::move(other.grid_down_offset) } {}

        slice_slot_container_pair& operator=(const slice_slot_container_pair& other) {
            if (this != &other) { // guard against self-assignment
                slots1 = other.slots1;
                slots2 = other.slots2;
                previous_slots = other.previous_slots == &other.slots1 ? &slots1 : &slots2;
                current_slots = other.current_slots == &other.slots1 ? &slots1 : &slots2;
                grid_down_offset = other.grid_down_offset;
            }
            return *this;
        }

        slice_slot_container_pair& operator=(slice_slot_container_pair&& other) {
            if (this != &other) { // guard against self-assignment
                slots1 = std::move(other.slots1);
                slots2 = std::move(other.slots2);
                previous_slots = other.previous_slots == &other.slots1 ? &slots1 : &slots2;
                current_slots = other.current_slots == &other.slots1 ? &slots1 : &slots2;
                grid_down_offset = std::move(other.grid_down_offset);
            }
        }

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
        bool debug_mode,
        readable_sliceable_pairwise_alignment_graph G,
        forward_walker_container_creator_pack<
            typename G::N,
            typename G::E,
            typename G::ED
        > CONTAINER_CREATOR_PACK = forward_walker_heap_container_creator_pack<
            debug_mode,
            typename G::N,
            typename G::E,
            typename G::ED
        >
    >
    class forward_walker {
    private:
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using INDEX = typename G::INDEX;

        using SLICE_SLOT_CONTAINER_CONTAINER_CREATOR_PACK =
            decltype(std::declval<CONTAINER_CREATOR_PACK>().create_slice_slot_container_container_creator_pack());
        using RESIDENT_SLOT_CONTAINER_CONTAINER_CREATOR_PACK =
            decltype(std::declval<CONTAINER_CREATOR_PACK>().create_resident_slot_container_container_creator_pack());

        const G& g;
        resident_slot_container<debug_mode, G, RESIDENT_SLOT_CONTAINER_CONTAINER_CREATOR_PACK> resident_slots;
        slice_slot_container_pair<debug_mode, G, SLICE_SLOT_CONTAINER_CONTAINER_CREATOR_PACK> slice_slots;
        decltype(g.slice_nodes(0u)) slice;
        decltype(slice.begin()) slice_it;
        slice_entry<N, E, ED> slice_entry_;

    public:
        static forward_walker create_and_initialize(
            const G& g_,
            const INDEX target_slice,
            CONTAINER_CREATOR_PACK container_creator_pack_ = {}
        ) {
            if constexpr (debug_mode) {
                if (target_slice >= g_.grid_down_cnt) {
                    throw std::runtime_error("Slice too far down");
                }
            }
            forward_walker ret {
                g_,
                container_creator_pack_
            };
            while (ret.slice_slots.grid_down_offset != target_slice || ret.slice_it != ret.slice.end()) {
                ret.step_forward();
            }
            return ret;
        }

        slot<E, ED>& find(const N& node) {
            auto found_resident { resident_slots.find(node) };
            if (found_resident.has_value()) {
                return (*found_resident).get().slot_;
            }
            auto found_slice { slice_slots.find(node) };
            if (found_slice.has_value()) {
                return *found_slice;
            }
            if constexpr (debug_mode) {
                throw std::runtime_error("Node not found");
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
        , slice_slots { g, container_creator_pack_.create_slice_slot_container_container_creator_pack() }
        , slice { g.slice_nodes(0u) }
        , slice_it { slice.begin() }
        , slice_entry_ {} {
            slice_entry_.node = *slice_it;
            slice_entry_.slot_ptr = &find(slice_entry_.node); // should be equivalent to g.get_root_node()
        }

        void step_forward() {
            if (slice_it != slice.end()) {
                slice_entry_.node = *slice_it;
                slice_entry_.slot_ptr = &find(slice_entry_.node);
            } else {
                if (slice_slots.grid_down_offset == g.grid_down_cnt - 1u) {
                    return;
                }
                slice_slots.move_down();
                slice = g.slice_nodes(slice_slots.grid_down_offset);
                slice_it = slice.begin();
                slice_entry_.node = *slice_it;
                slice_entry_.slot_ptr = &find(slice_entry_.node);
            }
            // Compute only if node is not a resident. A resident node's backtracking weight + backtracking edge should
            // be computed as its inputs are walked over one-by-one by this function (see block below this one).
            if (!resident_slots.find(slice_entry_.node).has_value()) {
                auto incoming_accumulated {
                    std::views::common(
                        g.get_inputs(slice_entry_.node)
                        | std::views::transform(
                            [&](const auto& edge) noexcept -> std::pair<E, ED> {
                                const N& n_from { g.get_edge_from(edge) };
                                const ED& edge_weight { g.get_edge_data(edge) };
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
                    slice_entry_.slot_ptr->backtracking_edge = { (*found).first };
                    slice_entry_.slot_ptr->backtracking_weight = (*found).second;
                }
            }

            // Update resident node weights
            for (const E& edge : g.outputs_to_residents(slice_entry_.node)) {
                const N& resident_node { g.get_edge_to(edge) };
                std::optional<std::reference_wrapper<resident_slot<E, ED>>> resident_slot_maybe {
                    resident_slots.find(resident_node)
                };
                if constexpr (debug_mode) {
                    if (!resident_slot_maybe.has_value()) {
                        throw std::runtime_error("This should never happen");
                    }
                }
                resident_slot<E, ED>& resident_slot_ { (*resident_slot_maybe).get() };
                const ED& edge_weight { g.get_edge_data(edge) };
                if (!resident_slot_.initialized) {
                    resident_slot_.slot_.backtracking_edge = { edge };
                    resident_slot_.slot_.backtracking_weight = edge_weight;
                    resident_slot_.initialized = true;
                } else {
                    const ED& new_weight { slice_entry_.slot_ptr->backtracking_weight + edge_weight };
                    if (new_weight > resident_slot_.slot_.backtracking_weight) {
                        resident_slot_.slot_.backtracking_edge = { edge };
                        resident_slot_.slot_.backtracking_weight = new_weight;
                    }
                }
            }

            // Move to next node
            ++slice_it;
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_FORWARD_WALKER_H
