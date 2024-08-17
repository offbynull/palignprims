#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLICED_SUBDIVIDER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLICED_SUBDIVIDER_H

#include <iostream>
#include "path_container.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/concepts.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/bidi_walker.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/prefix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/suffix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/middle_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/reversed_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::sliced_subdivider {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::concepts::backtrackable_node;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::concepts::backtrackable_edge;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker::slot;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker::bidi_walker;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker::bidi_walker_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker::bidi_walker_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker::bidi_walker_stack_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::element;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::path_container;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::path_container_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::path_container_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::path_container_stack_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container::resident_slot_with_node;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container::resident_slot_container_stack_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::slice_slot_container::slice_slot_container_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::slice_slot_container::slice_slot_container_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::slice_slot_container::slice_slot_container_stack_container_creator_pack;
    using offbynull::aligner::graphs::prefix_sliceable_pairwise_alignment_graph::prefix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::suffix_sliceable_pairwise_alignment_graph::suffix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::middle_sliceable_pairwise_alignment_graph::middle_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::reversed_sliceable_pairwise_alignment_graph::reversed_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::concepts::weight;
    using offbynull::concepts::range_of_type;
    using offbynull::concepts::widenable_to_size_t;




    template<
        typename T,
        typename N,
        typename E,
        typename ED
    >
    concept sliced_subdivider_container_creator_pack =
        weight<ED>
        && requires(T t) {
            { t.create_slice_slot_container_container_creator_pack() } -> slice_slot_container_container_creator_pack<E, ED>;
            { t.create_path_container_container_creator_pack() } -> path_container_container_creator_pack<E>;
        };

    template<
        bool debug_mode,
        typename N,
        typename E,
        weight ED
    >
    struct sliced_subdivider_heap_container_creator_pack {
        slice_slot_container_heap_container_creator_pack<debug_mode, E, ED> create_slice_slot_container_container_creator_pack() const {
            return {};
        }

        path_container_heap_container_creator_pack<debug_mode, E> create_path_container_container_creator_pack() const {
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
        std::size_t path_edge_capacity
    >
    struct sliced_subdivider_stack_container_creator_pack {
        slice_slot_container_stack_container_creator_pack<debug_mode, E, ED, grid_right_cnt, grid_depth_cnt> create_slice_slot_container_container_creator_pack() const {
            return {};
        }

        path_container_stack_container_creator_pack<debug_mode, E, path_edge_capacity> create_path_container_container_creator_pack() const {
            return {};
        }
    };




    template<
        bool debug_mode,
        readable_sliceable_pairwise_alignment_graph G,
        sliced_subdivider_container_creator_pack<typename G::N, typename G::E, typename G::ED> CONTAINER_CREATOR_PACK=sliced_subdivider_heap_container_creator_pack<debug_mode, typename G::N, typename G::E, typename G::ED>
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

        using SLICE_SLOT_CONTAINER_CONTAINER_CREATOR_PACK=decltype(std::declval<CONTAINER_CREATOR_PACK>().create_slice_slot_container_container_creator_pack());
        using PATH_CONTAINER_CONTAINER_CREATOR_PACK=decltype(std::declval<CONTAINER_CREATOR_PACK>().create_path_container_container_creator_pack());

        CONTAINER_CREATOR_PACK container_creator_pack;

        const G& whole_graph;

        enum class walk_direction {
            PREFIX,
            SUFFIX,
            INITIALIZE
        };

    public:
        sliced_subdivider(
            const G& g,
            CONTAINER_CREATOR_PACK container_creator_pack_ = {}
        )
        : whole_graph { g }
        , container_creator_pack { container_creator_pack_ } {
            if constexpr (debug_mode) {
                for (const auto& resident_node : whole_graph.resident_nodes()) {
                    if (resident_node != whole_graph.get_root_node() && resident_node != whole_graph.get_leaf_node()) {
                        throw std::runtime_error("Graph must not have any resident nodes");
                    }
                }
            }
        }

        path_container<debug_mode, G, PATH_CONTAINER_CONTAINER_CREATOR_PACK> subdivide() {
            path_container<debug_mode, G, PATH_CONTAINER_CONTAINER_CREATOR_PACK> path_container_ {
                whole_graph,
                container_creator_pack.create_path_container_container_creator_pack()
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
            path_container<debug_mode, G, PATH_CONTAINER_CONTAINER_CREATOR_PACK>& path_container_,
            element<E>* parent_element,
            const walk_direction dir,
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
            middle_sliceable_pairwise_alignment_graph<debug_mode, G> sub_graph {
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
            if constexpr (debug_mode) {
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
                // whole_graph expected to have 2 resident nodes at most (one at root and one at leaf) -- override the
                // resident_slot_containers being created to be stack-based and maxxed out to 2.
                using BACKING_SSC_CCP_TYPE = decltype(
                    container_creator_pack
                        .create_slice_slot_container_container_creator_pack()
                );
                struct override_forward_walker_container_creator_pack {
                    BACKING_SSC_CCP_TYPE backing_ssc_ccp;
                    auto create_slice_slot_container_container_creator_pack() const {
                        return backing_ssc_ccp;
                    }
                    auto create_resident_slot_container_container_creator_pack() const {
                        return resident_slot_container_stack_container_creator_pack<debug_mode, N, E, ED, 2zu> {};
                    }
                };
                struct override_bidi_walker_container_creator_pack {
                    BACKING_SSC_CCP_TYPE backing_ssc_ccp;
                    auto create_forward_walker_container_creator_pack() {
                        return override_forward_walker_container_creator_pack { backing_ssc_ccp };
                    }
                    auto create_backward_walker_container_creator_pack() {
                        return override_forward_walker_container_creator_pack { backing_ssc_ccp };
                    }
                };
                // create bidiwalker
                using BIDI_WALKER_TYPE = bidi_walker<
                    debug_mode,
                    decltype(sub_graph),
                    override_bidi_walker_container_creator_pack
                >;
                BIDI_WALKER_TYPE bidi_walker_ {
                    BIDI_WALKER_TYPE::create_and_initialize(
                        sub_graph,
                        mid_down_offset,
                        override_bidi_walker_container_creator_pack {
                            container_creator_pack
                                .create_slice_slot_container_container_creator_pack()
                        }
                    )
                };
                // walk middle, pulling out node/edge with max weight
                auto mid_slice { sub_graph.slice_nodes(mid_down_offset) };
                if constexpr (debug_mode) {
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
                if constexpr (debug_mode) {
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
                    if constexpr (debug_mode) {
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
