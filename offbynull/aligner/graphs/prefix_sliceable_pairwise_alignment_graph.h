#ifndef OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_PREFIX_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_PREFIX_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H

#include <functional>
#include <type_traits>
#include <stdfloat>
#include <ranges>
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/concepts.h"

namespace offbynull::aligner::graphs::prefix_sliceable_pairwise_alignment_graph {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_parwise_alignment_graph;
    using offbynull::aligner::concepts::weight;

    template<
        readable_sliceable_parwise_alignment_graph GRAPH,
        bool error_check=true
    >
    class prefix_sliceable_pairwise_alignment_graph {
    public:
        using INDEX = typename GRAPH::INDEX;
        using N = typename GRAPH::N;
        using E = typename GRAPH::E;
        using ED = typename GRAPH::ED;
        using ND = typename GRAPH::ND;

    private:
        GRAPH& g;

        bool node_out_of_bound(const N& node) {
            const auto& [down_offset, right_offset] { g.node_to_grid_offsets(node) };
            return down_offset >= down_node_cnt || (right_offset >= right_node_cnt);
        }

        bool edge_out_of_bound(const E& edge) {
            return node_out_of_bound(g.get_edge_from(edge)) || node_out_of_bound(g.get_edge_to(edge));
        }

    public:
        const INDEX down_node_cnt;
        const INDEX right_node_cnt;

        prefix_sliceable_pairwise_alignment_graph(
            GRAPH& _g,
            INDEX _down_node_cnt,
            INDEX _right_node_cnt
        )
        : g{_g}
        , down_node_cnt{_down_node_cnt}
        , right_node_cnt{_right_node_cnt} {
            if constexpr (error_check) {
                if (down_node_cnt > g.down_node_cnt || right_node_cnt > g.right_node_cnt) {
                    throw std::runtime_error("Out of bounds");
                }
            }
        }

        void update_node_data(const N& node, ND&& data) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            g.update_node_data(node, std::forward<ND>(data));
        }

        ND& get_node_data(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_node_data(node);
        }

