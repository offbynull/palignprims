#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKER_H

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
            SLICE_SLOT_CONTAINER_CREATOR slice_slot_container_creator = {},
            RESIDENT_SLOT_CONTAINER_CREATOR resident_slot_container_creator = {},
            PATH_CONTAINER_CREATOR path_container_creator = {}
        )
        : backtracker(
            g,
            g.get_root_node(),
            g.get_leaf_node(),
            slice_slot_container_creator,
            resident_slot_container_creator,
            path_container_creator
        ) {}

        backtracker(
            G& g,
            const N& root_node,
            const N& leaf_node,
            SLICE_SLOT_CONTAINER_CREATOR slice_slot_container_creator = {},
            RESIDENT_SLOT_CONTAINER_CREATOR resident_slot_container_creator = {},
            PATH_CONTAINER_CREATOR path_container_creator = {}
        )
        : whole_graph { g }
        , slice_slot_container_creator { slice_slot_container_creator }
        , resident_slot_container_creator { resident_slot_container_creator }
        , path_container_creator { path_container_creator }
        , sub_graph {
            whole_graph,
            root_node,
            leaf_node
        }
        , mid_down_offset { (sub_graph.grid_down_cnt - 1u) / 2u }
        , prefix_graph {
            sub_graph,
            sub_graph.slice_last_node(mid_down_offset)
        }
        , suffix_graph {
            sub_graph,
            sub_graph.slice_first_node(mid_down_offset)
        }
        , reversed_suffix_graph {
            suffix_graph
        } {}

        auto find_max_path(
            G& graph
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
            const auto& [root_down_offset, root_right_offset, root_depth] { whole_graph.node_to_grid_offsets(sub_graph.get_root_node()) };
            const auto& [leaf_down_offset, leaf_right_offset, leaf_depth] { whole_graph.node_to_grid_offsets(sub_graph.get_leaf_node()) };
            static int indent { 0 };
            std::string indent_str {};
            for (auto i { 0 }; i < indent; i++) {
                indent_str += ' ';
            }
            std::cout
                    << indent_str
                    << "root: [" << root_down_offset << "," << root_right_offset << "]"
                    << " leaf: [" << leaf_down_offset << "," << leaf_right_offset << "]"
                    << std::endl;

            if (sub_graph.grid_down_cnt == 1u && sub_graph.grid_right_cnt == 1u) {
                return 0.0;
            }
            ED max_weight;
            N max_node;
            E max_edge;
            {
                auto x { prefix_graph.slice_first_node(0u)};
                auto y { prefix_graph.slice_last_node(0u)};
                sliced_walker<
                    decltype(prefix_graph),
                    SLICE_SLOT_CONTAINER_CREATOR,
                    RESIDENT_SLOT_CONTAINER_CREATOR,
                    error_check
                > forward_walker {
                    prefix_graph,
                    slice_slot_container_creator,
                    resident_slot_container_creator
                };
                while (!forward_walker.next()) {
                    // do nothing
                }
                sliced_walker<
                    decltype(reversed_suffix_graph),
                    SLICE_SLOT_CONTAINER_CREATOR,
                    RESIDENT_SLOT_CONTAINER_CREATOR,
                    error_check
                > backward_walker {
                    reversed_suffix_graph,
                    slice_slot_container_creator,
                    resident_slot_container_creator
                };
                while (!backward_walker.next()) {
                    // do nothing
                }

                N first_node { sub_graph.slice_first_node(mid_down_offset) };
                const auto& first_forward_slot { forward_walker.find(first_node) };
                const auto& first_backward_slot { backward_walker.find(first_node) };
                std::tuple<WEIGHT, N, E> max {
                    first_forward_slot.backtracking_weight + first_backward_slot.backtracking_weight,
                    first_node,
                    first_backward_slot.backtracking_edge // don't use first_forward_slot's backtracking_edge because it'll be unset for first
                                                          // backward_forward_slot's backtracking edge fomat is the same as first_forward_slot's backtracking_edge -- you can break it using sub_graph
                };
                for (const N& node : sub_graph.slice_nodes(mid_down_offset)) {
                    const auto forward_slot { forward_walker.find(node) };
                    const auto backward_slot { backward_walker.find(node) };
                    std::tuple<WEIGHT, N, E> next {
                        forward_slot.backtracking_weight + backward_slot.backtracking_weight,
                        node,
                        forward_slot.backtracking_edge
                    };
                    if (std::get<0>(next) > std::get<0>(max)) {
                        max = next;
                    }
                }
                const auto& [max_weight_, max_node_, max_edge_] { max };
                max_weight = max_weight_;
                max_node = max_node_;
                max_edge = max_edge_;
            }  // Everything above wrapped in its own scope so that walkers (and their associated containers) are destroyed

            const auto& [cut_down_offset, cut_right_offset, cut_depth] { whole_graph.node_to_grid_offsets(max_node) };
            std::cout
                    << indent_str
                    << "found: [" << cut_down_offset << "," << cut_right_offset << "]"
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
            indent++;
            {
                const N& new_leaf_node { whole_graph.get_edge_from(max_edge) };
                backtracker upper_half_backtracker {
                    whole_graph,
                    sub_graph.get_root_node(),
                    new_leaf_node,
                    slice_slot_container_creator,
                    resident_slot_container_creator
                };
                std::cout << indent_str << " topleft" << std::endl;
                upper_half_backtracker.walk(
                    path_container_,
                    current_element,
                    walk_direction::PREFIX
                );
            } // Everything above wrapped in its own scope so that backtracker (and its associated containers) are destroyed
            {
                const N& new_root_node { whole_graph.get_edge_to(max_edge) };
                backtracker lower_half_backtracker {
                    whole_graph,
                    new_root_node,
                    sub_graph.get_leaf_node(),
                    slice_slot_container_creator,
                    resident_slot_container_creator
                };
                std::cout << indent_str << " bottomright" << std::endl;
                lower_half_backtracker.walk(
                    path_container_,
                    current_element,
                    walk_direction::SUFFIX
                );
            } // Everything above wrapped in its own scope so that backtracker (and its associated containers) are destroyed

            indent--;

            return max_weight;
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKER_H
