#ifndef OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_PREFIX_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_PREFIX_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H

#include <ranges>
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/concepts.h"

namespace offbynull::aligner::graphs::prefix_sliceable_pairwise_alignment_graph {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::concepts::weight;

    template<
        readable_sliceable_pairwise_alignment_graph G,
        bool error_check=true
    >
    class prefix_sliceable_pairwise_alignment_graph {
    public:
        using INDEX = typename G::INDEX;
        using N = typename G::N;
        using E = typename G::E;
        using ED = typename G::ED;
        using ND = typename G::ND;

    private:
        const G& g;
        const N new_leaf_node;

        bool is_leaf_node_reachable(const N& src_node) const {
            if (src_node == new_leaf_node) {
                return true;
            }
            const auto& [leaf_down_offset, leaf_right_offset, leaf_grid_offset] { g.node_to_grid_offsets(new_leaf_node) };
            for (const E& edge : g.get_outputs(src_node)) {
                const N& dst_node { g.get_edge_to(edge) };
                const auto& [dst_down_offset, dst_right_offset, dst_grid_offset] { g.node_to_grid_offsets(dst_node) };
                if (dst_down_offset > leaf_down_offset || dst_right_offset > leaf_right_offset) {
                    return false;
                }
                return is_leaf_node_reachable(dst_node);
            }
            return false;
        }

        bool node_out_of_bound(const N& node) const {
            const auto& [down_offset, right_offset, _] { g.node_to_grid_offsets(node) };
            if (down_offset >= grid_down_cnt || right_offset >= grid_right_cnt) {
                return true;
            }
            // If you're testing a node that's within the same grid offset as the leaf node, test to make sure that it
            // actually reaches the leaf node. If it doesn't reach the leaf node (walks into a grid offset that's
            // further down/right), that means it is out of bounds.
            if (down_offset == grid_down_cnt - 1u && right_offset == grid_right_cnt - 1u) {
                return !is_leaf_node_reachable(node);
            }
            return false;
        }

        bool edge_out_of_bound(const E& edge) const {
            return node_out_of_bound(g.get_edge_from(edge)) || node_out_of_bound(g.get_edge_to(edge));
        }

    public:
        const INDEX grid_down_cnt;
        const INDEX grid_right_cnt;

        prefix_sliceable_pairwise_alignment_graph(
            const G& g_,
            const N& new_leaf_node_
        )
        : g{g_}
        , new_leaf_node{ new_leaf_node_ }
        , grid_down_cnt{ std::get<0>(g.node_to_grid_offsets(new_leaf_node)) + 1u }
        , grid_right_cnt{ std::get<1>(g.node_to_grid_offsets(new_leaf_node)) + 1u } {
            if constexpr (error_check) {
                if (!g.has_node(new_leaf_node)) {
                    throw std::runtime_error("Leaf node not found");
                }
            }
        }

        // The first implementation of this was proxing the exact type. For example...
        //
        // using get_edge_to_ret_type = decltype(g.get_edge_to(std::declval<E>()));
        // get_edge_to_ret_type get_edge_to(const E &edge) {
        //     return g.get_edge_to(...);
        // }
        //
        // Decided not to do this. No pairwise graph type uses ND&/ED&/N&/E&, so just go ahead and return concrete
        // object.

        ND get_node_data(const N& node) const {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_node_data(node);
        }

        ED get_edge_data(const E& edge) const {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            return g.get_edge_data(edge);
        }

        N get_edge_from(const E& edge) const {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            return g.get_edge_from(edge);
        }

        N get_edge_to(const E& edge) const {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            return g.get_edge_to(edge);
        }

        std::tuple<N, N, ED> get_edge(const E& edge) const {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            return g.get_edge(edge);
        }

        auto get_root_nodes() const {
            return g.get_root_nodes();
        }

        N get_root_node() const {
            return g.get_root_node();
        }

        auto get_leaf_nodes() const {
            return std::views::single(get_leaf_node());
        }

        N get_leaf_node() const {
            return new_leaf_node;
        }

        auto get_nodes() const {
            return g.get_nodes()
                | std::views::filter([&](const N& node) { return !node_out_of_bound(node); });
        }

        auto get_edges() const {
            return g.get_edges()
                | std::views::filter([&](const E& edge) { return !edge_out_of_bound(edge); });
        }

        bool has_node(const N& node) const {
            return g.has_node(node) && !node_out_of_bound(node);
        }

