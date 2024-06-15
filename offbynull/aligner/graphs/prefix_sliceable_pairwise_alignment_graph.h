#ifndef OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_SUB_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_SUB_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H

#include <functional>
#include <type_traits>
#include <stdfloat>
#include <ranges>
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/concepts.h"

namespace offbynull::aligner::graphs::sub_sliceable_pairwise_alignment_graph {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_parwise_alignment_graph;
    using offbynull::aligner::concepts::weight;

    template<
        readable_sliceable_parwise_alignment_graph GRAPH,
        bool error_check
    >
    class sub_sliceable_pairwise_alignment_graph {
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
            return down_offset >= down_node_cnt || (right_offset >= right_node_cnt;
        }

        bool edge_out_of_bound(const E& edge) {
            return node_out_of_bound(g.get_edge_from(edge)) || node_out_of_bound(g.get_edge_to(edge));
        }

    public:
        const INDEX down_node_cnt;
        const INDEX right_node_cnt;
        const N leaf_node;

        sub_sliceable_pairwise_alignment_graph(
            GRAPH& _g,
            INDEX _down_node_cnt,
            INDEX _right_node_cnt,
            N _leaf_node
        )
        : g{_g}
        , down_node_cnt{_down_node_cnt}
        , right_node_cnt{_right_node_cnt}
        , leaf_node{_leaf_node} {
            if constexpr (error_check) {
                if (down_node_cnt > g.down_node_cnt || right_node_cnt > g.right_node_cnt) {
                    throw std::runtime_error("Out of bounds");
                }
                if (g.node_to_grid_offsets(leaf_node) != std::pair<INDEX, INDEX> { down_node_cnt - 1u, right_node_cnt - 1u}) {
                    throw std::runtime_error {"Leaf node bad index"};
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
            return std::views::single(leaf_node);
        }

        auto get_leaf_node() {
            return leaf_node;
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
            return node_out_of_bound(node) && g.has_node(node);
        }

        bool has_edge(const E& edge) {
            return edge_out_of_bound(edge) && g.has_edge(edge);
        }

        auto get_outputs_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_outputs_full(node)
                | std::views::filter([&](const auto& vals) { return !edge_out_of_bound(std::get<3>(vals)); });
        }

        auto get_inputs_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_inputs_full(node)
                | std::views::filter([&](const auto& vals) { return !edge_out_of_bound(std::get<3>(vals)); });
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
            auto outputs { get_outputs(node) };
            auto dist { std::distance(outputs.begin(), outputs.end()) };
            return static_cast<std::size_t>(dist);
        }

        std::size_t get_in_degree(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto inputs { get_inputs(node) };
            auto dist { std::distance(inputs.begin(), inputs.end()) };
            return static_cast<std::size_t>(dist);
        }

        template<weight WEIGHT=std::float64_t>
        void assign_weights(
            const std::ranges::random_access_range auto& v,  // random access container
            const std::ranges::random_access_range auto& w,  // random access container
            std::function<
                WEIGHT(
                    const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(v[0u])>>>&,
                    const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(w[0u])>>>&
                )
            > weight_lookup,
            std::function<void(ED&, WEIGHT weight)> weight_setter
        ) {
            // TODO: Make a concept that doesn't contain this func and use that instead for the code that requires this class (sliced walking)
            throw std::runtime_error("Unsupported");
        }

        auto edge_to_element_offsets(
            const E& edge
        ) {
            return g.edge_to_element_offsets(edge);
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

        static std::size_t slice_nodes_capacity(INDEX _down_node_cnt, INDEX _right_node_cnt) {
            return GRAPH::slice_nodes_capacity(_down_node_cnt, _right_node_cnt);
        }

        auto slice_nodes(INDEX n_down) {
            return g.slice_nodes(n_down)
                | std::views::filter([&](const N& node) { return !node_out_of_bound(node); });
        }

        N first_node_in_slice(INDEX n_down) {
            return g.first_node_in_slice(n_down);
        }

        N last_node_in_slice(INDEX n_down) {
            return g.last_node_in_slice(n_down, right_node_cnt - 1u);  // TODO: Create this override, if multiple nodes are asosciated with, this grabs the "last" one
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

        static std::size_t resident_nodes_capacity(INDEX _down_node_cnt, INDEX _right_node_cnt) {
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
#endif //OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_SUB_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_H
