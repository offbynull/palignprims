#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLICED_SUBDIVIDER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLICED_SUBDIVIDER_H

#include <ranges>
#include <algorithm>
#include <iostream>
#include "path_container.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/concepts.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/container_creator_packs.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/bidi_walker.h"
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
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker::slot;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker::bidi_walker;
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

        // whole_graph expected to have 2 resident nodes at most (one at root and one at leaf), so enforce this as a
        // 2-length static_vector
        using RESIDENT_SLOT_CONTAINER_CREATOR = static_vector_container_creator<
            node_searchable_slot<
                typename G::N,
                typename G::E,
                typename G::ED
            >,
            2zu,
            false
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
                for (const auto& resident_node : whole_graph.resident_nodes()) {
                    if (resident_node != whole_graph.get_root_node() && resident_node != whole_graph.get_leaf_node()) {
                        throw std::runtime_error("Graph must not have any resident nodes");
                    }
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
            const N& leaf_node,
            const ED existing_weight_at_root = {},
            const ED existing_weight_at_leaf = {}
        ) {
            // std::string indent_str {};
            // static int indent { 0 };
            // for (auto i { 0 }; i < indent; i++) {
            //     indent_str += ' ';
            // }
            middle_sliceable_pairwise_alignment_graph<G, error_check> sub_graph {
                whole_graph,
                root_node,
                leaf_node
            };
            const auto& [root_down_offset, root_right_offset, root_depth] { whole_graph.node_to_grid_offsets(sub_graph.get_root_node()) };
            const auto& [leaf_down_offset, leaf_right_offset, leaf_depth] { whole_graph.node_to_grid_offsets(sub_graph.get_leaf_node()) };
            // std::cout
            //         << indent_str
            //         << "root: [" << root_down_offset << "," << root_right_offset << ',' << root_depth << "]"
            //         << " leaf: [" << leaf_down_offset << "," << leaf_right_offset << ',' << leaf_depth << "]"
            //         << std::endl;
            if constexpr (error_check) {
                if (!whole_graph.is_reachable(root_node, leaf_node)) {
                    throw std::runtime_error("Root doesn't reach leaf");
                }
            }
            INDEX mid_down_offset { (sub_graph.grid_down_cnt - 1u) / 2u };

            if (root_node == leaf_node) {
                return 0.0;
            }
            ED max_path_weight;
            E max_edge;
            ED max_edge_weight;
            ED before_max_edge_weight;
            ED after_max_edge_weight;
            bool max_edge_assigned {};
            {
                using BIDI_WALKER_TYPE = bidi_walker<decltype(sub_graph), SLICE_SLOT_CONTAINER_CREATOR, RESIDENT_SLOT_CONTAINER_CREATOR, error_check>;
                BIDI_WALKER_TYPE bidi_walker_ { BIDI_WALKER_TYPE::create_and_initialize(sub_graph, mid_down_offset) };

                auto mid_slice { sub_graph.slice_nodes(mid_down_offset) };
                if constexpr (error_check) {
                    if (mid_slice.begin() == mid_slice.end()) {
                        throw std::runtime_error("Slice should never be empty");
                    }
                }
                for (const N& node : mid_slice) {
                    const auto& [forward_slot, backward_slot] { bidi_walker_.find(node) };
                    const auto new_potential_path_weight { existing_weight_at_root + forward_slot.backtracking_weight + backward_slot.backtracking_weight + existing_weight_at_leaf };
                    if (!max_edge_assigned || new_potential_path_weight > max_path_weight) {
                        if (forward_slot.backtracking_edge.has_value()) {
                            max_edge = *forward_slot.backtracking_edge;
                            max_edge_weight = sub_graph.get_edge_data(max_edge);
                            before_max_edge_weight = forward_slot.backtracking_weight - max_edge_weight;
                            after_max_edge_weight = backward_slot.backtracking_weight;
                        } else if (backward_slot.backtracking_edge.has_value()) {
                            max_edge = *backward_slot.backtracking_edge;
                            max_edge_weight = sub_graph.get_edge_data(max_edge);
                            before_max_edge_weight = forward_slot.backtracking_weight;
                            after_max_edge_weight = backward_slot.backtracking_weight - max_edge_weight;
                        } else [[unlikely]] {
                            throw std::runtime_error("Should never happen");
                        }
                        max_path_weight = new_potential_path_weight;
                        max_edge_assigned = true;
                    }
                }
                if constexpr (error_check) {
                    if (!max_edge_assigned) {
                        throw std::runtime_error("Couldn't find a reachable node");
                    }
                }
            }  // Everything above wrapped in its own scope so that walkers (and their associated containers) are destroyed

            const auto& n1 { whole_graph.get_edge_from(max_edge) };
            const auto& n2 { whole_graph.get_edge_to(max_edge) };
            const auto& [n1_down, n1_right, n1_depth] { whole_graph.node_to_grid_offsets(n1) };
            const auto& [n2_down, n2_right, n2_depth] { whole_graph.node_to_grid_offsets(n2) };
            // std::cout
            //         << indent_str
            //         << "found: " << n1_down << 'x' << n1_right << 'x' << n1_depth << "->" << n2_down << 'x' << n2_right << 'x' << n2_depth
            //         << " full_weight: " << max_path_weight
            //         << " pre_weight: " << before_max_edge_weight
            //         << " edge_weight: " << max_edge_weight
            //         << " post_weight: " << after_max_edge_weight << std::endl;

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
            // indent++;
            // std::cout << indent_str << " topleft" << std::endl;
            subdivide(
                path_container_,
                current_element,
                walk_direction::PREFIX,
                sub_graph.get_root_node(),
                sub_graph.get_edge_from(max_edge),
                existing_weight_at_root,
                existing_weight_at_leaf + after_max_edge_weight + max_edge_weight
            );
            // std::cout << indent_str << " bottomright" << std::endl;
            subdivide(
                path_container_,
                current_element,
                walk_direction::SUFFIX,
                sub_graph.get_edge_to(max_edge),
                sub_graph.get_leaf_node(),
                existing_weight_at_root + before_max_edge_weight + max_edge_weight,
                existing_weight_at_leaf
            );
            // indent--;

            return max_path_weight;
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLICED_SUBDIVIDER_H
