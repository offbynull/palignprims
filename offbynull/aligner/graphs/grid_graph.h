#ifndef OFFBYNULL_ALIGNER_GRAPHS_GRID_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_GRID_GRAPH_H

#include <cstddef>
#include <ranges>
#include <tuple>
#include <stdexcept>
#include <utility>
#include "boost/container/static_vector.hpp"
#include "boost/container/options.hpp"
#include "offbynull/concepts.h"
#include "offbynull/utils.h"
#include "offbynull/aligner/graph/grid_container_creator.h"
#include "offbynull/aligner/graph/grid_container_creators.h"

namespace offbynull::aligner::graphs::grid_graph {
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::aligner::graph::grid_container_creator::grid_container_creator;
    using offbynull::aligner::graph::grid_container_creators::vector_grid_container_creator;
    using offbynull::utils::static_vector_typer;

    template<
        typename ND_,
        typename ED_,
        widenable_to_size_t INDEX_ = unsigned int,
        grid_container_creator<INDEX_> ND_ALLOCATOR_ = vector_grid_container_creator<ND_, INDEX_, false>,
        grid_container_creator<INDEX_> ED_ALLOCATOR_ = vector_grid_container_creator<ED_, INDEX_, false>,
        bool error_check = true
    >
    class grid_graph {
    public:
        using INDEX = INDEX_;
        using N = std::pair<INDEX, INDEX>;
        using ND = ND_;
        using E = std::pair<N, N>;
        using ED = ED_;

        const INDEX down_node_cnt;
        const INDEX right_node_cnt;

    private:
        decltype(std::declval<ND_ALLOCATOR_>().create_objects(std::declval<INDEX>(), std::declval<INDEX>())) nodes;
        decltype(std::declval<ED_ALLOCATOR_>().create_objects(std::declval<INDEX>(), std::declval<INDEX>())) edges;

        ED_ indel_ed;

        auto construct_full_edge(N n1, N n2) {
            return std::tuple<E, N, N, ED*> {
                E { n1, n2 },
                n1,
                n2,
                &this->get_edge_data(
                    E { n1, n2 }
                )
            };
        }

        std::size_t to_raw_idx(INDEX down_idx, INDEX right_idx) {
            std::size_t down_idx_widened { down_idx };
            std::size_t right_idx_widened { right_idx };
            return (down_idx_widened * right_node_cnt) + right_idx_widened;
        }

    public:
        grid_graph(
            INDEX _down_node_cnt,
            INDEX _right_node_cnt,
            ED indel_data = {},
            ND_ALLOCATOR_ nd_container_creator = {},
            ED_ALLOCATOR_ ed_container_creator = {}
        )
        : down_node_cnt{_down_node_cnt}
        , right_node_cnt{_right_node_cnt}
        , nodes{nd_container_creator.create_objects(_down_node_cnt, _right_node_cnt)}
        , edges{ed_container_creator.create_objects(_down_node_cnt, _right_node_cnt)}
        , indel_ed{indel_data} {}

        void update_node_data(const N& node, ND&& data) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto [n_down, n_right] = node;
            this->nodes[to_raw_idx(n_down, n_right)] = std::forward<ND>(data);
        }

        ND& get_node_data(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto [n_down, n_right] = node;
            return this->nodes[to_raw_idx(n_down, n_right)];
        }

