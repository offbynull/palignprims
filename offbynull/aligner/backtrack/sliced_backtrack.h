#ifndef OFFBYNULL_ALIGNER_BACKTRACK_SLICED_BACKTRACK_H
#define OFFBYNULL_ALIGNER_BACKTRACK_SLICED_BACKTRACK_H

#include <cstddef>
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

        INDEX mid_down_offset;
        INDEX mid_right_offset;
        prefix_sliceable_pairwise_alignment_graph<
            G,
            error_check
        > forward_graph;
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
        > backward_graph;
        sliced_walker<
            suffix_sliceable_pairwise_alignment_graph<
                G,
                error_check
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
        : mid_down_offset { g.grid_down_cnt / 2 }
        , mid_right_offset { g.grid_right_cnt / 2 }
        , forward_graph {
            g,
            mid_down_offset,
            mid_right_offset
        }
        , forward_walker {
            forward_graph,
            get_edge_weight_func,
            slice_slot_container_creator,
            resident_slot_container_creator
        }
        , backward_graph {
            g,
            g.grid_down_cnt - mid_down_offset,
            g.grid_right_cnt - mid_right_offset
        }
        , backward_walker {
                backward_graph,
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
                        return std::pair<WEIGHT, N> {
                            f_slot.backtracking_weight + b_slot.backtracking_weight,
                            f_slot.node
                        };
                    }
                )
            };
            auto max_it { std::ranges::max(combined.begin(), combined.end()) };
            if constexpr (error_check) {
                if (max_it == combined.end()) {
                    throw std::runtime_error("No maximum?");
                }
            }
            const auto& [max_weight, max_node] { *max_it };
            return max_node;
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACK_SLICED_BACKTRACK_H
