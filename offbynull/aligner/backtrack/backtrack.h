#ifndef OFFBYNULL_ALIGNER_BACKTRACK_BACKTRACK_H
#define OFFBYNULL_ALIGNER_BACKTRACK_BACKTRACK_H

#include <cstddef>
#include <functional>
#include <ranges>
#include <algorithm>
#include "offbynull/aligner/graphs/pairwise_extended_gap_alignment_graph.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/backtrack/container_creator.h"
#include "offbynull/aligner/backtrack/container_creators.h"
#include "offbynull/aligner/backtrack/ready_queue.h"
#include "offbynull/aligner/backtrack/slot_container.h"
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrack::backtrack {
    using offbynull::aligner::graph::graph::readable_graph;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::backtrack::slot_container::slot_container;
    using offbynull::aligner::backtrack::slot_container::slot;
    using offbynull::aligner::backtrack::ready_queue::ready_queue;
    using offbynull::aligner::backtrack::container_creator::container_creator;
    using offbynull::aligner::backtrack::container_creators::vector_container_creator;
    using offbynull::concepts::range_of_type;
    using offbynull::concepts::widenable_to_size_t;

    template<
        readable_graph G,
        widenable_to_size_t COUNT,
        weight WEIGHT,
        container_creator SLOT_ALLOCATOR=vector_container_creator<slot<typename G::N, typename G::E, COUNT, WEIGHT>>,
        container_creator PATH_ALLOCATOR=vector_container_creator<typename G::E>,
        bool error_check = true
    >
    requires requires(typename G::N n)
    {
        {n < n} -> std::same_as<bool>;
    }
    class backtracker {
    public:
        using N = typename G::N;
        using E = typename G::E;
        using slot_container_t = slot_container<N, E, COUNT, WEIGHT, SLOT_ALLOCATOR, error_check>;

        slot_container_t populate_weights_and_backtrack_pointers(
            G& g,
            std::function<WEIGHT(const E&)> get_edge_weight_func
        ) {
            // Create "slots" list
            // -------------------
            // The "slots" list tracks number of unprocessed parents for each node within the graph. The list is sorted by the
            // ordering of the node being track (node type must be orderable using std::less or < or whatever stdlib deems) such
            // that the index for a node object can be quickly found.
            const auto& slots_lazy {
                g.get_nodes()
                | std::views::transform([&](const auto& n) -> slot<N, E, COUNT, WEIGHT> {
                    std::size_t in_degree { g.get_in_degree_unique(n) };
                    COUNT in_degree_narrowed { static_cast<COUNT>(in_degree) };
                    if constexpr (error_check) {
                        if (in_degree_narrowed != in_degree) {
                            throw std::runtime_error("Narrowed but led to information loss");
                        }
                    }
                    return { n, in_degree_narrowed };
                })
            };
            slot_container_t slots {slots_lazy.begin(), slots_lazy.end()};
            // Create "ready_idxes" queue
            // --------------------------
            // The "ready_idxes" queue contains indicies within "slots" that are ready-to-process (node in that slot has had all
            // parents processed, and so it can be processed). Since root nodes have no parents, they are ready-to-process from
            // the get-go. As such, the "ready_idxes" queue is primed with the "slots" indices for root nodes (of which there
            // should be only one).
            ready_queue ready_idxes {};
            const N& root_node { g.get_root_node() };
            const auto& [root_slot_idx, root_slot] { slots.find(root_node) };
            ready_idxes.push(root_slot_idx);
            // static_assert(std::numeric_limits<float>::is_iec559, "IEEE 754 required"); // Require for inf and nan?
            root_slot.backtracking_weight = 0.0;
            // Find max path within graph
            // --------------------------
            // Using the backtracking algorithm, find the path within graph that has the maximum weight. If more than one such
            // path exists, any one of the paths will be returned.
            //
            // The backtracking algorithm is a "dynamic programming" algorithm that walks over each node. A node is walked once
            // all of its parent nodes have already been walked, where the process of "walking a node" is inspecting how much
            // weight has been accumulated within each incoming edge (weight at parent + the edge from parent to the node), and
            // selecting highest one as the edge to backtrack to.
            top:
            while (!ready_idxes.empty()) {
                std::size_t idx { ready_idxes.pop() };
                auto& current_slot { slots.at_idx(idx) };
                for (const auto& edge : g.get_inputs(current_slot.node)) {
                    const auto& src_node { g.get_edge_from(edge) };
                    if (slots.find_ref(src_node).unwalked_parent_cnt != 0) {
                        goto top;
                    }
                }
                auto incoming_accumulated {
                    g.get_inputs(current_slot.node)
                    | std::views::transform(
                        [&](const auto& edge) noexcept -> std::pair<E, WEIGHT> {
                            const auto& src_node { g.get_edge_from(edge) };
                            const slot<N, E, COUNT, WEIGHT>& src_node_slot { slots.find_ref(src_node) };
                            const auto& edge_weight { get_edge_weight_func(edge) };
                            return { edge, src_node_slot.backtracking_weight + edge_weight };
                        }
                    )
                };
                auto found {
                    std::ranges::max_element(
                        incoming_accumulated,
                        [](const std::pair<E, WEIGHT>& a, const std::pair<E, WEIGHT>& b) noexcept {
                            return a.second < b.second;
                        }
                    )
                };
                if (found != incoming_accumulated.end()) {  // if no incoming nodes found, it's a root node
                    current_slot.backtracking_edge = (*found).first;
                    current_slot.backtracking_weight = (*found).second;
                }
                // For outgoing nodes this node points to, decrement its number of unprocessed parents (this node was one of its
                // parents, and it was processed in this iteration of the loop) then add it to "ready_idxes" if it has no more
                // unprocessed parents.
                for (const auto& edge : g.get_outputs(current_slot.node)) {
                    const auto& dst_node { g.get_edge_to(edge) };
                    const auto& [dst_slot_idx, dst_slot] { slots.find(dst_node) };
                    if constexpr (error_check) {
                        if (dst_slot.unwalked_parent_cnt == 0u) {
                            throw std::runtime_error("Invalid number of unprocessed parents");
                        }
                    }
                    dst_slot.unwalked_parent_cnt = static_cast<COUNT>(dst_slot.unwalked_parent_cnt - 1u);
                    if (dst_slot.unwalked_parent_cnt == 0u) {
                        ready_idxes.push(dst_slot_idx);
                    }
                }
            }
            // Return slots container, which can go on to be used for backtracking
            return slots;
        }

        range_of_type<E> auto backtrack(
                G& g,
                slot_container_t& slots,
                const N& end_node,
                PATH_ALLOCATOR path_container_creator = {}
        ) {
            auto next_node { end_node };
            auto path { path_container_creator.create_empty(std::nullopt) };
            while (true) {
                auto node { next_node };
                if (!g.has_inputs(node)) {
                    break;
                }
                const auto& node_slot { slots.find_ref(node) };
                path.push_back(node_slot.backtracking_edge);
                next_node = g.get_edge_from(node_slot.backtracking_edge);
            }
            // At this point, path is in reverse order (from last to first). Reverse it to get it into the forward order
            // (from first to last).
            std::reverse(path.begin(), path.end());
            return path;
        }

        auto find_max_path(
                G& graph,
                const N& end_node,
                std::function<WEIGHT(const E&)> get_edge_weight_func
        ) {
            auto slots {
                populate_weights_and_backtrack_pointers(
                    graph,
                    get_edge_weight_func
                )
            };
            const auto& path { backtrack(graph, slots, end_node) };
            const auto& weight { slots.find_ref(end_node).backtracking_weight };
            return std::make_pair(path, weight);
        }

        auto find_max_path(
                G& graph,
                std::function<WEIGHT(const E&)> get_edge_weight_func
        ) {
            return find_max_path(
                graph,
                graph.get_leaf_node(),
                get_edge_weight_func
            );
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACK_BACKTRACK_H
