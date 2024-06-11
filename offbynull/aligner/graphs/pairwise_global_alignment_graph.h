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
        const INDEX down_node_cnt;
        const INDEX right_node_cnt;
        static constexpr std::size_t max_in_degree { 3zu };

        pairwise_global_alignment_graph(
            INDEX _down_node_cnt,
            INDEX _right_node_cnt,
            ED indel_data = {},
            ND_ALLOCATOR_ nd_container_creator = {},
            ED_ALLOCATOR_ ed_container_creator = {}
        )
        : g{_down_node_cnt, _right_node_cnt, indel_data, nd_container_creator, ed_container_creator}
        , down_node_cnt{_down_node_cnt}
        , right_node_cnt{_right_node_cnt} {}

        void update_node_data(const N& node, ND&& data) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            g.update_node_data(node, std::forward<ND>(data));
        }

        ND& get_node_data(const N& node) {
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
            return std::get<2>(get_edge(edge));
        }

        N get_edge_from(const E& edge) {
            return std::get<0>(get_edge(edge));
        }

        N get_edge_to(const E& edge) {
            return std::get<1>(get_edge(edge));
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

        std::tuple<E, N, N, ED&> get_output_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_output_full(node);
        }

        auto get_inputs_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_inputs_full(node);
        }

        std::tuple<E, N, N, ED&> get_input_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_input_full(node);
        }

        auto get_outputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_outputs(node);
        }

        E get_output(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_output();
        }

        auto get_inputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_inputs(node);
        }

        E get_input(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.get_input(node);
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
                if (down_node_cnt != v.size() + 1zu || right_node_cnt != w.size() + 1zu) {
                    throw std::runtime_error("Mismatching node count");
                }
            }
            for (const auto& edge : get_edges()) {
                const auto& [n1, n2] { edge };
                const auto& [n1_down, n1_right] { n1 };
                const auto& [n2_down, n2_right] { n2 };
                std::optional<std::reference_wrapper<const V_ELEM>> v_elem { std::nullopt };
                if (n1_down + 1u == n2_down) {
                    v_elem = { v[n1_down] };
                }
                std::optional<std::reference_wrapper<const W_ELEM>> w_elem { std::nullopt };
                if (n1_right + 1u == n2_right) {
                    w_elem = { w[n1_right] };
                }
                WEIGHT weight { weight_lookup(v_elem, w_elem) };
                ED& ed { get_edge_data(edge) };
                weight_setter(ed, weight);
            }
        }

        static auto edge_to_elements(
            const E& edge,
            const auto& v,  // random access container
            const auto& w   // random access container
        ) {
            using V_ELEM = std::remove_reference_t<decltype(v[0])>;
            using W_ELEM = std::remove_reference_t<decltype(w[0])>;
            using OPT_V_ELEM_REF = std::optional<std::reference_wrapper<const V_ELEM>>;
            using OPT_W_ELEM_REF = std::optional<std::reference_wrapper<const W_ELEM>>;
            using RET = std::optional<std::pair<OPT_V_ELEM_REF, OPT_W_ELEM_REF>>;

            const auto& [n1, n2] {edge};
            const auto& [n1_down, n1_right] {n1};
            const auto& [n2_down, n2_right] {n2};
            if (n1_down + 1u == n2_down && n1_right + 1u == n2_right) {
                if constexpr (error_check) {
                    if (n1_down >= v.size() or n1_right >= w.size()) {
                        throw std::runtime_error("Out of bounds");
                    }
                }
                return RET { { { v[n1_down] },  { w[n1_right] } } };
            } else if (n1_down + 1u == n2_down && n1_right == n2_right) {
                if constexpr (error_check) {
                    if (n1_down >= v.size()) {
                        throw std::runtime_error("Out of bounds");
                    }
                }
                return RET { { { v[n1_down] }, std::nullopt } };
            } else if (n1_down == n2_down && n1_right + 1u == n2_right) {
                if constexpr (error_check) {
                    if (n1_right >= w.size()) {
                        throw std::runtime_error("Out of bounds");
                    }
                }
                return RET { { std::nullopt, { w[n1_right] } } };
            }
            if constexpr (error_check) {
                throw std::runtime_error("Bad edge");
            }
            std::unreachable();
        }

        constexpr static INDEX node_count(
            INDEX _down_node_cnt,
            INDEX _right_node_cnt
        ) {
            return grid_graph<ND, ED, INDEX, ND_ALLOCATOR_, ED_ALLOCATOR_, error_check>::node_count(
                _down_node_cnt, _right_node_cnt
            );
        }

        constexpr static INDEX edge_count(
            INDEX _down_node_cnt,
            INDEX _right_node_cnt
        ) {
            return grid_graph<ND, ED, INDEX, ND_ALLOCATOR_, ED_ALLOCATOR_, error_check>::edge_count(
                _down_node_cnt, _right_node_cnt
            );
        }

        constexpr static INDEX longest_path_edge_count(
            INDEX _down_node_cnt,
            INDEX _right_node_cnt
        ) {
            return grid_graph<ND, ED, INDEX, ND_ALLOCATOR_, ED_ALLOCATOR_, error_check>::longest_path_edge_count(
                _down_node_cnt, _right_node_cnt
            );
        }

        std::size_t max_slice_nodes_count() {
            return g.max_slice_nodes_count();
        }

        auto slice_nodes(INDEX n_down) {
            return g.slice_nodes(n_down);
        }

        N first_node_in_slice(INDEX n_down) {
            return g.first_node_in_slice(n_down);
        }

        N last_node_in_slice(INDEX n_down) {
            return g.last_node_in_slice(n_down);
        }

        N next_node_in_slice(const N& node) {
            return g.next_node_in_slice(node);
        }

        N prev_node_in_slice(const N& node) {
            return g.prev_node_in_slice(node);
        }

        std::size_t max_resident_nodes_count() {
            return g.max_resident_nodes_count();
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
