#ifndef OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_GLOBAL_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_GLOBAL_ALIGNMENT_GRAPH_H

#include <cstddef>
#include <ranges>
#include <tuple>
#include <stdexcept>
#include <utility>
#include <functional>
#include <type_traits>
#include <stdfloat>
#include "offbynull/aligner/graphs/grid_graph.h"
#include "offbynull/aligner/graph/grid_container_creator.h"
#include "offbynull/aligner/graph/grid_container_creators.h"
#include "offbynull/concepts.h"
#include "offbynull/aligner/concepts.h"

namespace offbynull::aligner::graphs::pairwise_global_alignment_graph {
    using offbynull::aligner::graphs::grid_graph::grid_graph;
    using offbynull::aligner::graph::grid_container_creator::grid_container_creator;
    using offbynull::aligner::graph::grid_container_creators::vector_grid_container_creator;
    using offbynull::aligner::concepts::weight;
    using offbynull::concepts::widenable_to_size_t;

    template<
        typename ND_,
        typename ED_,
        widenable_to_size_t INDEX_ = unsigned int,
        grid_container_creator<INDEX_> ND_ALLOCATOR_ = vector_grid_container_creator<ND_, INDEX_, false>,
        grid_container_creator<INDEX_> ED_ALLOCATOR_ = vector_grid_container_creator<ED_, INDEX_, false>,
        bool error_check = true
    >
    class pairwise_global_alignment_graph {
    public:
        using INDEX = INDEX_;
        using N = std::pair<INDEX, INDEX>;
        using E = std::pair<N, N>;
        using ED = ED_;
        using ND = ND_;

    private:
        grid_graph<ND, ED, INDEX, ND_ALLOCATOR_, ED_ALLOCATOR_, error_check> g;

    public:
        const INDEX grid_down_cnt;
        const INDEX grid_right_cnt;

        pairwise_global_alignment_graph(
            INDEX _grid_down_cnt,
            INDEX _grid_right_cnt,
            ED indel_data = {},
            ND_ALLOCATOR_ nd_container_creator = {},
            ED_ALLOCATOR_ ed_container_creator = {}
        )
        : g{_grid_down_cnt, _grid_right_cnt, indel_data, nd_container_creator, ed_container_creator}
        , grid_down_cnt{_grid_down_cnt}
        , grid_right_cnt{_grid_right_cnt} {}

        std::pair<INDEX, INDEX> node_to_grid_offsets(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.node_to_grid_offsets(node);
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
            return g.get_leaf_nodes();
        }

        auto get_leaf_node() {
            return g.get_leaf_node();
        }

        auto get_nodes() {
            return g.get_nodes();
        }

        auto get_edges() {
            return g.get_edges();
        }

        bool has_node(const N& node) {
            return g.has_node(node);
        }

        bool has_edge(const E& edge) {
            return g.has_edge(edge);
        }

        auto get_outputs_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_outputs_full(node);
        }

