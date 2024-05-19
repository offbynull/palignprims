#ifndef BACKTRACKER_2_H
#define BACKTRACKER_2_H

#include <tuple>
#include <optional>
#include <vector>
#include <functional>
#include <ranges>
#include <algorithm>
#include <queue>

CONTINUE TO FIX ME UP;
CONTINUE TO FIX ME UP;
CONTINUE TO FIX ME UP;
CONTINUE TO FIX ME UP;
CONTINUE TO FIX ME UP;
CONTINUE TO FIX ME UP;
CONTINUE TO FIX ME UP;
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
    // Create sorted tuple of (node, parents remaining)
    struct block {
        N node;
        size_t parent_cnt;
        bool processed;
    };
    const auto& blocks_lazy {
        g.get_nodes()
        | std::views::transform([&](const auto& n) noexcept -> block { return { n, g.get_in_degree(n), false }; })
    };
    std::vector<block> blocks(blocks_lazy.begin(), blocks_lazy.end());
    struct blocks_comparator {
        bool operator()(const block& lhs, const block& rhs) const {
            return lhs.node < rhs.node;
        }

        bool operator()(const block& lhs, const N& rhs) const {
            return lhs.node < rhs;
        }

        bool operator()(const N& lhs, const block& rhs) const {
            return lhs < rhs.node;
        }
    };
    std::ranges::sort(
        blocks.begin(),
        blocks.end(),
        blocks_comparator{}
    );
    auto find_blocks_idx {
        [&](const N& node) -> size_t {
            auto it { std::lower_bound(blocks.begin(), blocks.end(), node, blocks_comparator{}) };
            return it - blocks.begin();
        }
    };
    auto find_blocks_ref {
        [&](const N& node) -> block& {
            auto it { std::lower_bound(blocks.begin(), blocks.end(), node, blocks_comparator{}) };
            return *it;
        }
    };
    // For all root nodes, add to processed_nodes and set None weight and None backtracking edge.
    // For all root nodes, add any children where its the only parent to waiting_nodes.
    std::priority_queue<size_t> ready_idxes {};
    for (const auto& node : g.get_root_nodes()) {
        // Set backtracking and weight to nothing
        set_node_data_func(node, 0.0, std::nullopt);
        // Add children to ready
        ready_idxes.push(
            find_blocks_idx(node)
        );
    }
    auto pop_ready_idxes {
        [&]() {
            auto ret { ready_idxes.top() };
            ready_idxes.pop();
            return ret;
        }
    };
    // Any nodes in waiting_nodes have had all their parents already processed (in processed_nodes). As such, they can
    // have their weights and backtracking pointers calculated. They can then be placed into processed_nodes themselves.
    top:
    while (!ready_idxes.empty()) {
        size_t idx { pop_ready_idxes() };
        auto& current_block = blocks[idx];
        for (const auto& edge : g.get_inputs(current_block.node)) {
            const auto& src_node { g.get_edge_from(edge) };
            if (find_blocks_ref(src_node).parent_cnt != 0) {
                goto top;
            }
        }
        auto incoming_accumulated {
            g.get_inputs(current_block.node)
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
        std::optional<E> max_edge = std::nullopt;
        std::optional<double> max_weight = std::nullopt;
        if (found != incoming_accumulated.end()) {
            max_edge = { (*found).first };
            max_weight = { (*found).second };
        }
        set_node_data_func(current_block.node, max_weight, max_edge);
        // This node has been processed, move it over to processed_nodes.
        current_block.processed = true;
        // For outgoing nodes this node points to, if that outgoing node has all of its dependencies in processed_nodes,
        // then add it to waiting_nodes (so it can be processed).
        const auto outgoing_nodes {
            g.get_outputs(current_block.node)
            | std::views::transform([&g](const auto e) { return g.get_edge_to(e); })
        };
        for (const auto& edge : g.get_outputs(current_block.node)) {
            const auto & dst_node { g.get_edge_to(edge) };
            size_t dst_block_idx { find_blocks_idx(dst_node) } ;
            block& dst_block { blocks[dst_block_idx] };
            if constexpr (error_check) {
                if (dst_block.parent_cnt == 0u) {
                    throw std::runtime_error("Invalid number of parents");
                }
            }
            dst_block.parent_cnt -= 1u;
            if (dst_block.parent_cnt == 0u) {
                ready_idxes.push(dst_block_idx);
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
    auto next_node { end_node };
    std::vector<typename G::E> reverse_path {};
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
    return std::tuple<std::vector<typename G::E>, double>{path, *weight};
}

#endif //BACKTRACKER_2_H
