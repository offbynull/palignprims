#ifndef OFFBYNULL_ALIGNER_BACKTRACK_SLICED_BACKTRACK_H
#define OFFBYNULL_ALIGNER_BACKTRACK_SLICED_BACKTRACK_H

#include <functional>
#include <ranges>
#include <algorithm>
#include <iostream>

#include "offbynull/aligner/backtrack/sliced_walker.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/backtrack/container_creator.h"
#include "offbynull/aligner/backtrack/container_creators.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/prefix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/suffix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/middle_sliceable_pairwise_alignment_graph.h"
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
    using offbynull::aligner::graphs::middle_sliceable_pairwise_alignment_graph::middle_sliceable_pairwise_alignment_graph;
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

        G& whole_graph;
        std::function<WEIGHT(const E&)> edge_weight_getter;
        SLICE_SLOT_ALLOCATOR slice_slot_container_creator;
        RESIDENT_SLOT_ALLOCATOR resident_slot_container_creator;
        middle_sliceable_pairwise_alignment_graph<
            G,
            error_check
        > sub_graph;
        INDEX mid_down_offset;
        prefix_sliceable_pairwise_alignment_graph<
            decltype(sub_graph),
            error_check
        > prefix_graph;
        sliced_walker<
            decltype(prefix_graph),
            WEIGHT,
            SLICE_SLOT_ALLOCATOR,
            RESIDENT_SLOT_ALLOCATOR,
            error_check
        > forward_walker;
        suffix_sliceable_pairwise_alignment_graph<
            decltype(sub_graph),
            error_check
        > suffix_graph;
        reversed_sliceable_pairwise_alignment_graph<
            decltype(suffix_graph)
        > reversed_suffix_graph;
        sliced_walker<
            decltype(reversed_suffix_graph),
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
        : sliced_backtracker(
            g,
            0u,
            0u,
            g.grid_down_cnt,
            g.grid_right_cnt,
            get_edge_weight_func,
            slice_slot_container_creator,
            resident_slot_container_creator
        ) {}

        sliced_backtracker(
            G& g,
            INDEX grid_down_offset,
            INDEX grid_right_offset,
            INDEX grid_down_cnt,
            INDEX grid_right_cnt,
            std::function<WEIGHT(const E&)> get_edge_weight_func,
            SLICE_SLOT_ALLOCATOR slice_slot_container_creator = {},
            RESIDENT_SLOT_ALLOCATOR resident_slot_container_creator = {}
        )
        : whole_graph { g }
        , edge_weight_getter { get_edge_weight_func }
        , slice_slot_container_creator { slice_slot_container_creator }
        , resident_slot_container_creator { resident_slot_container_creator }
        , sub_graph {
            whole_graph,
            grid_down_offset,
            grid_right_offset,
            grid_down_cnt,
            grid_right_cnt
        }
        , mid_down_offset { (sub_graph.grid_down_cnt - 1u) / 2u }
        , prefix_graph {
            sub_graph,
            mid_down_offset + 1,
            sub_graph.grid_right_cnt
        }
        , forward_walker {
            prefix_graph,
            get_edge_weight_func,
            slice_slot_container_creator,
            resident_slot_container_creator
        }
        , suffix_graph {
            sub_graph,
            sub_graph.grid_down_cnt - mid_down_offset,
            sub_graph.grid_right_cnt
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

        INDEX target_weight() {
            sliced_walker<
                decltype(sub_graph),
                WEIGHT,
                SLICE_SLOT_ALLOCATOR,
                RESIDENT_SLOT_ALLOCATOR,
                error_check
            > forward_walker {
                sub_graph,
                edge_weight_getter,
                slice_slot_container_creator,
                resident_slot_container_creator
            };
            while (!forward_walker.next()) {
                // do nothing
            }
            const auto& slot { forward_walker.active_slot() };
            return slot.backtracking_weight;
            // TODO: When you're doing the forward_walked / backward_walker in the function below, CHECK TO MAKE SURE
            // THE WEIGHT HITS THIS NUMBER (not just max). IF NO NUMBER MATCHES THIS NUMBER, IT MEANS THAT SLICE IS
            // SKIPPED OVER (e.g. free ride skips it)
            //
            // DO THIS FOR EVERY INVOCATION OF walk()
        }

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
            const auto& [cut_down_offset, cut_right_offset] { sub_graph.node_to_grid_offsets(max_node) };
            std::cout
                    << "down bounds: ["
                    << sub_graph.grid_down_offset << "," << sub_graph.grid_down_offset + sub_graph.grid_down_cnt - 1u << "]"
                    << std::endl
                    << "right bounds: ["
                    << sub_graph.grid_right_offset << "," << sub_graph.grid_right_offset + sub_graph.grid_right_cnt - 1u << "]"
                    << std::endl;
            std::cout
                    << "found " << sub_graph.grid_down_offset + cut_down_offset << "x" << sub_graph.grid_right_offset + cut_right_offset
                    << std::endl;
            if (cut_down_offset == 0u && cut_right_offset == 0u) {
                return max_node;
            }
            {
                sliced_backtracker<
                    G,
                    WEIGHT,
                    SLICE_SLOT_ALLOCATOR,
                    RESIDENT_SLOT_ALLOCATOR,
                    error_check
                > upper_half_backtracker {
                    whole_graph,
                    sub_graph.grid_down_offset,
                    sub_graph.grid_right_offset,
                    cut_down_offset + 1u,
                    cut_right_offset + 1u,
                    edge_weight_getter,
                    slice_slot_container_creator,
                    resident_slot_container_creator
                };
                std::cout << "topleft" << std::endl;
                upper_half_backtracker.walk();
            }
            {
                sliced_backtracker<
                    G,
                    WEIGHT,
                    SLICE_SLOT_ALLOCATOR,
                    RESIDENT_SLOT_ALLOCATOR,
                    error_check
                > lower_half_backtracker {
                    whole_graph,
                    sub_graph.grid_down_offset + cut_down_offset,
                    sub_graph.grid_right_offset + cut_right_offset,
                    sub_graph.grid_down_cnt - cut_down_offset,
                    sub_graph.grid_right_cnt - cut_right_offset,
                    edge_weight_getter,
                    slice_slot_container_creator,
                    resident_slot_container_creator
                };
                std::cout << "bottomright" << std::endl;
                lower_half_backtracker.walk();
            }

            return max_node;
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACK_SLICED_BACKTRACK_H
