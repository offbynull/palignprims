#ifndef OFFBYNULL_ALIGNER_BACKTRACK_BACKTRACK_H
#define OFFBYNULL_ALIGNER_BACKTRACK_BACKTRACK_H

#include <tuple>
#include <vector>
#include <functional>
#include <ranges>
#include <algorithm>
#include "offbynull/aligner/backtrack/ready_queue.h"
#include "offbynull/aligner/backtrack/slot_container.h"

namespace offbynull::aligner::backtrack::backtrack {

    using offbynull::aligner::backtrack::slot_container::slot_container;
    using offbynull::aligner::backtrack::slot_container::slot;
    using offbynull::aligner::backtrack::ready_queue::ready_queue;

    template<typename G, bool error_check = true>
    slot_container<typename G::N, typename G::E> populate_weights_and_backtrack_pointers(
            G& g,
            const typename G::N& from_node,
            std::function<double(typename G::E)> get_edge_weight_func
    ) {
        if constexpr (error_check) {
            if (from_node != g.get_root_node()) {  // get_root_node() should throw exception if none or > 1
                throw std::runtime_error("Start node isn't root node");
            }
        }
        using N = typename G::N;
        using E = typename G::E;
        // Create "slots" list
        // -------------------
        // The "slots" list tracks number of unprocessed parents for each node within the graph. The list is sorted by the
        // ordering of the node being track (node type must be orderable using std::less or < or whatever stdlib deems) such
        // that the index for a node object can be quickly found.
        const auto& slots_lazy {
            g.get_nodes()
            | std::views::transform([&](const auto& n) noexcept -> slot<N, E> { return { n, g.get_in_degree(n) }; })
        };
        slot_container<N, E> slots(slots_lazy.begin(), slots_lazy.end());
        // Create "ready_idxes" queue
        // --------------------------
        // The "ready_idxes" queue contains indicies within "slots" that are ready-to-process (node in that slot has had all
        // parents processed, and so it can be processed). Since root nodes have no parents, they are ready-to-process from
        // the get-go. As such, the "ready_idxes" queue is primed with the "slots" indices for root nodes.
        ready_queue ready_idxes {};
        for (const auto& node : g.get_root_nodes()) {
            const auto& [root_slot_idx, root_slot] { slots.find(node) };
            ready_idxes.push(root_slot_idx);
            // static_assert(std::numeric_limits<float>::is_iec559, "IEEE 754 required");  // Because of -infinity, or will inifinity() error on its own if not supported?
            root_slot.backtracking_weight = -std::numeric_limits<double>::infinity();
        }
        slots.find_ref(from_node).backtracking_weight = 0.0;
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
            size_t idx { ready_idxes.pop() };
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
                    [&](const auto& edge) noexcept -> std::pair<E, double> {
                        const auto& src_node { g.get_edge_from(edge) };
                        const slot<N, E>& src_node_slot { slots.find_ref(src_node) };
                        const auto& edge_weight { get_edge_weight_func(edge) };
                        return { edge, src_node_slot.backtracking_weight + edge_weight };
                    }
                )
            };
            auto found {
                std::ranges::max_element(
                    incoming_accumulated,
                    [](const std::pair<E, double>& a, const std::pair<E, double>& b) noexcept {
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
            const auto outgoing_nodes {
                g.get_outputs(current_slot.node)
                | std::views::transform([&g](const auto e) { return g.get_edge_to(e); })
            };
            for (const auto& edge : g.get_outputs(current_slot.node)) {
                const auto& dst_node { g.get_edge_to(edge) };
                const auto& [dst_slot_idx, dst_slot] { slots.find(dst_node) };
                if constexpr (error_check) {
                    if (dst_slot.unwalked_parent_cnt == 0u) {
                        throw std::runtime_error("Invalid number of unprocessed parents");
                    }
                }
                dst_slot.unwalked_parent_cnt -= 1u;
                if (dst_slot.unwalked_parent_cnt == 0u) {
                    ready_idxes.push(dst_slot_idx);
                }
            }
        }
        // Return slots container, which can go on to be used for backtracking
        return slots;
    }

    template<typename G>
    std::vector<typename G::E> backtrack(
            G& g,
            slot_container<typename G::N, typename G::E>& slots,
            const typename G::N& end_node
    ) {
        using N = typename G::N;
        using E = typename G::E;
        auto next_node { end_node };
        std::vector<E> path {};
        while (true) {
            auto node { next_node };
            if (g.get_in_degree(node) == 0u) {
                break;
            }
            slot<N, E>& node_slot { slots.find_ref(node) };
            path.push_back(node_slot.backtracking_edge);
            next_node = g.get_edge_from(node_slot.backtracking_edge);
        }
        // At this point, path is in reverse order (from last to first). Reverse it to get it into the forward order
        // (from first to last).
        std::reverse(path.begin(), path.end());
        return path;
    }

    template<typename G>
    std::tuple<std::vector<typename G::E>, double> find_max_path(
            G& graph,
            const typename G::N& start_node,
            const typename G::N& end_node,
            std::function<double(typename G::E)> get_edge_weight_func
    ) {
        using E = typename G::E;
        auto slots {
            populate_weights_and_backtrack_pointers(
                graph,
                start_node,
                get_edge_weight_func
            )
        };
        const auto& path { backtrack(graph, slots, end_node) };
        const auto& weight { slots.find_ref(end_node).backtracking_weight };
        return std::tuple<std::vector<E>, double> { path, weight };
    }
}

#endif //OFFBYNULL_ALIGNER_BACKTRACK_BACKTRACK_H
