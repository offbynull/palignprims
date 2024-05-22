#ifndef OFFBYNULL_ALIGNER_GRAPHS_GRID_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_GRID_GRAPH_H

#include <ranges>
#include <tuple>
#include <stdexcept>
#include <utility>
#include "boost/container/static_vector.hpp"
#include "offbynull/aligner/graph/grid_allocator.h"
#include "offbynull/aligner/graph/grid_allocators.h"

namespace offbynull::aligner::graphs::grid_graph {
    using offbynull::aligner::graph::grid_allocator::grid_allocator;
    using offbynull::aligner::graph::grid_allocators::VectorAllocator;

    template<
        typename ND_,
        typename ED_,
        std::unsigned_integral T = unsigned int,
        grid_allocator<T> ND_ALLOCATOR_ = VectorAllocator<ND_, T, false>,
        grid_allocator<T> ED_ALLOCATOR_ = VectorAllocator<ED_, T, false>,
        bool error_check = true
    >
    class grid_graph {
    public:
        using N = std::pair<T, T>;
        using ND = ND_;
        using E = std::pair<N, N>;
        using ED = ED_;

        const T down_node_cnt;
        const T right_node_cnt;

    private:
        decltype(std::declval<ND_ALLOCATOR_>().allocate(0u, 0u)) nodes;
        decltype(std::declval<ED_ALLOCATOR_>().allocate(0u, 0u)) edges;

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

        size_t to_raw_idx(size_t down_idx, size_t right_idx) {
            return (down_idx * right_node_cnt) + right_idx;
        }

    public:
        grid_graph(
            T _down_node_cnt,
            T _right_node_cnt,
            ED indel_data = {},
            ND_ALLOCATOR_ nd_container_creator = {},
            ED_ALLOCATOR_ ed_container_creator = {}
        )
        : down_node_cnt{_down_node_cnt}
        , right_node_cnt{_right_node_cnt}
        , nodes{nd_container_creator.allocate(_down_node_cnt, _right_node_cnt)}
        , edges{ed_container_creator.allocate(_down_node_cnt, _right_node_cnt)}
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
            // throw std::runtime_error("This shouldn't happen if error checking is enabled");
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
            return std::tuple<N, N, ED&> {this->get_edge_from(edge), this->get_edge_from(edge), this->get_edge_data(edge)};
        }

        auto get_root_nodes() {
            return std::ranges::single_view { std::pair<T, T>(0u, 0u) };
        }

        N get_root_node() {
            return std::pair<T, T>(0u, 0u);
        }

        auto get_leaf_nodes() {
            return std::ranges::single_view { std::pair<T, T>(down_node_cnt - 1u, right_node_cnt - 1u) };
        }

        auto get_nodes() {
            auto down_range { std::views::iota(0u, down_node_cnt) };
            auto right_range { std::views::iota(0u, right_node_cnt) };
            return
                std::views::cartesian_product(down_range, right_range)
                | std::views::transform([](const auto & p) noexcept {
                    return std::pair<T, T>(std::get<0>(p), std::get<1>(p));
                });
        }

        auto get_edges() {
            auto down_range { std::views::iota(0u, down_node_cnt) };
            auto right_range { std::views::iota(0u, right_node_cnt) };
            return std::views::cartesian_product(down_range, right_range)
                | std::views::transform([&](const auto & p) noexcept {
                    std::pair<T, T> node { std::get<0>(p), std::get<1>(p) };
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
            boost::container::static_vector<std::tuple<E, N, N, ED*>, 3> ret {};
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
            boost::container::static_vector<std::tuple<E, N, N, ED*>, 3> ret {};
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

        size_t get_out_degree(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_outputs(node).size();
        }

        size_t get_in_degree(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_inputs(node).size();
        }
    };
}
#endif //OFFBYNULL_ALIGNER_GRAPHS_GRID_GRAPH_H
