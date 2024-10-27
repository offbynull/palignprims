#ifndef OFFBYNULL_ALIGNER_GRAPH_MULTITHREADED_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPH_MULTITHREADED_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H

#include <cstddef>
#include <cstdint>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <tuple>
#include <algorithm>
#include "offbynull/concepts.h"
#include "offbynull/helpers/join_bidirectional_view.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"

namespace offbynull::aligner::graph::multithreaded_sliceable_pairwise_alignment_graph {
    using offbynull::concepts::bidirectional_range_of_bidirectional_range_of_one_of;
    using offbynull::concepts::unqualified_value_type;
    using offbynull::helpers::join_bidirectional_view::join_bidirectional_view_adaptor;
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_pairwise_alignment_graph;

    enum class axis : std::uint8_t {
        DOWN_FROM_TOP_LEFT = 0u,
        RIGHT_FROM_BOTTOM_LEFT = 1u
    };

    template <typename G>
    concept readable_multithreaded_sliceable_pairwise_alignment_graph =
        unqualified_value_type<G>
        && readable_sliceable_pairwise_alignment_graph<G>
        && requires(
            G g,
            typename G::N node,
            axis axis_,
            typename G::INDEX axis_position,
            std::size_t max_segments
        ) {
            { g.segmented_diagonal_nodes(axis_, axis_position, max_segments) }
                -> bidirectional_range_of_bidirectional_range_of_one_of<typename G::N, const typename G::N&>;
            { g.segmented_diagonal_nodes(axis_, axis_position, node, node, max_segments) }
                -> bidirectional_range_of_bidirectional_range_of_one_of<typename G::N, const typename G::N&>;
        };