        void update_edge_data(const E& edge, ED&& data) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            auto [n1_down, n1_right] = edge.first;
            auto [n2_down, n2_right] = edge.second;
            if (n1_down == n2_down && n1_right + 1u == n2_right) {
                indel_ed = std::forward<ED>(data);
            } else if (n1_down + 1u == n2_down && n1_right == n2_right) {
                indel_ed = std::forward<ED>(data);
            } else if (n1_down + 1u == n2_down && n1_right + 1u == n2_right) {
                this->edges[to_raw_idx(n1_down, n1_right)] = std::forward<ED>(data);
            }
        }

        ED& get_edge_data(const E& edge) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error("Edge doesn't exist");
                }
            }
            auto [n1_down, n1_right] = edge.first;
            auto [n2_down, n2_right] = edge.second;
            if (n1_down == n2_down && n1_right + 1u == n2_right) {
                return indel_ed;
            } else if (n1_down + 1u == n2_down && n1_right == n2_right) {
                return indel_ed;
            } else if (n1_down + 1u == n2_down && n1_right + 1u == n2_right) {
                return this->edges[to_raw_idx(n1_down, n1_right)];
            }
            if constexpr (error_check) {
                throw std::runtime_error("Bad edge");
            }
            std::unreachable();
        }

        N get_edge_from(const E& edge) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            return edge.first;
        }

        N get_edge_to(const E& edge) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            return edge.second;
        }

        std::tuple<N, N, ED&> get_edge(const E& edge) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            return std::tuple<N, N, ED&> {this->get_edge_from(edge), this->get_edge_to(edge), this->get_edge_data(edge)};
        }

        auto get_root_nodes() {
            return std::ranges::single_view { N { 0u, 0u } };
        }

        N get_root_node() {
            return N { 0u, 0u };
        }

        auto get_leaf_nodes() {
            return std::ranges::single_view { N { down_node_cnt - 1u, right_node_cnt - 1u } };
        }

        auto get_leaf_node() {
            return N { down_node_cnt - 1u, right_node_cnt - 1u };
        }

        auto get_nodes() {
            auto down_range { std::views::iota(0u, down_node_cnt) };
            auto right_range { std::views::iota(0u, right_node_cnt) };
            return
                std::views::cartesian_product(down_range, right_range)
                | std::views::transform([](const auto & p) noexcept {
                    return N { std::get<0>(p), std::get<1>(p) };
                });
        }

        auto get_edges() {
            auto down_range { std::views::iota(0u, down_node_cnt) };
            auto right_range { std::views::iota(0u, right_node_cnt) };
            return std::views::cartesian_product(down_range, right_range)
                | std::views::transform([&](const auto & p) noexcept {
                    N node { std::get<0>(p), std::get<1>(p) };
                    return this->get_outputs(node);
                })
                | std::views::join;
        }

        bool has_node(const N& node) {
            auto [n1_down, n1_right] = node;
            return n1_down < down_node_cnt && n1_down >= 0u && n1_right < right_node_cnt && n1_right >= 0u;
        }

        bool has_edge(const E& edge) {
            auto [n1_down, n1_right] = edge.first;
            auto [n2_down, n2_right] = edge.second;
            return (n1_down == n2_down && n1_right + 1u == n2_right && n2_right < right_node_cnt)
                || (n1_down + 1u == n2_down && n1_right == n2_right && n2_down < down_node_cnt)
                || (n1_down + 1u == n2_down && n1_right + 1u == n2_right && n2_down < down_node_cnt && n2_right < right_node_cnt);
        }

        auto get_outputs_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto [n_down, n_right] = node;
            typename static_vector_typer<std::tuple<E, N, N, ED*>, 3u, error_check>::type ret{};
            if (n_down == down_node_cnt - 1u && n_right == right_node_cnt - 1u) {
                // do nothing
            } else if (n_down < down_node_cnt - 1u && n_right < right_node_cnt - 1u) {
                ret.push_back(this->construct_full_edge(node, N {n_down, n_right + 1u}));
                ret.push_back(this->construct_full_edge(node, N {n_down + 1u, n_right}));
                ret.push_back(this->construct_full_edge(node, N {n_down + 1u, n_right + 1u}));
            } else if (n_right == right_node_cnt - 1u) {
                ret.push_back(this->construct_full_edge(node, N {n_down + 1u, n_right}));
            } else if (n_down == down_node_cnt - 1u) {
                ret.push_back(this->construct_full_edge(node, N {n_down, n_right + 1u}));
            }
            return std::move(ret)
                | std::views::transform([](const auto& edge_full) noexcept {
                    auto [e, n1, n2, ed_ptr] = edge_full;
                    return std::tuple<E, N, N, ED&>(e, n1, n2, *ed_ptr);
                });
        }

        std::tuple<E, N, N, ED&> get_output_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto it = this->get_outputs_full(node).begin();
            const auto& ret = *it;
            ++it;
            if (it != this->get_outputs_full(node).end()) {
                throw std::runtime_error {"More than one edge exists"};
            }
            return ret;
        }

        auto get_inputs_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto [n_down, n_right] = node;
            typename static_vector_typer<std::tuple<E, N, N, ED*>, 3u, error_check>::type ret{};
            if (n_down == 0u && n_right == 0u) {
                // do nothing
            } else if (n_down > 0u && n_right > 0u) {
                ret.push_back(this->construct_full_edge(N {n_down, n_right - 1u}, node));
                ret.push_back(this->construct_full_edge(N {n_down - 1u, n_right}, node));
                ret.push_back(this->construct_full_edge(N {n_down - 1u, n_right - 1u}, node));
            } else if (n_right > 0u) {
                ret.push_back(this->construct_full_edge(N {n_down, n_right - 1u}, node));
            } else if (n_down > 0u) {
                ret.push_back(this->construct_full_edge(N {n_down - 1u, n_right}, node));
            }
            return std::move(ret)
                | std::views::transform([](auto& edge_full) noexcept {
                    auto [e, n1, n2, ed_ptr] = edge_full;
                    return std::tuple<E, N, N, ED&>(e, n1, n2, *ed_ptr);
                });
        }

        std::tuple<const E&, const N&, const N&, ED&> get_input_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto it = this->get_inputs_full(node).begin();
            const auto& ret = *it;
            ++it;
            if (it != this->get_inputs_full(node).end()) {
                throw std::runtime_error {"More than one edge exists"};
            }
            return ret;
        }

        auto get_outputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_outputs_full(node) | std::views::transform([this](auto v) noexcept -> E { return std::get<0>(v); });
        }

        E get_output(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto it = this->get_outputs(node).begin();
            const auto& ret = *it;
            ++it;
            if (it != this->get_outputs_full(node).end()) {
                throw std::runtime_error {"More than one edge exists"};
            }
            return ret;
        }

        auto get_inputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_inputs_full(node) | std::views::transform([this](auto v) noexcept -> E { return std::get<0>(v); });
        }

        E get_input(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto it = this->get_inputs(node).begin();
            const auto& ret = *it;
            ++it;
            if (it != this->get_outputs_full(node).end()) {
                throw std::runtime_error {"More than one edge exists"};
            }
            return ret;
        }

        bool has_outputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_outputs(node).size() > 0u;
        }

        bool has_inputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_inputs(node).size() > 0u;
        }

        std::size_t get_out_degree(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_outputs(node).size();
        }

        std::size_t get_in_degree(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_inputs(node).size();
        }

        constexpr static INDEX node_count(
            INDEX _down_node_cnt,
            INDEX _right_node_cnt
        ) {
            return _down_node_cnt * _right_node_cnt;
        }

        constexpr static INDEX edge_count(
            INDEX _down_node_cnt,
            INDEX _right_node_cnt
        ) {
            // Start off by assuming each node has 3 outgoing edges.
            INDEX edge_cnt { (_down_node_cnt * _right_node_cnt) * 3u };
            // The leaf node doesn't have any outgoing edges, so adjust for that.
            edge_cnt -= 3u;
            // The right-most column (not counting the leaf node) only has down-ward edges, so adjust for that.
            if (_down_node_cnt > 1u) {
                edge_cnt -= (_down_node_cnt - 1u) * 2u;
            }
            // The down-most column (not counting the leaf node) only has down-ward edges, so adjust for that.
            if (_right_node_cnt > 1u) {
                edge_cnt -= (_right_node_cnt - 1u) * 2u;
            }
            return edge_cnt;
        }

        constexpr static INDEX longest_path_edge_count(
            INDEX _down_node_cnt,
            INDEX _right_node_cnt
        ) {
            return (_right_node_cnt - 1u) + (_down_node_cnt - 1u);
        }
    };
}
#endif //OFFBYNULL_ALIGNER_GRAPHS_GRID_GRAPH_H
