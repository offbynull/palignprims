#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLICED_SUBDIVIDER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLICED_SUBDIVIDER_H

#include <ranges>
#include <algorithm>
#include <iostream>
#include "path_container.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/concepts.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/container_creator_packs.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/forward_walker.h"
#include "offbynull/helpers/container_creators.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/prefix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/suffix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/middle_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/reversed_sliceable_pairwise_alignment_graph.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::sliced_subdivider {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::concepts::backtrackable_node;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::concepts::backtrackable_edge;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker::forward_walker;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker::slot;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::path_container;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::element;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container::node_searchable_slot;
    using offbynull::helpers::container_creators::container_creator;
    using offbynull::helpers::container_creators::container_creator_of_type;
    using offbynull::helpers::container_creators::array_container_creator;
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
        container_creator SLICE_SLOT_CONTAINER_CREATOR=vector_container_creator<
            slot<
                typename G::E,
                typename G::ED
            >,
            true
        >,
        container_creator ELEMENT_CONTAINER_CREATOR=vector_container_creator<
            element<typename G::E>,
            true
        >,
        bool error_check = true
    >
    requires backtrackable_node<typename G::N> &&
        backtrackable_edge<typename G::E>
    class sliced_subdivider {
    private:
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using INDEX = typename G::INDEX;

        // whole_graph expected to have 0 resident nodes, so enforce this as a 0-length array
        using RESIDENT_SLOT_CONTAINER_CREATOR = array_container_creator<
            node_searchable_slot<
                typename G::N,
                typename G::E,
                typename G::ED
            >,
            0zu
        >;

        G& whole_graph;
        SLICE_SLOT_CONTAINER_CREATOR slice_slot_container_creator;
        RESIDENT_SLOT_CONTAINER_CREATOR resident_slot_container_creator;
        ELEMENT_CONTAINER_CREATOR element_container_creator;

        enum class walk_direction {
            PREFIX,
            SUFFIX,
            INITIALIZE
        };

    public:
        sliced_subdivider(
            G& g,
            SLICE_SLOT_CONTAINER_CREATOR slice_slot_container_creator = {},
            ELEMENT_CONTAINER_CREATOR element_container_creator = {}
        )
        : whole_graph { g }
        , slice_slot_container_creator { slice_slot_container_creator }
        , element_container_creator { element_container_creator } {
            if constexpr (error_check) {
                for (const auto& _ : whole_graph.resident_nodes()) {
                    throw std::runtime_error("Graph must not have any resident nodes");
                }
            }
        }

        path_container<G, ELEMENT_CONTAINER_CREATOR, error_check> subdivide() {
            path_container<G, ELEMENT_CONTAINER_CREATOR, error_check> path_container_ {
                whole_graph,
                G::limits(
                    whole_graph.grid_down_cnt,
                    whole_graph.grid_right_cnt
                ).max_path_edge_cnt,
                element_container_creator
            };

            ED weight {
                subdivide(
                    path_container_,
                    nullptr,
                    walk_direction::INITIALIZE,
                    whole_graph.get_root_node(),
                    whole_graph.get_leaf_node()
                )
            };

            return path_container_;
        }

        ED subdivide(
            path_container<G, ELEMENT_CONTAINER_CREATOR>& path_container_,
            element<E>* parent_element,
            walk_direction dir,
            const N& root_node,
            const N& leaf_node
        ) {
            middle_sliceable_pairwise_alignment_graph<G, error_check> sub_graph {
                whole_graph,
                root_node,
                leaf_node
            };
            INDEX mid_down_offset { (sub_graph.grid_down_cnt - 1u) / 2u };
            prefix_sliceable_pairwise_alignment_graph<decltype(sub_graph), error_check> prefix_graph {
                sub_graph,
                sub_graph.slice_last_node(mid_down_offset)
            };
            suffix_sliceable_pairwise_alignment_graph<decltype(sub_graph), error_check> suffix_graph {
                sub_graph,
                sub_graph.slice_first_node(mid_down_offset)
            };
            reversed_sliceable_pairwise_alignment_graph<decltype(suffix_graph)> reversed_suffix_graph {
                suffix_graph
            };

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
            E max_edge;
            {
                forward_walker<
                    decltype(prefix_graph),
                    SLICE_SLOT_CONTAINER_CREATOR,
                    RESIDENT_SLOT_CONTAINER_CREATOR,
                    error_check
                > forward_walker_ {
                    prefix_graph,
                    slice_slot_container_creator,
                    resident_slot_container_creator
                };
                while (!forward_walker_.next()) {}

                forward_walker<
                    decltype(reversed_suffix_graph),
                    SLICE_SLOT_CONTAINER_CREATOR,
                    RESIDENT_SLOT_CONTAINER_CREATOR,
                    error_check
                > backward_walker {
                    reversed_suffix_graph,
                    slice_slot_container_creator,
                    resident_slot_container_creator
                };
                while (!backward_walker.next()) {}

                N first_node { sub_graph.slice_first_node(mid_down_offset) };
                const auto& first_forward_slot { forward_walker_.find(first_node) };
                const auto& first_backward_slot { backward_walker.find(first_node) };
                std::tuple<ED, N, E> max {
                    first_forward_slot.backtracking_weight + first_backward_slot.backtracking_weight,
                    first_node,
                    first_backward_slot.backtracking_edge // don't use first_forward_slot's backtracking_edge because it'll be unset for first
                                                          // backward_forward_slot's backtracking edge fomat is the same as first_forward_slot's backtracking_edge -- you can break it using sub_graph
                };
                for (const N& node : sub_graph.slice_nodes(mid_down_offset)) {
                    const auto forward_slot { forward_walker_.find(node) };
                    const auto backward_slot { backward_walker.find(node) };
                    std::tuple<ED, N, E> next {
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
                max_edge = max_edge_;
            }  // Everything above wrapped in its own scope so that walkers (and their associated containers) are destroyed

            const auto& n1 { whole_graph.get_edge_from(max_edge) };
            const auto& n2 { whole_graph.get_edge_to(max_edge) };
            const auto& [n1_down, n1_right] { n1 };
            const auto& [n2_down, n2_right] { n2 };
            std::cout
                    << indent_str
                    << "found: " << n1_down << 'x' << n1_right << "->" << n2_down << 'x' << n2_right
                    << " weight: " << max_weight << std::endl;

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

            const N& new_leaf_node { whole_graph.get_edge_from(max_edge) };
            std::cout << indent_str << " topleft" << std::endl;
            subdivide(
                path_container_,
                current_element,
                walk_direction::PREFIX,
                sub_graph.get_root_node(),
                new_leaf_node
            );

            const N& new_root_node { whole_graph.get_edge_to(max_edge) };
            std::cout << indent_str << " bottomright" << std::endl;
            subdivide(
                path_container_,
                current_element,
                walk_direction::SUFFIX,
                new_root_node,
                sub_graph.get_leaf_node()
            );

            indent--;

            return max_weight;
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLICED_SUBDIVIDER_H