        bool has_edge(const E& edge) const {
            return g.has_edge(edge) && !edge_out_of_bound(edge);
        }

        auto get_outputs_full(const N& node) const {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_outputs_full(node)
                | std::views::filter([&](const auto& vals) { return !edge_out_of_bound(std::get<0>(vals)); });
        }

        auto get_inputs_full(const N& node) const {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_inputs_full(node)
                | std::views::filter([&](const auto& vals) { return !edge_out_of_bound(std::get<0>(vals)); });
        }

        auto get_outputs(const N& node) const {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_outputs(node)
                | std::views::filter([&](const auto& vals) { return !edge_out_of_bound(vals); });
        }

        auto get_inputs(const N& node) const {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_inputs(node)
                | std::views::filter([&](const auto& vals) { return !edge_out_of_bound(vals); });
        }

        bool has_outputs(const N& node) const {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return get_out_degree(node) > 0zu;
        }

        bool has_inputs(const N& node) const {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return get_in_degree(node) > 0zu;
        }

        std::size_t get_out_degree(const N& node) const {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto outputs { std::views::common(get_outputs(node)) };
            auto dist { std::ranges::distance(outputs) };
            return static_cast<std::size_t>(dist);
        }

        std::size_t get_in_degree(const N& node) const {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto inputs { std::views::common(get_inputs(node)) };
            auto dist { std::ranges::distance(inputs) };
            return static_cast<std::size_t>(dist);
        }

        std::optional<std::pair<std::optional<INDEX>, std::optional<INDEX>>> edge_to_element_offsets(
            const E& edge
        ) const {
            return g.edge_to_element_offsets(edge);
        }

        std::tuple<INDEX, INDEX, std::size_t> node_to_grid_offsets(const N& node) const {
            return g.node_to_grid_offsets(node);
        }

        constexpr static auto limits(
            INDEX _grid_down_cnt,
            INDEX _grid_right_cnt
        ) {
            return G::limits(_grid_down_cnt, _grid_right_cnt);;
        }

        auto slice_nodes(INDEX grid_down) const {
            return g.slice_nodes(grid_down)
                | std::views::take(grid_right_cnt);
        }

        auto slice_nodes(INDEX grid_down, INDEX override_grid_right_cnt) const {
            return slice_nodes(grid_down)
                | std::views::take(override_grid_right_cnt);
        }

        N slice_first_node(INDEX grid_down) const {
            return g.slice_first_node(grid_down);
        }

        N slice_first_node(INDEX grid_down, INDEX grid_right) const {
            return g.slice_first_node(grid_down, grid_right);
        }

        N slice_last_node(INDEX grid_down) const {
            return g.slice_last_node(grid_down, grid_right_cnt - 1u);
        }

        N slice_last_node(INDEX grid_down, INDEX grid_right) const {
            return g.slice_last_node(grid_down, grid_right);
        }

        N slice_next_node(const N& node) const {
            N next_node { g.slice_next_node(node) };
            if constexpr (error_check) {
                const auto& [grid_down, grid_right, _] { node_to_grid_offsets(node) };
                if (grid_down >= grid_down_cnt) {
                    throw std::runtime_error("Node too far down");
                }
                if (grid_right >= grid_right_cnt) {
                    throw std::runtime_error("Node too far right");
                }
            }
            return next_node;
        }

        N slice_prev_node(const N& node) const {
            N prev_node { g.slice_prev_node(node) };
            if constexpr (error_check) {
                const auto& [grid_down, grid_right, _] { node_to_grid_offsets(node) };
                if (grid_down >= grid_down_cnt) {
                    throw std::runtime_error("Node too far down");
                }
                if (grid_right >= grid_right_cnt) {
                    throw std::runtime_error("Node too far right");
                }
            }
            return prev_node;
        }

        auto resident_nodes() const {
            return g.resident_nodes()
                | std::views::filter([&](const N& node) { return !node_out_of_bound(node); });
        }

        auto outputs_to_residents(const N& node) const {
            return g.outputs_to_residents(node)
                | std::views::filter([&](const E& edge) { return !edge_out_of_bound(edge); });
        }

        auto inputs_from_residents(const N& node) const {
            return g.inputs_from_residents(node)
                | std::views::filter([&](const E& edge) { return !edge_out_of_bound(edge); });
        }
    };
}
#endif //OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_PREFIX_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