        void update_edge_data(const E& edge, ED&& data) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            g.update_edge_data(edge, std::forward<ED>(data));
        }

        ED& get_edge_data(const E& edge) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            return g.get_edge_data(edge);
        }

        N get_edge_from(const E& edge) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            return g.get_edge_from(edge);
        }

        N get_edge_to(const E& edge) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            return g.get_edge_to(edge);
        }

        std::tuple<N, N, ED&> get_edge(const E& edge) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            return g.get_edge(edge);
        }

        auto get_root_nodes() {
            return g.get_root_nodes();
        }

        N get_root_node() {
            return g.get_root_node();
        }

        auto get_leaf_nodes() {
            return std::views::single(get_leaf_node());
        }

        auto get_leaf_node() {
            return g.last_node_in_slice(down_node_cnt - 1u, right_node_cnt - 1u);
        }

        auto get_nodes() {
            return g.get_nodes()
                | std::views::filter([&](const N& node) { return !node_out_of_bound(node); });
        }

        auto get_edges() {
            return g.get_edges()
                | std::views::filter([&](const E& edge) { return !edge_out_of_bound(edge); });
        }

        bool has_node(const N& node) {
            return g.has_node(node) && !node_out_of_bound(node);
        }

        bool has_edge(const E& edge) {
            return g.has_edge(edge) && !edge_out_of_bound(edge);
        }

        auto get_outputs_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_outputs_full(node)
                | std::views::filter([&](const auto& vals) { return !edge_out_of_bound(std::get<0>(vals)); });
        }

        auto get_inputs_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_inputs_full(node)
                | std::views::filter([&](const auto& vals) { return !edge_out_of_bound(std::get<0>(vals)); });
        }

        auto get_outputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_outputs(node)
                | std::views::filter([&](const auto& vals) { return !edge_out_of_bound(vals); });
        }

        auto get_inputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_inputs(node)
                | std::views::filter([&](const auto& vals) { return !edge_out_of_bound(vals); });
        }

        bool has_outputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return get_out_degree(node) > 0zu;
        }

        bool has_inputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return get_in_degree(node) > 0zu;
        }

        std::size_t get_out_degree(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto outputs { std::views::common(get_outputs(node)) };
            auto dist { std::distance(outputs.begin(), outputs.end()) };
            return static_cast<std::size_t>(dist);
        }

        std::size_t get_in_degree(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto inputs { std::views::common(get_inputs(node)) };
            auto dist { std::distance(inputs.begin(), inputs.end()) };
            return static_cast<std::size_t>(dist);
        }

        auto edge_to_element_offsets(
            const E& edge
        ) {
            return g.edge_to_element_offsets(edge);
        }

        std::pair<INDEX, INDEX> node_to_grid_offsets(const N& node) {
            return g.node_to_grid_offsets(node);
        }

        constexpr static INDEX node_count(
            INDEX _down_node_cnt,
            INDEX _right_node_cnt
        ) {
            return GRAPH::node_count(_down_node_cnt, _right_node_cnt);
        }

        constexpr static INDEX longest_path_edge_count(
            INDEX _down_node_cnt,
            INDEX _right_node_cnt
        ) {
            return GRAPH::longest_path_edge_count(_down_node_cnt, _right_node_cnt);
        }

        constexpr static std::size_t slice_nodes_capacity(INDEX _down_node_cnt, INDEX _right_node_cnt) {
            return GRAPH::slice_nodes_capacity(_down_node_cnt, _right_node_cnt);
        }

        auto slice_nodes(INDEX n_down) {
            return g.slice_nodes(n_down)
                | std::views::take(right_node_cnt);
        }

        auto slice_nodes(INDEX n_down, INDEX override_right_node_cnt) {
            return slice_nodes(n_down)
                | std::views::take(override_right_node_cnt);
        }

        N first_node_in_slice(INDEX n_down) {
            return g.first_node_in_slice(n_down, right_node_cnt);
        }

        N first_node_in_slice(INDEX n_down, INDEX override_right_node_cnt) {
            return g.first_node_in_slice(n_down, override_right_node_cnt);
        }

        N last_node_in_slice(INDEX n_down) {
            return g.last_node_in_slice(n_down, right_node_cnt);
        }

        N last_node_in_slice(INDEX n_down, INDEX override_right_node_cnt) {
            return g.last_node_in_slice(n_down, override_right_node_cnt);
        }

        N next_node_in_slice(const N& node) {
            N next_node { g.next_node_in_slice(node) };
            if constexpr (error_check) {
                const auto& [n_down, n_right] { node_to_grid_offsets(node) };
                if (n_down >= down_node_cnt) {
                    throw std::runtime_error("Node too far down");
                }
                if (n_right >= right_node_cnt) {
                    throw std::runtime_error("Node too far right");
                }
            }
            return next_node;
        }

        N prev_node_in_slice(const N& node) {
            N prev_node { g.prev_node_in_slice(node) };
            if constexpr (error_check) {
                const auto& [n_down, n_right] { node_to_grid_offsets(node) };
                if (n_down >= down_node_cnt) {
                    throw std::runtime_error("Node too far down");
                }
                if (n_right >= right_node_cnt) {
                    throw std::runtime_error("Node too far right");
                }
            }
            return prev_node;
        }

        constexpr static std::size_t resident_nodes_capacity(INDEX _down_node_cnt, INDEX _right_node_cnt) {
            return GRAPH::resident_nodes_capacity(_down_node_cnt, _right_node_cnt);
        }

        auto resident_nodes() {
            return g.resident_nodes()
                | std::views::filter([&](const N& node) { return !node_out_of_bound(node); });
        }

        auto outputs_to_residents(const N& node) {
            return g.outputs_to_residents(node)
                | std::views::filter([&](const E& edge) { return !edge_out_of_bound(edge); });
        }

        auto inputs_from_residents(const N& node) {
            return g.inputs_from_residents(node)
                | std::views::filter([&](const E& edge) { return !edge_out_of_bound(edge); });
        }
    };
}
#endif //OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_PREFIX_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