        auto get_inputs_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_inputs_full(node);
        }

        auto get_outputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_outputs(node);
        }

        auto get_inputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_inputs(node);
        }

        bool has_outputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.has_outputs(node);
        }

        bool has_inputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.has_inputs(node);
        }

        std::size_t get_out_degree(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_out_degree(node);
        }

        std::size_t get_in_degree(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_in_degree(node);
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
            using V_ELEM = std::decay_t<decltype(*v.begin())>;
            using W_ELEM = std::decay_t<decltype(*w.begin())>;
            if constexpr (error_check) {
                if (grid_down_cnt != v.size() + 1zu || grid_right_cnt != w.size() + 1zu) {
                    throw std::runtime_error("Mismatching node count");
                }
            }
            for (const auto& edge : get_edges()) {
                const auto& [n1, n2] { edge };
                const auto& [n1_grid_down, n1_grid_right] { n1 };
                const auto& [n2_grid_down, n2_grid_right] { n2 };
                std::optional<std::reference_wrapper<const V_ELEM>> v_elem { std::nullopt };
                if (n1_grid_down + 1u == n2_grid_down) {
                    v_elem = { v[n1_grid_down] };
                }
                std::optional<std::reference_wrapper<const W_ELEM>> w_elem { std::nullopt };
                if (n1_grid_right + 1u == n2_grid_right) {
                    w_elem = { w[n1_grid_right] };
                }
                WEIGHT weight { weight_lookup(v_elem, w_elem) };
                ED& ed { get_edge_data(edge) };
                weight_setter(ed, weight);
            }
        }

        auto edge_to_element_offsets(
            const E& edge
        ) {
            using OPT_INDEX = std::optional<INDEX>;
            using RET = std::optional<std::pair<OPT_INDEX, OPT_INDEX>>;

            const auto& [n1, n2] {edge};
            const auto& [n1_grid_down, n1_grid_right] {n1};
            const auto& [n2_grid_down, n2_grid_right] {n2};
            if (n1_grid_down + 1u == n2_grid_down && n1_grid_right + 1u == n2_grid_right) {
                return RET { { { n1_grid_down }, { n1_grid_right } } };
            } else if (n1_grid_down + 1u == n2_grid_down && n1_grid_right == n2_grid_right) {
                return RET { { { n1_grid_down }, std::nullopt } };
            } else if (n1_grid_down == n2_grid_down && n1_grid_right + 1u == n2_grid_right) {
                return RET { { std::nullopt, { n1_grid_right } } };
            }
            if constexpr (error_check) {
                throw std::runtime_error("Bad edge");
            }
            std::unreachable();
        }

        constexpr static INDEX node_count(
            INDEX _grid_down_cnt,
            INDEX _grid_right_cnt
        ) {
            return grid_graph<ND, ED, INDEX, ND_ALLOCATOR_, ED_ALLOCATOR_, error_check>::node_count(
                _grid_down_cnt, _grid_right_cnt
            );
        }

        constexpr static INDEX longest_path_edge_count(
            INDEX _grid_down_cnt,
            INDEX _grid_right_cnt
        ) {
            return grid_graph<ND, ED, INDEX, ND_ALLOCATOR_, ED_ALLOCATOR_, error_check>::longest_path_edge_count(
                _grid_down_cnt, _grid_right_cnt
            );
        }

        constexpr static std::size_t slice_nodes_capacity(INDEX _grid_down_cnt, INDEX _grid_right_cnt) {
            using G = std::decay_t<decltype(g)>;
            return G::slice_nodes_capacity(_grid_down_cnt, _grid_right_cnt);
        }

        auto slice_nodes(INDEX grid_down) {
            return g.slice_nodes(grid_down);
        }

        auto slice_nodes(INDEX grid_down, INDEX grid_right_cnt_) {
            return g.slice_nodes(grid_down, grid_right_cnt_);
        }

        N slice_first_node(INDEX grid_down) {
            return g.slice_first_node(grid_down);
        }

        N slice_first_node(INDEX grid_down, INDEX grid_right_cnt_) {
            return g.slice_first_node(grid_down, grid_right_cnt_);
        }

        N slice_last_node(INDEX grid_down) {
            return g.slice_last_node(grid_down);
        }

        N slice_last_node(INDEX grid_down, INDEX grid_right_cnt_) {
            return g.slice_last_node(grid_down, grid_right_cnt_);
        }

        N slice_next_node(const N& node) {
            return g.slice_next_node(node);
        }

        N slice_prev_node(const N& node) {
            return g.slice_prev_node(node);
        }

        constexpr static std::size_t resident_nodes_capacity(INDEX _grid_down_cnt, INDEX _grid_right_cnt) {
            using G = std::decay_t<decltype(g)>;
            return G::resident_nodes_capacity(_grid_down_cnt, _grid_right_cnt);
        }

        auto resident_nodes() {
            return g.resident_nodes();
        }

        auto outputs_to_residents(const N& node) {
            return g.outputs_to_residents(node);
        }

        auto inputs_from_residents(const N& node) {
            return g.inputs_from_residents(node);
        }
    };
}
#endif //OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_GLOBAL_ALIGNMENT_GRAPH_H
