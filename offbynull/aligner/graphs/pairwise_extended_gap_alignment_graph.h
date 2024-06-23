#ifndef OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_EXTENDED_GAP_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_EXTENDED_GAP_ALIGNMENT_GRAPH_H

#include <cstddef>
#include <ranges>
#include <tuple>
#include <stdexcept>
#include <utility>
#include <functional>
#include <stdfloat>
#include "offbynull/aligner/concepts.h"
#include "offbynull/concepts.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph {
    using offbynull::aligner::concepts::weight;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::utils::concat_view;
    using offbynull::utils::static_vector_typer;

    using empty_type = std::tuple<>;

    enum class layer : uint8_t {
        DIAGONAL,
        DOWN,
        RIGHT
    };

    template<
        std::ranges::random_access_range DOWN_SEQ,
        std::ranges::random_access_range RIGHT_SEQ,
        widenable_to_size_t INDEX_ = std::size_t,
        weight WEIGHT = std::float64_t,
        bool error_check = true
    >
    class pairwise_extended_gap_alignment_graph {
    public:
        using INDEX = INDEX_;
        using N = std::tuple<layer, INDEX, INDEX>;
        using ND = empty_type;
        using E = std::pair<N, N>;
        using ED = WEIGHT;

    private:
        const DOWN_SEQ& down_seq;
        const RIGHT_SEQ& right_seq;
        std::function<
            WEIGHT(
                const E&,
                const std::decay_t<decltype(down_seq[0u])>&,
                const std::decay_t<decltype(right_seq[0u])>&
            )
        > match_lookup;
        std::function<
            WEIGHT(
                const E&
            )
        > initial_indel_lookup;
        std::function<
            WEIGHT(
                const E&
            )
        > extended_indel_lookup;
        std::function<
            WEIGHT(
                const E&
            )
        > freeride_lookup;

        auto construct_full_edge(N n1, N n2) {
            return std::tuple<E, N, N, ED> {
                E { n1, n2 },
                n1,
                n2,
                get_edge_data(E { n1, n2 })
            };
        }

        std::size_t to_raw_idx(INDEX down_idx, INDEX right_idx) {
            std::size_t down_idx_widened { down_idx };
            std::size_t right_idx_widened { right_idx };
            return (down_idx_widened * grid_right_cnt) + right_idx_widened;
        }

    public:
        const INDEX grid_down_cnt;
        const INDEX grid_right_cnt;

        pairwise_extended_gap_alignment_graph(
            const DOWN_SEQ& _down_seq,
            const RIGHT_SEQ& _right_seq,
            std::function<
                WEIGHT(
                    const E&,
                    const std::decay_t<decltype(_down_seq[0u])>&,
                    const std::decay_t<decltype(_right_seq[0u])>&
                )
            > _match_lookup,
            std::function<
                WEIGHT(
                    const E&
                )
            > _initial_indel_lookup,
            std::function<
                WEIGHT(
                    const E&
                )
            > _extended_indel_lookup,
            std::function<
                WEIGHT(
                    const E&
                )
            > _freeride_lookup
        )
        : down_seq{_down_seq}
        , right_seq{_right_seq}
        , match_lookup{_match_lookup}
        , initial_indel_lookup{_initial_indel_lookup}
        , extended_indel_lookup{_extended_indel_lookup}
        , freeride_lookup{_freeride_lookup}
        , grid_down_cnt{_down_seq.size() + 1zu}
        , grid_right_cnt{_right_seq.size() + 1zu} {}

        ND get_node_data(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return {};
        }

        ED get_edge_data(const E& edge) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            const auto& [n1_layer, n1_grid_down, n1_grid_right] { edge.first };
            const auto& [n2_layer, n2_grid_down, n2_grid_right] { edge.second };
            if (n1_layer == layer::DIAGONAL && n2_layer == layer::DIAGONAL) {  // match
                return match_lookup(
                    edge,
                    down_seq[n1_grid_down],
                    right_seq[n1_grid_right]
                );
            } else if (n1_layer == layer::DOWN && n2_layer == layer::DOWN) {  // gap
                return extended_indel_lookup(edge);
            } else if (n1_layer == layer::RIGHT && n2_layer == layer::RIGHT) {  // gap
                return extended_indel_lookup(edge);
            } else if (n1_layer == layer::DIAGONAL && n2_layer == layer::DOWN) {  // indel
                return initial_indel_lookup(edge);
            } else if (n1_layer == layer::DIAGONAL && n2_layer == layer::RIGHT) {  // indel
                return initial_indel_lookup(edge);
            } else if (n1_layer == layer::DOWN && n2_layer == layer::DIAGONAL) {  // freeride
                return freeride_lookup(edge);
            } else if (n1_layer == layer::RIGHT && n2_layer == layer::DIAGONAL) {  // freeride
                return freeride_lookup(edge);
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

        std::tuple<N, N, ED> get_edge(const E& edge) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            return std::tuple<N, N, ED> {get_edge_from(edge), get_edge_from(edge), get_edge_data(edge)};
        }

        auto get_root_nodes() {
            return std::ranges::single_view { N { layer::DIAGONAL, 0u, 0u } };
        }

        N get_root_node() {
            return N { layer::DIAGONAL, 0u, 0u };
        }

        auto get_leaf_nodes() {
            return std::ranges::single_view { N { layer::DIAGONAL, grid_down_cnt - 1u, grid_right_cnt - 1u } };
        }

        N get_leaf_node() {
            return N { layer::DIAGONAL, grid_down_cnt - 1u, grid_right_cnt - 1u };
        }

        auto get_nodes() {
            auto diagonal_layer_nodes {
                std::views::cartesian_product(
                    std::views::iota(0u, grid_down_cnt),
                    std::views::iota(0u, grid_right_cnt)
                )
                | std::views::transform([](const auto & p) noexcept {
                    const auto &[grid_down, grid_right] { p };
                    return N { layer::DIAGONAL, grid_down, grid_right };
                })
            };
            auto down_layer_nodes {
                std::views::cartesian_product(
                    std::views::iota(1u, grid_down_cnt),
                    std::views::iota(0u, grid_right_cnt)
                )
                | std::views::transform([](const auto & p) noexcept {
                    const auto &[grid_down, grid_right] { p };
                    return N { layer::DOWN, grid_down, grid_right };
                })
            };
            auto right_layer_nodes {
                std::views::cartesian_product(
                    std::views::iota(0u, grid_down_cnt),
                    std::views::iota(1u, grid_right_cnt)
                )
                | std::views::transform([](const auto & p) noexcept {
                    const auto &[grid_down, grid_right] { p };
                    return N { layer::RIGHT, grid_down, grid_right };
                })
            };
            return concat_view(
                std::move(diagonal_layer_nodes),
                concat_view(
                    std::move(down_layer_nodes),
                    std::move(right_layer_nodes)
                )
            );
        }

        auto get_edges() {
            auto diagonal_layer_edges {
                std::views::cartesian_product(
                    std::views::iota(0u, grid_down_cnt),
                    std::views::iota(0u, grid_right_cnt)
                )
                | std::views::transform([&](const auto & p) noexcept {
                    const auto &[grid_down, grid_right] { p };
                    return this->get_outputs(N { layer::DIAGONAL, grid_down, grid_right });
                })
                | std::views::join
            };
            auto down_layer_edges {
                std::views::cartesian_product(
                    std::views::iota(1u, grid_down_cnt),
                    std::views::iota(0u, grid_right_cnt)
                )
                | std::views::transform([&](const auto & p) noexcept {
                    const auto &[grid_down, grid_right] { p };
                    return this->get_outputs(N { layer::DOWN, grid_down, grid_right });
                })
                | std::views::join
            };
            auto right_layer_edges {
                std::views::cartesian_product(
                    std::views::iota(0u, grid_down_cnt),
                    std::views::iota(1u, grid_right_cnt)
                )
                | std::views::transform([&](const auto & p) noexcept {
                    const auto &[grid_down, grid_right] { p };
                    return this->get_outputs(N { layer::RIGHT, grid_down, grid_right });
                })
                | std::views::join
            };
            return concat_view(
                std::move(diagonal_layer_edges),
                concat_view(
                    std::move(down_layer_edges),
                    std::move(right_layer_edges)
                )
            );
        }

        bool has_node(const N& node) {
            const auto& [n_layer, grid_down, grid_right] { node };
            return (n_layer == layer::DIAGONAL && grid_down < grid_down_cnt && grid_down >= 0u && grid_right < grid_right_cnt && grid_right >= 0u)
                || (n_layer == layer::DOWN && grid_down < grid_down_cnt && grid_down >= 1u && grid_right < grid_right_cnt && grid_right >= 0u)
                || (n_layer == layer::RIGHT && grid_down < grid_down_cnt && grid_down >= 0u && grid_right < grid_right_cnt && grid_right >= 1u);
        }

        bool has_edge(const E& edge) {
            const auto& [n1_layer, n1_grid_down, n1_grid_right] { edge.first };
            const auto& [n2_layer, n2_grid_down, n2_grid_right] { edge.second };
            return (n1_layer == layer::DIAGONAL && n2_layer == layer::DIAGONAL && n1_grid_down + 1u == n2_grid_down && n1_grid_right + 1u == n2_grid_right && n2_grid_down < grid_down_cnt && n2_grid_right < grid_right_cnt)  // match
                || (n1_layer == layer::DIAGONAL && n2_layer == layer::DOWN && n1_grid_down + 1u == n2_grid_down && n1_grid_right == n2_grid_right && n2_grid_down < grid_down_cnt && n2_grid_right < grid_right_cnt)  // initial gap (down)
                || (n1_layer == layer::DIAGONAL && n2_layer == layer::RIGHT && n1_grid_down == n2_grid_down && n1_grid_right + 1u == n2_grid_right && n2_grid_down < grid_down_cnt && n2_grid_right < grid_right_cnt) // initial gap (right)
                || (n1_layer == layer::DOWN && n2_layer == layer::DOWN && n1_grid_down > 0u && n1_grid_down + 1u == n2_grid_down && n1_grid_right == n2_grid_right && n2_grid_down < grid_down_cnt && n2_grid_right < grid_right_cnt)  // extended gap (down)
                || (n1_layer == layer::RIGHT && n2_layer == layer::RIGHT && n1_grid_right > 0u && n1_grid_down == n2_grid_down && n1_grid_right + 1u == n2_grid_right && n2_grid_down < grid_down_cnt && n2_grid_right < grid_right_cnt) // extended gap (right)
                || (n1_layer == layer::DOWN && n2_layer == layer::DIAGONAL && n1_grid_down > 0u && n1_grid_down == n2_grid_down && n1_grid_right == n2_grid_right && n2_grid_down < grid_down_cnt && n2_grid_right < grid_right_cnt)  // freeride (down)
                || (n1_layer == layer::RIGHT && n2_layer == layer::DIAGONAL && n1_grid_right > 0u && n1_grid_down == n2_grid_down && n1_grid_right == n2_grid_right && n2_grid_down < grid_down_cnt && n2_grid_right < grid_right_cnt); // freeride (right)
        }

        auto get_outputs_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            typename static_vector_typer<std::tuple<E, N, N, ED>, 3u, error_check>::type ret {};
            const auto& [n1_layer, n1_grid_down, n1_grid_right] { node };
            if (n1_layer == layer::DIAGONAL) {
                if (n1_grid_down == grid_down_cnt - 1u && n1_grid_right == grid_right_cnt - 1u) {
                    // do nothing
                } else if (n1_grid_down < grid_down_cnt - 1u && n1_grid_right < grid_right_cnt - 1u) {
                    ret.push_back(construct_full_edge(node, { layer::DIAGONAL, n1_grid_down + 1u, n1_grid_right + 1u }));
                    ret.push_back(construct_full_edge(node, { layer::DOWN, n1_grid_down + 1u, n1_grid_right }));
                    ret.push_back(construct_full_edge(node, { layer::RIGHT, n1_grid_down, n1_grid_right + 1u }));
                } else if (n1_grid_right == grid_right_cnt - 1u) {
                    ret.push_back(construct_full_edge(node, { layer::DOWN, n1_grid_down + 1u, n1_grid_right }));
                } else if (n1_grid_down == grid_down_cnt - 1u) {
                    ret.push_back(construct_full_edge(node, { layer::RIGHT, n1_grid_down, n1_grid_right + 1u }));
                }
            } else if (n1_layer == layer::DOWN) {
                ret.push_back(construct_full_edge(node, { layer::DIAGONAL, n1_grid_down, n1_grid_right }));
                if (n1_grid_down < grid_down_cnt - 1u) {
                    ret.push_back(construct_full_edge(node, { layer::DOWN, n1_grid_down + 1u, n1_grid_right }));
                }
            } else if (n1_layer == layer::RIGHT) {
                ret.push_back(construct_full_edge(node, { layer::DIAGONAL, n1_grid_down, n1_grid_right }));
                if (n1_grid_right < grid_right_cnt - 1u) {
                    ret.push_back(construct_full_edge(node, { layer::RIGHT, n1_grid_down, n1_grid_right + 1u }));
                }
            }
            return ret;
        }

        auto get_inputs_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            typename static_vector_typer<std::tuple<E, N, N, ED>, 3u, error_check>::type ret {};
            const auto& [n2_layer, n2_grid_down, n2_grid_right] { node };
            if (n2_layer == layer::DIAGONAL) {
                if (n2_grid_down == 0u && n2_grid_right == 0u) {
                    // do nothing
                } else if (n2_grid_down > 0u && n2_grid_right > 0u) {
                    ret.push_back(construct_full_edge({ layer::DIAGONAL, n2_grid_down - 1u, n2_grid_right - 1u }, node));
                    ret.push_back(construct_full_edge({ layer::DOWN, n2_grid_down, n2_grid_right }, node));
                    ret.push_back(construct_full_edge({ layer::RIGHT, n2_grid_down, n2_grid_right }, node));
                } else if (n2_grid_right > 0u) {
                    ret.push_back(construct_full_edge({ layer::RIGHT, n2_grid_down, n2_grid_right }, node));
                } else if (n2_grid_down > 0u) {
                    ret.push_back(construct_full_edge({ layer::DOWN, n2_grid_down, n2_grid_right }, node));
                }
            } else if (n2_layer == layer::DOWN) {
                if (n2_grid_down > 0u) {
                    ret.push_back(construct_full_edge({ layer::DIAGONAL, n2_grid_down - 1u, n2_grid_right }, node));
                }
                if (n2_grid_down > 1u) {
                    ret.push_back(construct_full_edge({ layer::DOWN, n2_grid_down - 1u, n2_grid_right }, node));
                }
            } else if (n2_layer == layer::RIGHT) {
                if (n2_grid_right > 0u) {
                    ret.push_back(construct_full_edge({ layer::DIAGONAL, n2_grid_down, n2_grid_right - 1u }, node));
                }
                if (n2_grid_right > 1u) {
                    ret.push_back(construct_full_edge({ layer::RIGHT, n2_grid_down, n2_grid_right - 1u }, node));
                }
            }
            return ret;
        }

        auto get_outputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_outputs_full(node) | std::views::transform([this](const auto& v) noexcept -> E { return std::get<0>(v); });
        }

        auto get_inputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_inputs_full(node) | std::views::transform([this](const auto& v) noexcept -> E { return std::get<0>(v); });
        }

        bool has_outputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_outputs(node).size() > 0zu;
        }

        bool has_inputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_inputs(node).size() > 0zu;
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

        auto edge_to_element_offsets(
            const E& edge
        ) {
            using OPT_INDEX = std::optional<INDEX>;
            using RET = std::optional<std::pair<OPT_INDEX, OPT_INDEX>>;

            const auto& [n1, n2] { edge };
            const auto& [n1_layer, n1_grid_down, n1_grid_right] { n1 };
            const auto& [n2_layer, n2_grid_down, n2_grid_right] { n2 };
            if (n1_layer == layer::DIAGONAL && n2_layer == layer::DIAGONAL) {  // match
                if (n1_grid_down + 1u == n2_grid_down && n1_grid_right + 1u == n2_grid_right) {
                    return RET { { { n1_grid_down }, { n1_grid_right } } };
                }
            } else if ((n1_layer == layer::DOWN && n2_layer == layer::DOWN)  // extended indel
                || (n1_layer == layer::DIAGONAL && n2_layer == layer::DOWN)) {  // indel
                if (n1_grid_down + 1u == n2_grid_down && n1_grid_right == n2_grid_right) {
                    return RET { { { n1_grid_down }, std::nullopt } };
                }
            } else if ((n1_layer == layer::RIGHT && n2_layer == layer::RIGHT)  // extended indel
                || (n1_layer == layer::DIAGONAL && n2_layer == layer::RIGHT)) {  // indel
                if (n1_grid_down == n2_grid_down && n1_grid_right + 1u == n2_grid_right) {
                    return RET { { std::nullopt, { n1_grid_right } } };
                }
            } else if ((n1_layer == layer::DOWN && n2_layer == layer::DIAGONAL)  // freeride
                || (n1_layer == layer::RIGHT && n2_layer == layer::DIAGONAL)) {  // freeride
                return RET { std::nullopt };
            }
            if constexpr (error_check) {
                throw std::runtime_error("Bad edge");
            }
            std::unreachable();
        }

        std::pair<INDEX, INDEX> node_to_grid_offsets(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return { std::get<1>(node), std::get<2>(node) };
        }

        constexpr static INDEX node_count(
            INDEX _grid_down_cnt,
            INDEX _grid_right_cnt
        ) {
            INDEX node_cnt {};
            node_cnt += _grid_down_cnt * _grid_right_cnt; // Middle layer
            node_cnt += (_grid_down_cnt - 1u) * _grid_right_cnt; // Down gap layer
            node_cnt += _grid_down_cnt * (_grid_right_cnt - 1u); // Right gap layer
            return node_cnt;
        }

        constexpr static INDEX longest_path_edge_count(
            INDEX _grid_down_cnt,
            INDEX _grid_right_cnt
        ) {
            return (_grid_right_cnt - 1u) * 2u + (_grid_down_cnt - 1u) * 2u;
        }

        constexpr static std::size_t slice_nodes_capacity(INDEX _grid_down_cnt, INDEX _grid_right_cnt) {
            // THIS IS NOT SUPPOSED TO BE THE CAPACITY AT EACH SLICE, this is supposed to be the max capacity across all slices, meaning the implementation below is correct.
            return 1zu + 3zu * (_grid_right_cnt - 1zu);
        }

        auto slice_nodes(INDEX grid_down) {
            return std::views::iota(1u, grid_right_cnt)
                | std::views::transform(
                    [grid_down](const auto& grid_right) {
                        using CONTAINER = typename static_vector_typer<N, 3zu, error_check>::type;
                        CONTAINER ret {};
                        if (grid_down == 0u) {
                            ret.push_back(N { layer::DIAGONAL, grid_down, grid_right });
                        } else {
                            if (grid_right != 0u) {
                                ret.push_back(N { layer::DOWN, grid_down, grid_right });
                                ret.push_back(N { layer::RIGHT, grid_down, grid_right });
                            }
                            ret.push_back(N { layer::DIAGONAL, grid_down, grid_right });
                        }
                        return ret;
                    }
                )
                | std::views::join;
        }

        auto slice_nodes(INDEX grid_down, INDEX override_grid_right_cnt) {
            auto node_cnt { slice_nodes_capacity(0u, override_grid_right_cnt) };
            return slice_nodes(grid_down)
                | std::views::take(node_cnt);
        }

        N slice_first_node(INDEX grid_down) {
            return slice_first_node(grid_down, 0u);
        }

        N slice_first_node(INDEX grid_down, INDEX grid_right) {
            N first_node;
            if (grid_down == 0u) {
                first_node = { layer::DIAGONAL, grid_down, grid_right };
            } else {
                if (grid_right == 0u) {
                    first_node = { layer::DIAGONAL, grid_down, grid_right };
                } else {
                    first_node = { layer::DOWN, grid_down, grid_right };
                }
            }
            if constexpr (error_check) {
                if (std::get<1>(first_node) >= grid_down_cnt) {
                    throw std::runtime_error("Node too far down");
                }
            }
            return first_node;
        }

        N slice_last_node(INDEX grid_down) {
            return slice_last_node(grid_down, grid_right_cnt - 1u);
        }

        N slice_last_node(INDEX grid_down, INDEX grid_right) {
            N last_node { layer::DOWN, grid_down, grid_right };
            if constexpr (error_check) {
                if (std::get<1>(last_node) >= grid_down_cnt) {
                    throw std::runtime_error("Node too far down");
                }
            }
            return last_node;
        }

        N slice_next_node(const N& node) {
            const auto& [_layer, grid_down, grid_right] { node };
            N next_node;
            if (_layer == layer::DOWN) {
                next_node = N { layer::RIGHT, grid_down, grid_right };
            } else if (_layer == layer::RIGHT) {
                next_node = N { layer::DIAGONAL, grid_down, grid_right };
            } else if (_layer == layer::DIAGONAL) {
                next_node = N { layer::DOWN, grid_down, grid_right + 1u };
            } else {
                if (error_check) {
                    throw std::runtime_error("This should never happen");
                }
            }
            if constexpr (error_check) {
                if (std::get<1>(next_node) >= grid_down_cnt) {
                    throw std::runtime_error("Node too far down");
                }
                if (std::get<2>(next_node) >= grid_right_cnt) {
                    throw std::runtime_error("Node too far right");
                }
            }
            return next_node;
        }

        N slice_prev_node(const N& node) {
            const auto& [_layer, grid_down, grid_right] { node };
            N prev_node;
            if (_layer == layer::DIAGONAL) {
                prev_node = N { layer::RIGHT, grid_down, grid_right };
            } else if (_layer == layer::RIGHT) {
                prev_node = N { layer::DOWN, grid_down, grid_right };
            } else if (_layer == layer::DOWN) {
                prev_node = N { layer::DIAGONAL, grid_down, grid_right - 1u };
            } else {
                if (error_check) {
                    throw std::runtime_error("This should never happen");
                }
            }
            if constexpr (error_check) {
                if (std::get<1>(prev_node) >= grid_down_cnt) {
                    throw std::runtime_error("Node too far down");
                }
                if (std::get<2>(prev_node) >= grid_right_cnt) {
                    throw std::runtime_error("Node too far right");
                }
            }
            return prev_node;
        }

        constexpr static std::size_t resident_nodes_capacity(INDEX _grid_down_cnt, INDEX _grid_right_cnt) {
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
#endif //OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_EXTENDED_GAP_ALIGNMENT_GRAPH_H
