#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKER_H

#include <functional>
#include <ranges>
#include <algorithm>
#include <iostream>
#include "path_container.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/concepts.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/container_creator_packs.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/sliced_walker.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/helpers/container_creators.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/prefix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/suffix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/middle_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/reversed_sliceable_pairwise_alignment_graph.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtracker {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::container_creator_packs::container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::container_creator_packs::heap_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::concepts::backtrackable_node;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::concepts::backtrackable_edge;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::sliced_walker::sliced_walker;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::sliced_walker::slot;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::path_container;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::element;
    using offbynull::helpers::container_creators::container_creator;
    using offbynull::helpers::container_creators::container_creator_of_type;
    using offbynull::helpers::container_creators::vector_container_creator;
    using offbynull::helpers::container_creators::static_vector_container_creator;
    using offbynull::aligner::graphs::prefix_sliceable_pairwise_alignment_graph::prefix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::suffix_sliceable_pairwise_alignment_graph::suffix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::middle_sliceable_pairwise_alignment_graph::middle_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::reversed_sliceable_pairwise_alignment_graph::reversed_sliceable_pairwise_alignment_graph;
    using offbynull::concepts::range_of_type;
    using offbynull::concepts::widenable_to_size_t;

    template<
        readable_sliceable_pairwise_alignment_graph G,
        weight WEIGHT,
        container_creator_pack<G, WEIGHT> CONTAINER_CREATOR_PACK = heap_container_creator_pack<G, WEIGHT, true>,
        bool error_check = true
    >
    requires backtrackable_node<typename G::N> &&
        backtrackable_edge<typename G::E>
    class backtracker {
    private:
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using INDEX = typename G::INDEX;

        using SLICE_SLOT_CONTAINER_CREATOR = typename CONTAINER_CREATOR_PACK::SLICE_SLOT_CONTAINER_CREATOR;
        using RESIDENT_SLOT_CONTAINER_CREATOR = typename CONTAINER_CREATOR_PACK::RESIDENT_SLOT_CONTAINER_CREATOR;
        using ELEMENT_CONTAINER_CREATOR = typename CONTAINER_CREATOR_PACK::ELEMENT_CONTAINER_CREATOR;
        using PATH_CONTAINER_CREATOR = typename CONTAINER_CREATOR_PACK::PATH_CONTAINER_CREATOR;

        G& whole_graph;
        std::function<WEIGHT(const E&)> edge_weight_getter;
        SLICE_SLOT_CONTAINER_CREATOR slice_slot_container_creator;
        RESIDENT_SLOT_CONTAINER_CREATOR resident_slot_container_creator;
        PATH_CONTAINER_CREATOR path_container_creator;
        middle_sliceable_pairwise_alignment_graph<
            G,
            error_check
        > sub_graph;
        INDEX mid_down_offset;
        prefix_sliceable_pairwise_alignment_graph<
            decltype(sub_graph),
            error_check
        > prefix_graph;
        suffix_sliceable_pairwise_alignment_graph<
            decltype(sub_graph),
            error_check
        > suffix_graph;
        reversed_sliceable_pairwise_alignment_graph<
            decltype(suffix_graph)
        > reversed_suffix_graph;

    public:
        backtracker(
            G& g,
            std::function<WEIGHT(const E&)> get_edge_weight_func,
            SLICE_SLOT_CONTAINER_CREATOR slice_slot_container_creator = {},
            RESIDENT_SLOT_CONTAINER_CREATOR resident_slot_container_creator = {},
            PATH_CONTAINER_CREATOR path_container_creator = {}
        )
        : backtracker(
            g,
            0u,
            0u,
            g.grid_down_cnt,
            g.grid_right_cnt,
            get_edge_weight_func,
            slice_slot_container_creator,
            resident_slot_container_creator,
            path_container_creator
        ) {}

        backtracker(
            G& g,
            INDEX grid_down_offset,
            INDEX grid_right_offset,
            INDEX grid_down_cnt,
            INDEX grid_right_cnt,
            std::function<WEIGHT(const E&)> get_edge_weight_func,
            SLICE_SLOT_CONTAINER_CREATOR slice_slot_container_creator = {},
            RESIDENT_SLOT_CONTAINER_CREATOR resident_slot_container_creator = {},
            PATH_CONTAINER_CREATOR path_container_creator = {}
        )
        : whole_graph { g }
        , edge_weight_getter { get_edge_weight_func }
        , slice_slot_container_creator { slice_slot_container_creator }
        , resident_slot_container_creator { resident_slot_container_creator }
        , path_container_creator { path_container_creator }
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
        , suffix_graph {
            sub_graph,
            sub_graph.grid_down_cnt - mid_down_offset,
            sub_graph.grid_right_cnt
        }
        , reversed_suffix_graph {
            suffix_graph
        } {}

        // INDEX target_weight() {
        //     sliced_walker<
        //         decltype(sub_graph),
        //         WEIGHT,
        //         SLICE_SLOT_CONTAINER_CREATOR,
        //         RESIDENT_SLOT_CONTAINER_CREATOR,
        //         error_check
        //     > forward_walker {
        //         sub_graph,
        //         edge_weight_getter,
        //         slice_slot_container_creator,
        //         resident_slot_container_creator
        //     };
        //     while (!forward_walker.next()) {
        //         // do nothing
        //     }
        //     const auto& slot { forward_walker.active_slot() };
        //     return slot.backtracking_weight;
        //     // TODO: When you're doing the forward_walked / backward_walker in the function below, CHECK TO MAKE SURE
        //     // THE WEIGHT HITS THIS NUMBER (not just max). IF NO NUMBER MATCHES THIS NUMBER, IT MEANS THAT SLICE IS
        //     // SKIPPED OVER (e.g. free ride skips it)
        //     //
        //     // DO THIS FOR EVERY INVOCATION OF walk()
        // }

        auto find_max_path(
                G& graph,
                std::function<WEIGHT(const E&)> get_edge_weight_func
        ) {
            path_container<G, ELEMENT_CONTAINER_CREATOR, error_check> path_container_ {
                G::limits(
                    graph.grid_down_cnt,
                    graph.grid_right_cnt
                ).max_path_edge_cnt
            };
            WEIGHT weight { walk(path_container_, nullptr, walk_direction::INITIALIZE) };

            auto path { path_container_creator.create_empty(std::nullopt) };
            for (E backtracking_edge : path_container_.walk_path_backward(graph)) {
                path.push_back(backtracking_edge);
            }
            // At this point, path is in reverse order (from last to first). Reverse it to get it into the forward order
            // (from first to last).
            std::reverse(path.begin(), path.end());
            return std::make_pair(path, weight);
        }

    private:
        enum class walk_direction {
            PREFIX,
            SUFFIX,
            INITIALIZE
        };

        WEIGHT walk(
            path_container<G, ELEMENT_CONTAINER_CREATOR>& path_container_,
            element<E>* parent_element,
            walk_direction dir
        ) {
            ED max_weight;
            N max_node;
            E max_edge;
            {
                sliced_walker<
                    decltype(prefix_graph),
                    WEIGHT,
                    SLICE_SLOT_CONTAINER_CREATOR,
                    RESIDENT_SLOT_CONTAINER_CREATOR,
                    error_check
                > forward_walker {
                    prefix_graph,
                    edge_weight_getter,
                    slice_slot_container_creator,
                    resident_slot_container_creator
                };
                while (!forward_walker.next()) {
                    // do nothing
                }
                sliced_walker<
                    decltype(reversed_suffix_graph),
                    WEIGHT,
                    SLICE_SLOT_CONTAINER_CREATOR,
                    RESIDENT_SLOT_CONTAINER_CREATOR,
                    error_check
                > backward_walker {
                    reversed_suffix_graph,
                    edge_weight_getter,
                    slice_slot_container_creator,
                    resident_slot_container_creator
                };
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
                            return std::tuple<WEIGHT, N, E> {
                                f_slot.backtracking_weight + b_slot.backtracking_weight,
                                f_slot.node,
                                f_slot.backtracking_edge
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
                const auto& [max_weight_, max_node_, max_edge_] { *max_it };
                max_weight = max_weight_;
                max_node = max_node_;
                max_edge = max_edge_;
            }  // Everything above wrapped in its own scope so that walkers (and their associated containers) are destroyed


            const auto& [cut_down_offset, cut_right_offset, _] { sub_graph.node_to_grid_offsets(max_node) };
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
                return max_weight;
            }

            // Add
            element<E>* current_element { nullptr };
            switch (dir) {
                case walk_direction::PREFIX:
                    current_element = path_container_.push_prefix(parent_element, max_edge);
                    break;
                case walk_direction::SUFFIX:
                    current_element = path_container_.push_suffix(parent_element, max_edge);
                    break;
                [[unlikely]] case walk_direction::INITIALIZE:
                    current_element = path_container_.initialize(max_edge);
                    break;
                [[unlikely]] default:
                    if constexpr (error_check) {
                        throw std::runtime_error("Unexpected");
                    }
                    break;
            }

            // Recurse
            {
                backtracker<
                    G,
                    WEIGHT,
                    CONTAINER_CREATOR_PACK,
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
                upper_half_backtracker.walk(
                    path_container_,
                    current_element,
                    walk_direction::PREFIX
                );
            } // Everything above wrapped in its own scope so that backtracker (and its associated containers) are destroyed
            {
                backtracker<
                    G,
                    WEIGHT,
                    CONTAINER_CREATOR_PACK,
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
                lower_half_backtracker.walk(
                    path_container_,
                    current_element,
                    walk_direction::SUFFIX
                );
            } // Everything above wrapped in its own scope so that backtracker (and its associated containers) are destroyed

            return max_weight;
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKER_H