    // Reference implementation for segmented_diagonal_nodes()
    // -------------------------------------------------------
    // You can either ...
    // 1. have the readable_sliceable_pairwise_alignment_graph's segmented_diagonal_nodes() function call into this function
    // 2. write a custom implementation for the readable_sliceable_pairwise_alignment_graph's segmented_diagonal_nodes() (typically more
    //    optimized than) and test against this to ensure things are working correctly.
    template<
        bool debug_mode,
        typename G  /* Should be readable_multithreaded_sliceable_pairwise_alignment_graph, but can't do this because of cyclic dep. */
    >
    std::ranges::bidirectional_range auto generic_segmented_diagonal_nodes(
        const G& g,
        axis axis_,
        typename G::INDEX axis_position,
        const typename G::N& root_node,
        const typename G::N& leaf_node,
        std::size_t max_segments
    ) {
        using INDEX = typename G::INDEX;
        using N = typename G::N;

        if constexpr (debug_mode) {
            if (!g.has_node(root_node) || !g.has_node(leaf_node)) {
                throw std::runtime_error { "Bad root / leaf node" };
            }
            switch (axis_) {
                case axis::DOWN_FROM_TOP_LEFT:
                    if (axis_position + root_node.down > leaf_node.down) {
                        throw std::runtime_error { "Bad node" };
                    }
                break;
                case axis::RIGHT_FROM_BOTTOM_LEFT:
                    if (axis_position + root_node.right > leaf_node.right) {
                        throw std::runtime_error { "Bad node" };
                    }
                break;
                [[unlikely]] default:
                    throw std::runtime_error { "This should never happen" };
            }
            if (!g.is_reachable(root_node, leaf_node)) {
                throw std::runtime_error { "Root node unreachable to leaf node" };
            }
        }

        /*
                          0
                          1 1
         0 1 2 3 4        2 2 2
         1 2 3 4 5        3 3 3
         2 3 4 5 6        4 4 4
                          5 5
                          6


                          0
         0 1 2            1 1
         1 2 3            2 2 2
         2 3 4            3 3 3
         3 4 5            4 4 4
         4 5 6            5 5
                          6
         */
        // ADJUST EVERYTHING SO THE GRID IS ISOLATED BETWEEN root_node AND leaf_node
        // axis_position SHOULD ALREADY BE ISOLATED
        const auto& [root_down, root_right, root_depth] { g.node_to_grid_offset(root_node) };
        const auto& [leaf_down, leaf_right, leaf_depth] { g.node_to_grid_offset(leaf_node) };
        const INDEX isolated_down_cnt { leaf_down - root_down + 1u };
        const INDEX isolated_right_cnt { leaf_right - root_right + 1u };
        std::pair<INDEX, INDEX> isolated_grid_start;
        std::pair<INDEX, INDEX> isolated_grid_stop;  // inclusive
        switch (axis_) {
            case axis::DOWN_FROM_TOP_LEFT: {
                isolated_grid_start = { axis_position, 0u };
                if (axis_position < isolated_right_cnt) {
                    isolated_grid_stop = { 0u, axis_position };
                } else {
                    isolated_grid_stop = { axis_position - (isolated_right_cnt - 1u), isolated_right_cnt - 1u };
                }
                break;
            }
            case axis::RIGHT_FROM_BOTTOM_LEFT: {
                isolated_grid_start = { isolated_down_cnt - 1u, axis_position };
                if (isolated_down_cnt < isolated_right_cnt) {
                    if (axis_position < isolated_down_cnt) {
                        isolated_grid_stop = { 0u, axis_position + isolated_right_cnt - 1u };  // ok
                    } else {
                        isolated_grid_stop = { axis_position - isolated_down_cnt + 1u, isolated_right_cnt - 1u };  // ok
                    }
                } else {
                    isolated_grid_stop = { isolated_down_cnt - (isolated_right_cnt - axis_position), isolated_right_cnt - 1u }; // ok
                }
                break;
            }
            [[unlikely]] default: {
                if constexpr (debug_mode) {
                    throw std::runtime_error { "This should never happen" };
                }
                std::unreachable();
            }
        }

        // isolated_grid_start's down will always be below or at that of isolated_grid_stop's down, so no need to worry about rollover for
        // isolated_diagonal_len.
        INDEX isolated_diagonal_len { std::get<0>(isolated_grid_start) - std::get<0>(isolated_grid_stop) + 1u };
        INDEX isolated_segments { std::min(max_segments, isolated_diagonal_len) };
        INDEX isolated_nodes_per_segment { isolated_diagonal_len / isolated_segments };
        // Add 1 to isolated_nodes_per_segment if there wasn't a clean division. Why? Because the way nodes are broken up into segments when
        // 1 isn't added, any remainder gets pushed to the last worker (code further below). For example, imagine that ...
        //
        // * isolated_diagonal_len == 4 and isolated_segments == 4, 4/4 == 1, the workload will break down be as [*] [*] [*] [*]
        // * isolated_diagonal_len == 5 and isolated_segments == 4, 4/5 == 1, the workload will break down be as [*] [*] [*] [**]
        // * isolated_diagonal_len == 6 and isolated_segments == 4, 4/6 == 1, the workload will break down be as [*] [*] [*] [***]
        // * isolated_diagonal_len == 7 and isolated_segments == 4, 4/7 == 1, the workload will break down be as [*] [*] [*] [****]
        // * isolated_diagonal_len == 8 and isolated_segments == 4, 4/8 == 2, the workload will break down be as [**] [**] [**] [**]
        //
        // With this change, nodes are more evenly distributed between segments. For example, imagine that ...
        //
        // * isolated_diagonal_len == 4 and isolated_segments == 4, 4/4+0 == 1, the workload will break down be as [*] [*] [*] [*]
        // * isolated_diagonal_len == 5 and isolated_segments == 4, 4/5+1 == 2, the workload will break down be as [**] [**] [*] []
        // * isolated_diagonal_len == 6 and isolated_segments == 4, 4/6+1 == 2, the workload will break down be as [**] [**] [**] []
        // * isolated_diagonal_len == 7 and isolated_segments == 4, 4/7+1 == 2, the workload will break down be as [**] [**] [**] [*]
        // * isolated_diagonal_len == 8 and isolated_segments == 4, 4/8+0 == 2, the workload will break down be as [**] [**] [**] [**]
        //
        // The idea is that each segment is processed by a single core (4 segments = 4 cores processing in parallel). As such, the total
        // processing time should be only as long as the longest segment. When one of the segments is lop-sided (as in the example where 1
        // is not added), it'll unduly add to the processing time. This is assuming that each node requires roughly the same amount of
        // processing time.
        if (isolated_diagonal_len % isolated_segments != 0u) {
            ++isolated_nodes_per_segment;
            // Update isolated_segments based on incremented isolated_nodes_per_segment. Now that more nodes are being consumed in each
            // segment, the number of segments needed may get reduced.
            bool extra_segment { isolated_diagonal_len % isolated_nodes_per_segment != 0u }; // Used to add extra segment if not cleanly div
            isolated_segments = std::min(
                isolated_segments,
                isolated_diagonal_len / isolated_nodes_per_segment + (extra_segment ? 1u : 0u)
            );
        }
        return
            std::views::iota(0zu, isolated_segments)
            | std::views::transform(
                [
                    &g,
                    root_node,
                    leaf_node,
                    isolated_grid_start,
                    isolated_grid_stop,
                    isolated_nodes_per_segment,
                    isolated_segments
                ](const auto& segment) {
                    const auto& [root_down_, root_right_, _] { g.node_to_grid_offset(root_node) };
                    // First grid offset in segment (this is NOT isolated)
                    std::pair<INDEX, INDEX> segment_first {
                        root_down_ + std::get<0>(isolated_grid_start) - (segment * isolated_nodes_per_segment),
                        root_right_ + std::get<1>(isolated_grid_start) + (segment * isolated_nodes_per_segment)
                    };
                    // Last grid offset in segment (this is NOT isolated)
                    std::pair<INDEX, INDEX> segment_last;
                    if (segment != isolated_segments - 1u) {
                        segment_last = {
                            std::get<0>(segment_first) - isolated_nodes_per_segment + 1u, // +1 because exclusive
                            std::get<1>(segment_first) + isolated_nodes_per_segment - 1u  // -1 because exclusive
                        };
                    } else {
                        // Ensure last segment consumes the remainder -- division used to produce isolated_nodes_per_segment may have
                        // resulted in rounding.
                        segment_last = {
                            root_down_ + std::get<0>(isolated_grid_stop),
                            root_right_ + std::get<1>(isolated_grid_stop)
                        };
                    }
                    // Return range that walks between the two (inclusive)
                    const auto& [segment_first_down, segment_first_right] { segment_first };
                    const auto& [segment_last_down, segment_last_right] { segment_last };
                    INDEX steps { segment_first_down - segment_last_down + 1u };
                    return
                        std::views::iota(0zu, steps)
                        | std::views::transform(
                            [&g, segment_first_down, segment_first_right](const INDEX& step) {
                                return g.grid_offset_to_nodes(
                                    segment_first_down - step,
                                    segment_first_right + step
                                );
                            }
                        )
                        | join_bidirectional_view_adaptor {}
                        | std::views::filter(
                            [&g, root_node, leaf_node](const N& node) {
                                return g.is_reachable(root_node, node) && g.is_reachable(node, leaf_node);
                            }
                        );
                }
            );
    }

    template<
        bool debug_mode,
        typename G  /* Should be readable_multithreaded_sliceable_pairwise_alignment_graph, but can't do this because of cyclic dep. */
    >
    std::ranges::bidirectional_range auto generic_segmented_diagonal_nodes(
        const G& g,
        axis axis_,
        typename G::INDEX axis_position,
        std::size_t max_segments
    ) {
        return generic_segmented_diagonal_nodes<debug_mode, G>(
            g,
            axis_,
            axis_position,
            g.get_root_node(),
            g.get_leaf_node(),
            max_segments
        );
    }
}

#endif //OFFBYNULL_ALIGNER_GRAPH_MULTITHREADED_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
