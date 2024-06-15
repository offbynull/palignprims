#ifndef OFFBYNULL_ALIGNER_GRAPHS_GRID_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_GRID_GRAPH_H

#include <cstddef>
#include <ranges>
#include <tuple>
#include <stdexcept>
#include <utility>
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
            return std::tuple<E, N, N, ED&> {
                E { n1, n2 },
                n1,
                n2,
                this->get_edge_data(
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
                | std::views::transform([](const auto & p) {
                    return N { std::get<0>(p), std::get<1>(p) };
                });
        }

        auto get_edges() {
            auto down_range { std::views::iota(0u, down_node_cnt) };
            auto right_range { std::views::iota(0u, right_node_cnt) };
            return std::views::cartesian_product(down_range, right_range)
                | std::views::transform([&](const auto & p) {
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
            // Cartesian product has some issues with bloat, so not using it here:
            //     std::views::cartesian_product(
            //         std::views::iota(static_cast<INDEX>(0), static_cast<INDEX>(1)),
            //         std::views::iota(static_cast<INDEX>(0), static_cast<INDEX>(1))
            //     )
            //     | std::views::drop(1)
            std::array<N, 3zu> offsets {
                std::pair{ static_cast<INDEX>(0),static_cast<INDEX>(1) },
                std::pair{ static_cast<INDEX>(1),static_cast<INDEX>(0) },
                std::pair{ static_cast<INDEX>(1),static_cast<INDEX>(1) }
            };
            return std::move(offsets)
                | std::views::filter([node, this](const auto& offset) {
                    const auto& [down_offset, right_offset] { offset };
                    const auto& [n_down, n_right] { node };
                    if (down_offset == 1u && n_down == down_node_cnt - 1u) {
                        return false;
                    }
                    if (right_offset == 1u && n_right == right_node_cnt - 1u) {
                        return false;
                    }
                    return true;
                })
                | std::views::transform([node, this](const auto& offset) {
                    const auto& [down_offset, right_offset] { offset };
                    const auto& [n_down, n_right] { node };
                    N n2 { n_down + down_offset, n_right + right_offset };
                    return this->construct_full_edge(node, n2);
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
            std::array<N, 3zu> offsets {
                std::pair{ static_cast<INDEX>(0),static_cast<INDEX>(1) },
                std::pair{ static_cast<INDEX>(1),static_cast<INDEX>(0) },
                std::pair{ static_cast<INDEX>(1),static_cast<INDEX>(1) }
            };
            return std::move(offsets)
                | std::views::filter([node, this](const auto& offset) {
                    const auto& [down_offset, right_offset] { offset };
                    const auto& [n_down, n_right] { node };
                    if (down_offset == 1u && n_down == 0u) {
                        return false;
                    }
                    if (right_offset == 1u && n_right == 0u) {
                        return false;
                    }
                    return true;
                })
                | std::views::transform([node, this](const auto& offset) {
                    const auto& [down_offset, right_offset] { offset };
                    const auto& [n_down, n_right] { node };
                    N n1 { n_down - down_offset, n_right - right_offset };
                    return this->construct_full_edge(n1, node);
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
            return this->get_outputs_full(node)
                | std::views::transform([](const auto& v) -> E { return std::get<0>(v); });
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
            if (it != this->get_outputs(node).end()) {
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
            return this->get_inputs_full(node)
                | std::views::transform([](const auto& v) -> E { return std::get<0>(v); });
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
            if (it != this->get_inputs(node).end()) {
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
            return this->get_out_degree(node) > 0zu;
        }

        bool has_inputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_in_degree(node) > 0zu;
        }

        std::size_t get_out_degree(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto outputs { this->get_outputs(node) };
            auto dist { std::distance(outputs.begin(), outputs.end()) };
            return static_cast<std::size_t>(dist);
        }

        std::size_t get_in_degree(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto inputs { this->get_inputs(node) };
            auto dist { std::distance(inputs.begin(), inputs.end()) };
            return static_cast<std::size_t>(dist);
        }

        constexpr static INDEX node_count(
            INDEX _down_node_cnt,
            INDEX _right_node_cnt
        ) {
            return _down_node_cnt * _right_node_cnt;
        }

        constexpr static INDEX longest_path_edge_count(
            INDEX _down_node_cnt,
            INDEX _right_node_cnt
        ) {
            return (_right_node_cnt - 1u) + (_down_node_cnt - 1u);
        }

        std::pair<INDEX, INDEX> node_to_grid_offsets(const N& node) {
            return node;
        }

        static std::size_t slice_nodes_capacity(INDEX _down_node_cnt, INDEX _right_node_cnt) {
            return _right_node_cnt;
        }

        auto slice_nodes(INDEX n_down) {
            return std::views::iota(0u, right_node_cnt)
                | std::views::transform([n_down](const auto& n_right) { return N { n_down, n_right }; });
        }

        N first_node_in_slice(INDEX n_down) {
            N first_node { n_down, 0u };
            if constexpr (error_check) {
                if (std::get<0>(first_node) >= down_node_cnt) {
                    throw std::runtime_error("Node too far down");
                }
            }
            return first_node;
        }

        N last_node_in_slice(INDEX n_down) {
            N last_node { n_down, right_node_cnt - 1u };
            if constexpr (error_check) {
                if (std::get<0>(last_node) >= down_node_cnt) {
                    throw std::runtime_error("Node too far down");
                }
            }
            return last_node;
        }

        N next_node_in_slice(const N& node) {
            N next_node { std::get<0>(node), std::get<1>(node) + 1u};
            if constexpr (error_check) {
                if (std::get<0>(next_node) >= down_node_cnt) {
                    throw std::runtime_error("Node too far down");
                }
                if (std::get<1>(next_node) >= right_node_cnt) {
                    throw std::runtime_error("Node too far right");
                }
            }
            return next_node;
        }

        N prev_node_in_slice(const N& node) {
            N prev_node { std::get<0>(node), std::get<1>(node) - 1u};
            if constexpr (error_check) {
                if (std::get<0>(prev_node) >= down_node_cnt) {
                    throw std::runtime_error("Node too far down");
                }
                if (std::get<1>(prev_node) >= right_node_cnt) {
                    throw std::runtime_error("Node too far right");
                }
            }
            return prev_node;
        }

        static std::size_t resident_nodes_capacity(INDEX _down_node_cnt, INDEX _right_node_cnt) {
            return 0zu;
        }

        auto resident_nodes() {
            return std::views::empty<N>;
        }

        auto outputs_to_residents(const N& node) {
            return std::views::empty<E>;
        }

        auto inputs_from_residents(const N& node) {
            return std::views::empty<E>;
        }
    };
}
#endif //OFFBYNULL_ALIGNER_GRAPHS_GRID_GRAPH_H
