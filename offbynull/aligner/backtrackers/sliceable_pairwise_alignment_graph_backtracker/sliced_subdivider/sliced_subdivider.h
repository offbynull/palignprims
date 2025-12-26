#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLICED_SUBDIVIDER_SLICED_SUBDIVIDER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLICED_SUBDIVIDER_SLICED_SUBDIVIDER_H

#include <string>
#include <iostream>
#include <ostream>
#include <utility>
#include <stdexcept>
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/path_container/path_container.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/path_container/element.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtrackable_node.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtrackable_edge.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/sliced_subdivider/sliced_subdivider_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/sliced_subdivider/sliced_subdivider_heap_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_slot_container/resident_slot_container_stack_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/bidi_walker/bidi_walker.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/prefix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/suffix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/middle_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/reversed_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::sliced_subdivider::sliced_subdivider {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::sliceable_pairwise_alignment_graph;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtrackable_node::backtrackable_node;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtrackable_edge::backtrackable_edge;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::sliced_subdivider
        ::sliced_subdivider_container_creator_pack::sliced_subdivider_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::sliced_subdivider
        ::sliced_subdivider_heap_container_creator_pack::sliced_subdivider_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker::bidi_walker::bidi_walker;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::path_container::element;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::path_container::path_container;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container
        ::resident_slot_container_stack_container_creator_pack::resident_slot_container_stack_container_creator_pack;
    using offbynull::aligner::graphs::prefix_sliceable_pairwise_alignment_graph::prefix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::suffix_sliceable_pairwise_alignment_graph::suffix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::middle_sliceable_pairwise_alignment_graph::middle_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::reversed_sliceable_pairwise_alignment_graph::reversed_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::concepts::weight;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::concepts::unqualified_object_type;

    /**
     * Sliced subdivider finds a maximally-weighted path through a
     * @ref offbynull::aligner::graph::sliceable_pairwise_alignment_graph::sliceable_pairwise_alignment_graph "sliceable pairwise alignment
     * graph"'s
     * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::segment::segment "segment"
     * by recursively
     * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker::bidi_walker::bidi_walker
     * "walking" and bisecting that segment. At each recursive step, the process walks the middle row of the segment to identify the edge
     * converging to the greatest weight. The segment is then split on that edge and the process is re-applied to the two new segments.
     *
     * ```
     *         original                      original split                original.top split                 original.top.top split
     * *---->*---->*---->*---->*         *---->*                           *---->*                            *---->*
     * |'.   |'.   |'.   |'.   |         |'.   |                           |'.   |                                  |
     * |  '. |  '. |  '. |  '. |         |  '. |                           |  '. |                                  |
     * v    vv    vv    vv    vv         v    vv                           v    vv                                  v
     * *---->*---->*---->*---->*         *---->*                           *---->*                                  *
     * |'.   |'.   |'.   |'.   |         |'.   |                                 |
     * |  '. |  '. |  '. |  '. |         |  '. |                                 |
     * v    vv    vv    vv    vv         v    vv                                 v
     * *---->*---->*---->*---->*         *---->*                                 *
     * |'.   |'.   |'.   |'.   |                '.
     * |  '. |  '. |  '. |  '. |                  '.
     * v    vv    vv    vv    vv                    v                           original.bottom split         original.bottom.bottom split
     * *---->*---->*---->*---->*                     *---->*---->*                      *---->*---->*                       *
     * |'.   |'.   |'.   |'.   |                     |'.   |'.   |                            |'.   |                        '.
     * |  '. |  '. |  '. |  '. |                     |  '. |  '. |                            |  '. |                          '.
     * v    vv    vv    vv    vv                     v    vv    vv                            v    vv                            v
     * *---->*---->*---->*---->*                     *---->*---->*                            *---->*                             *
     * ```
     *
     * The converging / bisecting edges form a maximally-weighted path through the segment.
     *
     *```
     *    maximally-weighted path
     * *---->*
     *       |
     *       |
     *       v
     *       *
     *       |
     *       |
     *       v
     *       *
     *        '.
     *          '.
     *            v
     *             *---->*
     *                    '.
     *                      '.
     *                        v
     *                         *
     * ```
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam G Graph type.
     * @tparam CONTAINER_CREATOR_PACK Container factory type.
     */
    template<
        bool debug_mode,
        sliceable_pairwise_alignment_graph G,
        sliced_subdivider_container_creator_pack<
            typename G::N,
            typename G::E,
            typename G::ED
        > CONTAINER_CREATOR_PACK = sliced_subdivider_heap_container_creator_pack<
            debug_mode,
            typename G::E,
            typename G::ED
        >
    >
    requires backtrackable_node<typename G::N> &&
        backtrackable_edge<typename G::E>
    class sliced_subdivider {
    private:
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using N_INDEX = typename G::N_INDEX;

        static constexpr N_INDEX I0 { static_cast<N_INDEX>(0zu) };
        static constexpr N_INDEX I1 { static_cast<N_INDEX>(1zu) };
        static constexpr N_INDEX I2 { static_cast<N_INDEX>(2zu) };

        using ROW_SLOT_CONTAINER_CONTAINER_CREATOR_PACK =
            decltype(std::declval<CONTAINER_CREATOR_PACK>().create_row_slot_container_container_creator_pack());
        using PATH_CONTAINER_CONTAINER_CREATOR_PACK =
            decltype(std::declval<CONTAINER_CREATOR_PACK>().create_path_container_container_creator_pack());

        const G& whole_graph;

        CONTAINER_CREATOR_PACK container_creator_pack;

        enum class walk_direction {
            PREFIX,
            SUFFIX,
            INITIALIZE
        };

    public:
        /**
         * Construct an
         * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::sliced_subdivider::sliced_subdivider::sliced_subdivider
         * object.
         *
         * Although the algorithm is intended to operate on a
         * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::segment::segment
         * "segment" of a @ref offbynull::aligner::graph::sliceable_pairwise_alignment_graph::sliceable_pairwise_alignment_graph "sliceable
         * pairwise alignment graph", it can technically work on any
         * @ref offbynull::aligner::graph::sliceable_pairwise_alignment_graph::sliceable_pairwise_alignment_graph "sliceable pairwise
         * alignment graph" so long as it doesn't contain resident nodes. When passing in a segment, the typical workflow is to create a
         * view of the graph isolated to the segment (see
         * @ref offbynull::aligner::graphs::middle_sliceable_pairwise_alignment_graph::middle_sliceable_pairwise_alignment_graph).
         *
         * @param g Graph (view of a segment).
         * @param container_creator_pack_ Container factory.
         */
        sliced_subdivider(
            const G& g,
            CONTAINER_CREATOR_PACK container_creator_pack_ = {}
        )
        : whole_graph { g }
        , container_creator_pack { container_creator_pack_ } {
            if constexpr (debug_mode) {
                for (const auto& resident_node : whole_graph.resident_nodes()) {
                    if (resident_node != whole_graph.get_root_node() && resident_node != whole_graph.get_leaf_node()) {
                        throw std::runtime_error { "Graph must not have any resident nodes" };
                    }
                }
            }
        }

        /**
         * Find maximally-weighted path throughout the graph segment assigned to this object.
         *
         * @return Maximally-weighted path through graph segment.
         */
        path_container<debug_mode, G, PATH_CONTAINER_CONTAINER_CREATOR_PACK> subdivide() {
            path_container<debug_mode, G, PATH_CONTAINER_CONTAINER_CREATOR_PACK> path_container_ {
                whole_graph,
                container_creator_pack.create_path_container_container_creator_pack()
            };

            subdivide(
                path_container_,
                nullptr,
                walk_direction::INITIALIZE,
                whole_graph.get_root_node(),
                whole_graph.get_leaf_node()
            );

            return path_container_;
        }

    private:
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
            // std::cout
            //         << indent_str
            //         << "root: " << sub_graph.get_root_node()
            //         << " leaf: " << sub_graph.get_leaf_node()
            //         << std::endl;
            if constexpr (debug_mode) {
                if (!whole_graph.is_reachable(root_node, leaf_node)) {
                    throw std::runtime_error { "Root doesn't reach leaf" };
                }
            }
            N_INDEX mid_down_offset {
                static_cast<N_INDEX>((sub_graph.grid_down_cnt - I1) / I2)  // Cast to prevent narrowing warning
            };

            if (root_node == leaf_node) {
                return ED { 0zu };
            }
            ED max_path_weight;
            E max_edge;
            ED max_edge_weight;
            ED before_max_edge_weight;
            ED after_max_edge_weight;
            {
                bool max_edge_assigned {};
                // whole_graph expected to have 2 resident nodes at most (one at root and one at leaf) -- override the
                // resident_slot_containers being created to be stack-based and maxxed out to 2.
                using BACKING_SSC_CCP_TYPE = decltype(
                    container_creator_pack
                        .create_row_slot_container_container_creator_pack()
                );
                struct override_forward_walker_container_creator_pack {
                    BACKING_SSC_CCP_TYPE backing_ssc_ccp;
                    auto create_row_slot_container_container_creator_pack() const {
                        return backing_ssc_ccp;
                    }
                    auto create_resident_slot_container_container_creator_pack() const {
                        return resident_slot_container_stack_container_creator_pack<debug_mode, N, E, ED, 2zu> {};
                    }
                };
                struct override_bidi_walker_container_creator_pack {
                    BACKING_SSC_CCP_TYPE backing_ssc_ccp;
                    auto create_forward_walker_container_creator_pack() const {
                        return override_forward_walker_container_creator_pack { backing_ssc_ccp };
                    }
                    auto create_backward_walker_container_creator_pack() const {
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
                                .create_row_slot_container_container_creator_pack()
                        }
                    )
                };
                // walk middle, pulling out node/edge with max weight
                auto mid_row { sub_graph.row_nodes(mid_down_offset) };
                if constexpr (debug_mode) {
                    if (mid_row.begin() == mid_row.end()) {
                        throw std::runtime_error { "Row should never be empty" };
                    }
                }
                for (const N& node : mid_row) {
                    const auto& [forward_slot, backward_slot] { bidi_walker_.find(node) };
                    const ED new_potential_path_weight {
                        static_cast<ED>(
                            existing_weight_at_root
                            + forward_slot.backtracking_weight
                            + backward_slot.backtracking_weight
                            + existing_weight_at_leaf
                        )  // Cast to prevent narrowing warning
                    };
                    if (!max_edge_assigned || new_potential_path_weight > max_path_weight) {
                        if (forward_slot.backtracking_edge.has_value()) {
                            max_edge = *forward_slot.backtracking_edge;
                            max_edge_weight = sub_graph.get_edge_data(max_edge);
                            before_max_edge_weight = static_cast<ED>(
                                forward_slot.backtracking_weight - max_edge_weight
                            );  // Cast to prevent narrowing warning
                            after_max_edge_weight = backward_slot.backtracking_weight;
                        } else if (backward_slot.backtracking_edge.has_value()) {
                            max_edge = *backward_slot.backtracking_edge;
                            max_edge_weight = sub_graph.get_edge_data(max_edge);
                            before_max_edge_weight = forward_slot.backtracking_weight;
                            after_max_edge_weight = static_cast<ED>(
                                backward_slot.backtracking_weight - max_edge_weight
                            );  // Cast to prevent narrowing warning
                        } else [[unlikely]] {
                            throw std::runtime_error { "This should never happen" };
                        }
                        max_path_weight = new_potential_path_weight;
                        max_edge_assigned = true;
                    }
                }
                if constexpr (debug_mode) {
                    if (!max_edge_assigned) {
                        throw std::runtime_error { "Couldn't find a reachable node" };
                    }
                }
            }  // Everything above wrapped in its own scope so that walkers (and their associated containers) are destroyed

            // std::cout
            //         << indent_str
            //         << "found: " << max_edge
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
                        throw std::runtime_error { "This should never happen" };
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
                static_cast<ED>(existing_weight_at_leaf + after_max_edge_weight + max_edge_weight)  // Cast to prevent narrowing warning
            );
            // std::cout << indent_str << " bottomright" << std::endl;
            subdivide(
                path_container_,
                current_element,
                walk_direction::SUFFIX,
                sub_graph.get_edge_to(max_edge),
                sub_graph.get_leaf_node(),
                static_cast<ED>(existing_weight_at_root + before_max_edge_weight + max_edge_weight),  // Cast to prevent narrowing warning
                existing_weight_at_leaf
            );
            // indent--;

            return max_path_weight;
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLICED_SUBDIVIDER_SLICED_SUBDIVIDER_H
