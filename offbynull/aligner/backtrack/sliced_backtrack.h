#ifndef OFFBYNULL_ALIGNER_BACKTRACK_SLICED_BACKTRACK_H
#define OFFBYNULL_ALIGNER_BACKTRACK_SLICED_BACKTRACK_H

#include <functional>
#include <ranges>
#include <algorithm>
#include "offbynull/aligner/backtrack/sliced_walker.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/backtrack/container_creator.h"
#include "offbynull/aligner/backtrack/container_creators.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/prefix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/suffix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/reversed_sliceable_pairwise_alignment_graph.h"
#include "offbynull/concepts.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::backtrack::sliced_backtrack {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_parwise_alignment_graph;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::backtrack::sliced_walker::sliced_walker;
    using offbynull::aligner::backtrack::sliced_walker::slot;
    using offbynull::aligner::backtrack::container_creator::container_creator;
    using offbynull::aligner::backtrack::container_creators::vector_container_creator;
    using offbynull::aligner::graphs::prefix_sliceable_pairwise_alignment_graph::prefix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::suffix_sliceable_pairwise_alignment_graph::suffix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::reversed_sliceable_pairwise_alignment_graph::reversed_sliceable_pairwise_alignment_graph;
    using offbynull::concepts::range_of_type;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::utils::max_element;

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
    class sliced_backtracker {
    private:
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using INDEX = typename G::INDEX;

        G& original_graph;
        std::function<WEIGHT(const E&)> edge_weight_getter;
        SLICE_SLOT_ALLOCATOR slice_slot_container_creator;
        RESIDENT_SLOT_ALLOCATOR resident_slot_container_creator;
        INDEX mid_down_offset;
        prefix_sliceable_pairwise_alignment_graph<
            G,
            error_check
        > prefix_graph;
        sliced_walker<
            prefix_sliceable_pairwise_alignment_graph<
                G,
                error_check
            >,
            WEIGHT,
            SLICE_SLOT_ALLOCATOR,
            RESIDENT_SLOT_ALLOCATOR,
            error_check
        > forward_walker;
        suffix_sliceable_pairwise_alignment_graph<
            G,
            error_check
        > suffix_graph;
        reversed_sliceable_pairwise_alignment_graph<
            suffix_sliceable_pairwise_alignment_graph<
                G,
                error_check
            >
        > reversed_suffix_graph;
        sliced_walker<
            reversed_sliceable_pairwise_alignment_graph<
                suffix_sliceable_pairwise_alignment_graph<
                    G,
                    error_check
                >
            >,
            WEIGHT,
            SLICE_SLOT_ALLOCATOR,
            RESIDENT_SLOT_ALLOCATOR,
            error_check
        > backward_walker;

    public:
        sliced_backtracker(
            G& g,
            std::function<WEIGHT(const E&)> get_edge_weight_func,
            SLICE_SLOT_ALLOCATOR slice_slot_container_creator = {},
            RESIDENT_SLOT_ALLOCATOR resident_slot_container_creator = {}
        )
        : original_graph { g }
        , edge_weight_getter { get_edge_weight_func }
        , slice_slot_container_creator { slice_slot_container_creator }
        , resident_slot_container_creator { resident_slot_container_creator }
        , mid_down_offset { g.grid_down_cnt / 2 }
        , prefix_graph {
            g,
            mid_down_offset + 1,
            g.grid_right_cnt
        }
        , forward_walker {
            prefix_graph,
            get_edge_weight_func,
            slice_slot_container_creator,
            resident_slot_container_creator
        }
        , suffix_graph {
            g,
            g.grid_down_cnt - mid_down_offset,
            g.grid_right_cnt
        }
        , reversed_suffix_graph {
            suffix_graph
        }
        , backward_walker {
                reversed_suffix_graph,
                get_edge_weight_func,
                slice_slot_container_creator,
                resident_slot_container_creator
        } {}

        auto walk() {
            while (!forward_walker.next()) {
                // do nothing
            }
            while (!backward_walker.next()) {
                // do nothing
            }

            auto combined {
                std::views::zip(
                    forward_walker.active_slots(),
                    backward_walker.active_slots()
                )
                | std::views::transform(
                    [](const auto &slot_pair) {
                        const auto& [f_slot, b_slot] { slot_pair };
                        if constexpr (error_check) {
                            if (f_slot.node != b_slot.node) {
                                throw std::runtime_error { "Node mismatch" };
                            }
                        }
                        return std::pair<WEIGHT, N> {
                            f_slot.backtracking_weight + b_slot.backtracking_weight,
                            f_slot.node
                        };
                    }
                )
            };
            auto max_it {
                std::ranges::max_element(
                    combined.begin(),
                    combined.end(),
                    [](const auto& a, const auto& b) { return std::get<0>(a) < std::get<0>(b); }
                )
            };
            if constexpr (error_check) {
                if (max_it == combined.end()) {
                    throw std::runtime_error("No maximum?");
                }
            }
            const auto& [max_weight, max_node] { *max_it };

            // Recurse
            // THIS IS CAUSING INFINITE TEMPLATE RECURSION
            const auto& [mid_down_offset, mid_right_offset] { original_graph.node_to_grid_offsets(max_node) };
            if (mid_down_offset == 1u && mid_right_offset == 1u) {
                return max_node;
            }
            INDEX upper_half_graph_down_cnt { mid_down_offset + 1u };
            INDEX upper_half_graph_right_cnt { mid_right_offset + 1u };
            prefix_sliceable_pairwise_alignment_graph<
                G,
                error_check
            > upper_half_graph {
                original_graph,
                upper_half_graph_down_cnt,
                upper_half_graph_right_cnt
            };
            sliced_backtracker<
                decltype(upper_half_graph),
                WEIGHT,
                SLICE_SLOT_ALLOCATOR,
                RESIDENT_SLOT_ALLOCATOR,
                error_check
            > upper_half_backtracker {
                upper_half_graph,
                edge_weight_getter,
                slice_slot_container_creator,
                resident_slot_container_creator
            };
            upper_half_backtracker.walk();
            INDEX lower_half_graph_down_cnt { original_graph.grid_down_cnt - mid_down_offset - 1u };
            INDEX lower_half_graph_right_cnt { original_graph.grid_right_cnt - mid_right_offset - 1u };
            suffix_sliceable_pairwise_alignment_graph<
                G,
                error_check
            > lower_half_graph {
                original_graph,
                lower_half_graph_down_cnt,
                lower_half_graph_right_cnt
            };
            sliced_backtracker<
                decltype(lower_half_graph),
                WEIGHT,
                SLICE_SLOT_ALLOCATOR,
                RESIDENT_SLOT_ALLOCATOR,
                error_check
            > lower_half_backtracker {
                lower_half_graph,
                edge_weight_getter,
                slice_slot_container_creator,
                resident_slot_container_creator
            };
            lower_half_backtracker.walk();

            return max_node;
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACK_SLICED_BACKTRACK_H
