#ifndef BACKTRACKER_2_H
#define BACKTRACKER_2_H

#include <tuple>
#include <optional>
#include <vector>
#include <functional>
#include <ranges>
#include <algorithm>

namespace offbynull::aligner::backtracker {
    template<typename G, bool error_check = true>
    void populate_weights_and_backtrack_pointers(
            G& g,
            const typename G::N& from_node,
            std::function<void(typename G::N, std::optional<double>, std::optional<typename G::E>)> set_node_data_func,
            std::function<std::tuple<std::optional<double>, std::optional<typename G::E>>(typename G::N)> get_node_data_func,
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
        //
        // The helper functions provided below (as lambdas) are there to simplify the code.
        struct slot {
            N node;
            size_t unwalked_parent_cnt;
        };
        const auto& slots_lazy {
            g.get_nodes()
            | std::views::transform([&](const auto& n) noexcept -> slot { return { n, g.get_in_degree(n) }; })
        };
        std::vector<slot> slots(slots_lazy.begin(), slots_lazy.end());
        struct slots_comparator {
            bool operator()(const slot& lhs, const slot& rhs) const {
                return lhs.node < rhs.node;
            }

            bool operator()(const slot& lhs, const N& rhs) const {
                return lhs.node < rhs;
            }

            bool operator()(const N& lhs, const slot& rhs) const {
                return lhs < rhs.node;
            }
        };
        std::ranges::sort(
            slots.begin(),
            slots.end(),
            slots_comparator{}
        );
        auto find_slots_idx {
            [&](const N& node) -> size_t {
                auto it { std::lower_bound(slots.begin(), slots.end(), node, slots_comparator{}) };
                return it - slots.begin();
            }
        };
        auto find_slots_ref {
            [&](const N& node) -> slot& {
                auto it { std::lower_bound(slots.begin(), slots.end(), node, slots_comparator{}) };
                return *it;
            }
        };
        // Create "ready_idxes" queue
        // --------------------------
        // The "ready_idxes" queue contains indicies within "slots" that are ready-to-process (node in that slot has had all
        // parents processed, and so it can be processed). Since root nodes have no parents, they are ready-to-process from
        // the get-go. As such, the "ready_idxes" queue is primed with the "slots" indices for root nodes.
        //
        // The helper functions provided below (as lambdas) are there to simplify the code.
        std::vector<size_t> ready_idxes {};
        for (const auto& node : g.get_root_nodes()) {
            set_node_data_func(node, 0.0, std::nullopt);  // set
            ready_idxes.push_back(
                find_slots_idx(node)
            );
        }
        auto has_ready_idx {
            [&]() {
                return !ready_idxes.empty();
            }
        };
        auto push_ready_idx {
            [&](size_t idx) {
                ready_idxes.push_back(idx);
            }
        };
        auto pop_ready_idx {
            [&]() {
                auto ret { ready_idxes.back() };
                ready_idxes.pop_back();
                return ret;
            }
        };
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
        while (has_ready_idx()) {
            size_t idx { pop_ready_idx() };
            auto& current_slot = slots[idx];
            for (const auto& edge : g.get_inputs(current_slot.node)) {
                const auto& src_node { g.get_edge_from(edge) };
                if (find_slots_ref(src_node).unwalked_parent_cnt != 0) {
                    goto top;
                }
            }
            auto incoming_accumulated {
                g.get_inputs(current_slot.node)
                | std::views::transform(
                    [&](const auto& edge) noexcept -> std::optional<std::pair<E, double>> {
                        const auto& src_node { g.get_edge_from(edge) };
                        const auto& [src_node_weight, _] { get_node_data_func(src_node) };
                        const auto& edge_weight { get_edge_weight_func(edge) };
                        if (src_node_weight != std::nullopt) {
                            return { { edge, *src_node_weight + edge_weight } };
                        } else {
                            return std::nullopt;
                        }
                    }
                )
                | std::views::filter([](const auto& item) noexcept { return item != std::nullopt; })
                | std::views::transform([](const auto& item) noexcept -> std::pair<E, double> { return *item; })
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
                std::optional<E> max_edge = { (*found).first };
                std::optional<double> max_weight = { (*found).second };
                set_node_data_func(current_slot.node, max_weight, max_edge);
            }
            // For outgoing nodes this node points to, decrement its number of unprocessed parents (this node was one of its
            // parents, and it was processed in this iteration of the loop) then add it to "ready_idxes" if it has no more
            // unprocessed parents.
            const auto outgoing_nodes {
                g.get_outputs(current_slot.node)
                | std::views::transform([&g](const auto e) { return g.get_edge_to(e); })
            };
            for (const auto& edge : g.get_outputs(current_slot.node)) {
                const auto & dst_node { g.get_edge_to(edge) };
                size_t dst_slot_idx { find_slots_idx(dst_node) } ;
                slot& dst_slot { slots[dst_slot_idx] };
                if constexpr (error_check) {
                    if (dst_slot.unwalked_parent_cnt == 0u) {
                        throw std::runtime_error("Invalid number of unprocessed parents");
                    }
                }
                dst_slot.unwalked_parent_cnt -= 1u;
                if (dst_slot.unwalked_parent_cnt == 0u) {
                    push_ready_idx(dst_slot_idx);
                }
            }
        }
    }

    template<typename G>
    std::vector<typename G::E> backtrack(
            G& g,
            const typename G::N& end_node,
            std::function<std::tuple<std::optional<double>, std::optional<typename  G::E>>(typename  G::N)> get_node_data_func
    ) {
        using N = typename G::N;
        using E = typename G::E;
        auto next_node { end_node };
        std::vector<E> reverse_path {};
        while (true) {
            auto node { next_node };
            const auto& [weight, backtracking_edge] { get_node_data_func(node) };
            if (backtracking_edge == std::nullopt) {
                break;
            }
            reverse_path.push_back(*backtracking_edge);
            next_node = g.get_edge_from(*backtracking_edge);
        }
        std::reverse(reverse_path.begin(), reverse_path.end());  // this is the path in reverse -- reverse it to get it in the correct order
        return reverse_path;
    }

    template<typename G>
    std::optional<std::tuple<std::vector<typename G::E>, double>> find_max_path(
            G& graph,
            const typename G::N& start_node,
            const typename G::N& end_node,
            std::function<void(typename G::N, std::optional<double>, std::optional<typename G::E>)> set_node_data_func,
            std::function<std::tuple<std::optional<double>, std::optional<typename G::E>>(typename G::N)> get_node_data_func,
            std::function<double(typename G::E)> get_edge_weight_func
    ) {
        using N = typename G::N;
        using E = typename G::E;
        populate_weights_and_backtrack_pointers(
            graph,
            start_node,
            set_node_data_func,
            get_node_data_func,
            get_edge_weight_func
        );
        const auto& path { backtrack(graph, end_node, get_node_data_func) };
        if (path.empty()) {
            return std::nullopt;
        }
        const auto& [weight, _] { get_node_data_func(end_node) };
        return { std::tuple<std::vector<E>, double>{path, *weight} };
    }
}

#endif //BACKTRACKER_2_H
