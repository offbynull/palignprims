#ifndef OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_EXTENDED_GAP_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_EXTENDED_GAP_ALIGNMENT_GRAPH_H

#include <cstdint>
#include <cstddef>
#include <ranges>
#include <tuple>
#include <stdexcept>
#include <utility>
#include <functional>
#include <stdfloat>
#include <iostream>
#include <format>
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/concepts.h"
#include "offbynull/helpers/concat_view.h"
#include "offbynull/utils.h"
#include "offbynull/helpers/simple_value_bidirectional_view.h"

namespace offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph {
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::helpers::concat_view::concat_view;
    using offbynull::utils::static_vector_typer;
    using offbynull::helpers::simple_value_bidirectional_view::simple_value_bidirectional_view;

    using empty_type = std::tuple<>;

    enum class node_layer : std::uint8_t {
        DOWN,
        RIGHT,
        DIAGONAL  // Diag is last - while in same grid offset, this is the layer that the other two feed into. Algorithms expect this.
    };

    template<widenable_to_size_t INDEX>
    struct node {
        node_layer layer;
        INDEX down;
        INDEX right;
        auto operator<=>(const node&) const = default;
    };

    template<widenable_to_size_t INDEX>
    struct edge {
        node<INDEX> source;
        node<INDEX> destination;
        auto operator<=>(const edge&) const = default;
    };

    template<
        bool debug_mode,
        sequence DOWN_SEQ,
        sequence RIGHT_SEQ,
        widenable_to_size_t INDEX_ = std::size_t,
        weight WEIGHT = std::float64_t
    >
    class pairwise_extended_gap_alignment_graph {
    public:
        using DOWN_ELEM = std::decay_t<decltype(std::declval<DOWN_SEQ>()[0u])>;
        using RIGHT_ELEM = std::decay_t<decltype(std::declval<RIGHT_SEQ>()[0u])>;
        using INDEX = INDEX_;
        using N = node<INDEX>;
        using ND = empty_type;
        using E = edge<INDEX>;
        using ED = WEIGHT;

    private:
        const DOWN_SEQ& down_seq;
        const RIGHT_SEQ& right_seq;
        const std::function<
            WEIGHT(
                const E&,
                const std::optional<std::reference_wrapper<const DOWN_ELEM>>,
                const std::optional<std::reference_wrapper<const RIGHT_ELEM>>
            )
        > substitution_lookup;
        const std::function<
            WEIGHT(
                const E&,
                const std::optional<std::reference_wrapper<const DOWN_ELEM>>,
                const std::optional<std::reference_wrapper<const RIGHT_ELEM>>
            )
        > initial_gap_lookup;
        const std::function<
            WEIGHT(
                const E&,
                const std::optional<std::reference_wrapper<const DOWN_ELEM>>,
                const std::optional<std::reference_wrapper<const RIGHT_ELEM>>
            )
        > extended_gap_lookup;
        const std::function<
            WEIGHT(
                const E&,
                const std::optional<std::reference_wrapper<const DOWN_ELEM>>,
                const std::optional<std::reference_wrapper<const RIGHT_ELEM>>
            )
        > freeride_lookup;

        auto construct_full_edge(N n1, N n2) const {
            return std::tuple<E, N, N, ED> {
                E { n1, n2 },
                n1,
                n2,
                get_edge_data(E { n1, n2 })
            };
        }

        std::size_t to_raw_idx(INDEX down_idx, INDEX right_idx) const {
            std::size_t down_idx_widened { down_idx };
            std::size_t right_idx_widened { right_idx };
            return (down_idx_widened * grid_right_cnt) + right_idx_widened;
        }

    public:
        const INDEX grid_down_cnt;
        const INDEX grid_right_cnt;
        static constexpr INDEX grid_depth_cnt { 3u };
        static constexpr std::size_t resident_nodes_capacity { 0zu };
        const std::size_t path_edge_capacity;

        pairwise_extended_gap_alignment_graph(
            const DOWN_SEQ& _down_seq,
            const RIGHT_SEQ& _right_seq,
            const std::function<
                WEIGHT(
                    const E&,
                    const std::optional<std::reference_wrapper<const DOWN_ELEM>>,
                    const std::optional<std::reference_wrapper<const RIGHT_ELEM>>
                )
            > _substitution_lookup,
            const std::function<
                WEIGHT(
                    const E&,
                    const std::optional<std::reference_wrapper<const DOWN_ELEM>>,
                    const std::optional<std::reference_wrapper<const RIGHT_ELEM>>
                )
            > _initial_gap_lookup,
            const std::function<
                WEIGHT(
                    const E&,
                    const std::optional<std::reference_wrapper<const DOWN_ELEM>>,
                    const std::optional<std::reference_wrapper<const RIGHT_ELEM>>
                )
            > _extended_gap_lookup,
            const std::function<
                WEIGHT(
                    const E&,
                    const std::optional<std::reference_wrapper<const DOWN_ELEM>>,
                    const std::optional<std::reference_wrapper<const RIGHT_ELEM>>
                )
            > _freeride_lookup
        )
        : down_seq { _down_seq }
        , right_seq { _right_seq }
        , substitution_lookup { _substitution_lookup }
        , initial_gap_lookup { _initial_gap_lookup }
        , extended_gap_lookup { _extended_gap_lookup }
        , freeride_lookup { _freeride_lookup }
        , grid_down_cnt { _down_seq.size() + 1zu }
        , grid_right_cnt { _right_seq.size() + 1zu }
        , path_edge_capacity { (grid_right_cnt - 1zu) * 2zu + (grid_down_cnt - 1zu) * 2zu } {}

        ND get_node_data(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return {};
        }

        ED get_edge_data(const E& edge) const {
            if constexpr (debug_mode) {
                if (!has_edge(edge)) {
                    throw std::runtime_error { "Edge doesn't exist" };
                }
            }
            const auto& [n1_layer, n1_grid_down, n1_grid_right] { edge.source };
            const auto& [n2_layer, n2_grid_down, n2_grid_right] { edge.destination };
            if (n1_layer == node_layer::DIAGONAL && n2_layer == node_layer::DIAGONAL) {  // match
                return substitution_lookup(
                    edge,
                    { { down_seq[n1_grid_down] } },
                    { { right_seq[n1_grid_right] } }
                );
            } else if (n1_layer == node_layer::DOWN && n2_layer == node_layer::DOWN) {  // gap
                return extended_gap_lookup(
                    edge,
                    { { down_seq[n1_grid_down] } },
                    { std::nullopt }
                );
            } else if (n1_layer == node_layer::RIGHT && n2_layer == node_layer::RIGHT) {  // gap
                return extended_gap_lookup(
                    edge,
                    { std::nullopt },
                    { { right_seq[n1_grid_right] } }
                );
            } else if (n1_layer == node_layer::DIAGONAL && n2_layer == node_layer::DOWN) {  // indel
                return initial_gap_lookup(
                    edge,
                    { { down_seq[n1_grid_down] } },
                    { std::nullopt }
                );
            } else if (n1_layer == node_layer::DIAGONAL && n2_layer == node_layer::RIGHT) {  // indel
                return initial_gap_lookup(
                    edge,
                    { std::nullopt },
                    { { right_seq[n1_grid_right] } }
                );
            } else if (n1_layer == node_layer::DOWN && n2_layer == node_layer::DIAGONAL) {  // freeride
                return freeride_lookup(
                    edge,
                    { std::nullopt },
                    { std::nullopt }
                );
            } else if (n1_layer == node_layer::RIGHT && n2_layer == node_layer::DIAGONAL) {  // freeride
                return freeride_lookup(
                    edge,
                    { std::nullopt },
                    { std::nullopt }
                );
            }
            if constexpr (debug_mode) {
                throw std::runtime_error("Bad edge");
            }
            std::unreachable();
        }

        N get_edge_from(const E& edge) const {
            if constexpr (debug_mode) {
                if (!has_edge(edge)) {
                    throw std::runtime_error { "Edge doesn't exist" };
                }
            }
            return edge.source;
        }

        N get_edge_to(const E& edge) const {
            if constexpr (debug_mode) {
                if (!has_edge(edge)) {
                    throw std::runtime_error { "Edge doesn't exist" };
                }
            }
            return edge.destination;
        }

        std::tuple<N, N, ED> get_edge(const E& edge) const {
            if constexpr (debug_mode) {
                if (!has_edge(edge)) {
                    throw std::runtime_error { "Edge doesn't exist" };
                }
            }
            return std::tuple<N, N, ED> { get_edge_from(edge), get_edge_from(edge), get_edge_data(edge) };
        }

        auto get_root_nodes() const {
            return std::ranges::single_view { N { node_layer::DIAGONAL, 0u, 0u } };
        }

        N get_root_node() const {
            return N { node_layer::DIAGONAL, 0u, 0u };
        }

        auto get_leaf_nodes() const {
            return std::ranges::single_view { N { node_layer::DIAGONAL, grid_down_cnt - 1u, grid_right_cnt - 1u } };
        }

        N get_leaf_node() const {
            return N { node_layer::DIAGONAL, grid_down_cnt - 1u, grid_right_cnt - 1u };
        }

        auto get_nodes() const {
            auto diagonal_layer_nodes {
                std::views::cartesian_product(
                    std::views::iota(0u, grid_down_cnt),
                    std::views::iota(0u, grid_right_cnt)
                )
                | std::views::transform([](const auto & p) noexcept {
                    const auto &[grid_down, grid_right] { p };
                    return N { node_layer::DIAGONAL, grid_down, grid_right };
                })
            };
            auto down_layer_nodes {
                std::views::cartesian_product(
                    std::views::iota(1u, grid_down_cnt),
                    std::views::iota(0u, grid_right_cnt)
                )
                | std::views::transform([](const auto & p) noexcept {
                    const auto &[grid_down, grid_right] { p };
                    return N { node_layer::DOWN, grid_down, grid_right };
                })
            };
            auto right_layer_nodes {
                std::views::cartesian_product(
                    std::views::iota(0u, grid_down_cnt),
                    std::views::iota(1u, grid_right_cnt)
                )
                | std::views::transform([](const auto & p) noexcept {
                    const auto &[grid_down, grid_right] { p };
                    return N { node_layer::RIGHT, grid_down, grid_right };
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

        std::ranges::bidirectional_range auto get_edges() const {
            auto diagonal_layer_edges {
                std::views::cartesian_product(
                    std::array<node_layer, 3zu> { node_layer::DIAGONAL, node_layer::DOWN, node_layer::RIGHT },
                    std::views::iota(0u, grid_down_cnt),
                    std::views::iota(0u, grid_right_cnt)
                )
                | std::views::transform([](const auto& tuple) {
                    const auto& [n2_layer, n1_grid_down, n1_grid_right] { tuple };
                    const N n1 { node_layer::DIAGONAL, n1_grid_down, n1_grid_right };
                    if (n2_layer == node_layer::DIAGONAL) {
                        return E { n1, N { n2_layer, n1_grid_down + 1u, n1_grid_right + 1u } };
                    } else if (n2_layer == node_layer::DOWN) {
                        return E { n1, N { n2_layer, n1_grid_down + 1u, n1_grid_right } };
                    } else if (n2_layer == node_layer::RIGHT) {
                        return E { n1, N { n2_layer, n1_grid_down, n1_grid_right + 1u } };
                    }
                    std::unreachable();
                })
                | std::views::filter([this](const E& edge) {
                    return has_edge(edge);
                })
            };
            static_assert(std::ranges::bidirectional_range<decltype(diagonal_layer_edges)>);
            auto down_layer_edges {
                std::views::cartesian_product(
                    std::array<node_layer, 2zu> { node_layer::DIAGONAL, node_layer::DOWN },
                    std::views::iota(1u, grid_down_cnt),
                    std::views::iota(0u, grid_right_cnt)
                )
                | std::views::transform([](const auto& tuple) {
                    const auto& [n2_layer, n1_grid_down, n1_grid_right] { tuple };
                    const N n1 { node_layer::DOWN, n1_grid_down, n1_grid_right };
                    if (n2_layer == node_layer::DIAGONAL) {
                        return E { n1, N { n2_layer, n1_grid_down, n1_grid_right } };
                    } else if (n2_layer == node_layer::DOWN) {
                        return E { n1, N { n2_layer, n1_grid_down + 1u, n1_grid_right } };
                    }
                    std::unreachable();
                })
                | std::views::filter([this](const E& edge) {
                    return has_edge(edge);
                })
            };
            static_assert(std::ranges::bidirectional_range<decltype(down_layer_edges)>);
            auto right_layer_edges {
                std::views::cartesian_product(
                    std::array<node_layer, 2zu> { node_layer::DIAGONAL, node_layer::RIGHT },
                    std::views::iota(0u, grid_down_cnt),
                    std::views::iota(1u, grid_right_cnt)
                )
                | std::views::transform([](const auto& tuple) {
                    const auto& [n2_layer, n1_grid_down, n1_grid_right] { tuple };
                    const N n1 { node_layer::RIGHT, n1_grid_down, n1_grid_right };
                    if (n2_layer == node_layer::DIAGONAL) {
                        return E { n1, N { n2_layer, n1_grid_down, n1_grid_right } };
                    } else if (n2_layer == node_layer::RIGHT) {
                        return E { n1, N { n2_layer, n1_grid_down, n1_grid_right + 1u } };
                    }
                    std::unreachable();
                })
                | std::views::filter([this](const E& edge) {
                    return has_edge(edge);
                })
            };
            static_assert(std::ranges::bidirectional_range<decltype(right_layer_edges)>);
            return concat_view(
                std::move(diagonal_layer_edges),
                concat_view(
                    std::move(down_layer_edges),
                    std::move(right_layer_edges)
                )
            );
        }

        bool has_node(const N& node) const {
            const auto& [n_layer, grid_down, grid_right] { node };
            return (n_layer == node_layer::DIAGONAL
                    && grid_down < grid_down_cnt && grid_down >= 0u
                    && grid_right < grid_right_cnt && grid_right >= 0u)
                || (n_layer == node_layer::DOWN
                    && grid_down < grid_down_cnt && grid_down >= 1u
                    && grid_right < grid_right_cnt && grid_right >= 0u)
                || (n_layer == node_layer::RIGHT
                    && grid_down < grid_down_cnt && grid_down >= 0u
                    && grid_right < grid_right_cnt && grid_right >= 1u);
        }

        bool has_edge(const E& edge) const {
            const auto& [n1_layer, n1_grid_down, n1_grid_right] { edge.source };
            const auto& [n2_layer, n2_grid_down, n2_grid_right] { edge.destination };
            return (n1_layer == node_layer::DIAGONAL && n2_layer == node_layer::DIAGONAL
                    && n1_grid_down + 1u == n2_grid_down && n1_grid_right + 1u == n2_grid_right
                    && n2_grid_down < grid_down_cnt && n2_grid_right < grid_right_cnt)  // match
                || (n1_layer == node_layer::DIAGONAL && n2_layer == node_layer::DOWN
                    && n1_grid_down + 1u == n2_grid_down && n1_grid_right == n2_grid_right
                    && n2_grid_down < grid_down_cnt && n2_grid_right < grid_right_cnt)  // initial gap (down)
                || (n1_layer == node_layer::DIAGONAL && n2_layer == node_layer::RIGHT
                    && n1_grid_down == n2_grid_down && n1_grid_right + 1u == n2_grid_right
                    && n2_grid_down < grid_down_cnt && n2_grid_right < grid_right_cnt) // initial gap (right)
                || (n1_layer == node_layer::DOWN && n2_layer == node_layer::DOWN
                    && n1_grid_down > 0u && n1_grid_down + 1u == n2_grid_down && n1_grid_right == n2_grid_right
                    && n2_grid_down < grid_down_cnt && n2_grid_right < grid_right_cnt)  // extended gap (down)
                || (n1_layer == node_layer::RIGHT && n2_layer == node_layer::RIGHT
                    && n1_grid_right > 0u && n1_grid_down == n2_grid_down && n1_grid_right + 1u == n2_grid_right
                    && n2_grid_down < grid_down_cnt && n2_grid_right < grid_right_cnt) // extended gap (right)
                || (n1_layer == node_layer::DOWN && n2_layer == node_layer::DIAGONAL
                    && n1_grid_down > 0u && n1_grid_down == n2_grid_down && n1_grid_right == n2_grid_right
                    && n2_grid_down < grid_down_cnt && n2_grid_right < grid_right_cnt)  // freeride (down)
                || (n1_layer == node_layer::RIGHT && n2_layer == node_layer::DIAGONAL
                    && n1_grid_right > 0u && n1_grid_down == n2_grid_down && n1_grid_right == n2_grid_right
                    && n2_grid_down < grid_down_cnt && n2_grid_right < grid_right_cnt); // freeride (right)
        }

        auto get_outputs_full(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            typename static_vector_typer<debug_mode, std::tuple<E, N, N, ED>, 3u>::type ret {};
            const auto& [n1_layer, n1_grid_down, n1_grid_right] { node };
            if (n1_layer == node_layer::DIAGONAL) {
                if (n1_grid_down == grid_down_cnt - 1u && n1_grid_right == grid_right_cnt - 1u) {
                    // do nothing
                } else if (n1_grid_down < grid_down_cnt - 1u && n1_grid_right < grid_right_cnt - 1u) {
                    ret.push_back(construct_full_edge(node, { node_layer::DIAGONAL, n1_grid_down + 1u, n1_grid_right + 1u }));
                    ret.push_back(construct_full_edge(node, { node_layer::DOWN, n1_grid_down + 1u, n1_grid_right }));
                    ret.push_back(construct_full_edge(node, { node_layer::RIGHT, n1_grid_down, n1_grid_right + 1u }));
                } else if (n1_grid_right == grid_right_cnt - 1u) {
                    ret.push_back(construct_full_edge(node, { node_layer::DOWN, n1_grid_down + 1u, n1_grid_right }));
                } else if (n1_grid_down == grid_down_cnt - 1u) {
                    ret.push_back(construct_full_edge(node, { node_layer::RIGHT, n1_grid_down, n1_grid_right + 1u }));
                }
            } else if (n1_layer == node_layer::DOWN) {
                ret.push_back(construct_full_edge(node, { node_layer::DIAGONAL, n1_grid_down, n1_grid_right }));
                if (n1_grid_down < grid_down_cnt - 1u) {
                    ret.push_back(construct_full_edge(node, { node_layer::DOWN, n1_grid_down + 1u, n1_grid_right }));
                }
            } else if (n1_layer == node_layer::RIGHT) {
                ret.push_back(construct_full_edge(node, { node_layer::DIAGONAL, n1_grid_down, n1_grid_right }));
                if (n1_grid_right < grid_right_cnt - 1u) {
                    ret.push_back(construct_full_edge(node, { node_layer::RIGHT, n1_grid_down, n1_grid_right + 1u }));
                }
            }
            return ret;
        }

        auto get_inputs_full(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            typename static_vector_typer<debug_mode, std::tuple<E, N, N, ED>, 3u>::type ret {};
            const auto& [n2_layer, n2_grid_down, n2_grid_right] { node };
            if (n2_layer == node_layer::DIAGONAL) {
                if (n2_grid_down == 0u && n2_grid_right == 0u) {
                    // do nothing
                } else if (n2_grid_down > 0u && n2_grid_right > 0u) {
                    ret.push_back(construct_full_edge({ node_layer::DIAGONAL, n2_grid_down - 1u, n2_grid_right - 1u }, node));
                    ret.push_back(construct_full_edge({ node_layer::DOWN, n2_grid_down, n2_grid_right }, node));
                    ret.push_back(construct_full_edge({ node_layer::RIGHT, n2_grid_down, n2_grid_right }, node));
                } else if (n2_grid_right > 0u) {
                    ret.push_back(construct_full_edge({ node_layer::RIGHT, n2_grid_down, n2_grid_right }, node));
                } else if (n2_grid_down > 0u) {
                    ret.push_back(construct_full_edge({ node_layer::DOWN, n2_grid_down, n2_grid_right }, node));
                }
            } else if (n2_layer == node_layer::DOWN) {
                if (n2_grid_down > 0u) {
                    ret.push_back(construct_full_edge({ node_layer::DIAGONAL, n2_grid_down - 1u, n2_grid_right }, node));
                }
                if (n2_grid_down > 1u) {
                    ret.push_back(construct_full_edge({ node_layer::DOWN, n2_grid_down - 1u, n2_grid_right }, node));
                }
            } else if (n2_layer == node_layer::RIGHT) {
                if (n2_grid_right > 0u) {
                    ret.push_back(construct_full_edge({ node_layer::DIAGONAL, n2_grid_down, n2_grid_right - 1u }, node));
                }
                if (n2_grid_right > 1u) {
                    ret.push_back(construct_full_edge({ node_layer::RIGHT, n2_grid_down, n2_grid_right - 1u }, node));
                }
            }
            return ret;
        }

        auto get_outputs(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return this->get_outputs_full(node) | std::views::transform([this](const auto& v) noexcept -> E { return std::get<0>(v); });
        }

        auto get_inputs(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return this->get_inputs_full(node) | std::views::transform([this](const auto& v) noexcept -> E { return std::get<0>(v); });
        }

        bool has_outputs(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return this->get_outputs(node).size() > 0zu;
        }

        bool has_inputs(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return this->get_inputs(node).size() > 0zu;
        }

        std::size_t get_out_degree(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return this->get_outputs(node).size();
        }

        std::size_t get_in_degree(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return this->get_inputs(node).size();
        }

        auto edge_to_element_offsets(
            const E& edge
        ) const {
            using OPT_INDEX = std::optional<INDEX>;
            using RET = std::optional<std::pair<OPT_INDEX, OPT_INDEX>>;

            const auto& [n1, n2] { edge };
            const auto& [n1_layer, n1_grid_down, n1_grid_right] { n1 };
            const auto& [n2_layer, n2_grid_down, n2_grid_right] { n2 };
            if (n1_layer == node_layer::DIAGONAL && n2_layer == node_layer::DIAGONAL) {  // match
                if (n1_grid_down + 1u == n2_grid_down && n1_grid_right + 1u == n2_grid_right) {
                    return RET { { { n1_grid_down }, { n1_grid_right } } };
                }
            } else if ((n1_layer == node_layer::DOWN && n2_layer == node_layer::DOWN)  // extended indel
                || (n1_layer == node_layer::DIAGONAL && n2_layer == node_layer::DOWN)) {  // indel
                if (n1_grid_down + 1u == n2_grid_down && n1_grid_right == n2_grid_right) {
                    return RET { { { n1_grid_down }, std::nullopt } };
                }
            } else if ((n1_layer == node_layer::RIGHT && n2_layer == node_layer::RIGHT)  // extended indel
                || (n1_layer == node_layer::DIAGONAL && n2_layer == node_layer::RIGHT)) {  // indel
                if (n1_grid_down == n2_grid_down && n1_grid_right + 1u == n2_grid_right) {
                    return RET { { std::nullopt, { n1_grid_right } } };
                }
            } else if ((n1_layer == node_layer::DOWN && n2_layer == node_layer::DIAGONAL)  // freeride
                || (n1_layer == node_layer::RIGHT && n2_layer == node_layer::DIAGONAL)) {  // freeride
                return RET { std::nullopt };
            }
            if constexpr (debug_mode) {
                throw std::runtime_error("Bad edge");
            }
            std::unreachable();
        }

        std::tuple<INDEX, INDEX, std::size_t> node_to_grid_offsets(const N& node) const {
            const auto& [layer, down_offset, right_offset] { node };
            return { down_offset, right_offset, static_cast<std::size_t>(layer) };
        }

    private:
        static N slice_next_node_(const N& node) {
            const auto& [layer_, grid_down, grid_right] { node };
            N next_node;
            if (grid_down == 0u && grid_right == 0u && layer_ == node_layer::DIAGONAL) {
                next_node = { node_layer::RIGHT, grid_down, grid_right + 1u };
            } else if (grid_down == 0u && layer_ == node_layer::RIGHT) {
                next_node = { node_layer::DIAGONAL, grid_down, grid_right };
            } else if (grid_down == 0u && layer_ == node_layer::DIAGONAL) {
                next_node = { node_layer::RIGHT, grid_down, grid_right + 1u };
            } else if (grid_right == 0u && layer_ == node_layer::DOWN) {
                next_node = { node_layer::DIAGONAL, grid_down, grid_right };
            } else if (grid_right == 0u && layer_ == node_layer::DIAGONAL) {
                next_node = { node_layer::DOWN, grid_down, grid_right + 1u };
            } else if (layer_ == node_layer::DOWN) {
                next_node = { node_layer::RIGHT, grid_down, grid_right };
            } else if (layer_ == node_layer::RIGHT) {
                next_node = { node_layer::DIAGONAL, grid_down, grid_right };
            } else if (layer_ == node_layer::DIAGONAL) {
                next_node = { node_layer::DOWN, grid_down, grid_right + 1u };
            } else {
                if constexpr (debug_mode) {
                    throw std::runtime_error("This should never happen");
                }
            }
            return next_node;
        }

        static N slice_prev_node_(const N& node) {
            const auto& [layer_, grid_down, grid_right] { node };
            N prev_node;
            if (grid_down == 0u && grid_right == 1u && layer_ == node_layer::RIGHT) {
                prev_node = { node_layer::DIAGONAL, grid_down, grid_right - 1u };
            } else if (grid_down == 0u && layer_ == node_layer::DIAGONAL) {
                prev_node = { node_layer::RIGHT, grid_down, grid_right };
            } else if (grid_down == 0u && layer_ == node_layer::RIGHT) {
                prev_node = { node_layer::DIAGONAL, grid_down, grid_right - 1u };
            } else if (grid_right == 0u && layer_ == node_layer::DIAGONAL) {
                prev_node = { node_layer::DOWN, grid_down, grid_right };
            } else if (grid_right == 0u && layer_ == node_layer::DOWN) {
                prev_node = { node_layer::DIAGONAL, grid_down, grid_right - 1u };
            } else if (layer_ == node_layer::DOWN) {
                prev_node = { node_layer::DIAGONAL, grid_down, grid_right - 1u };
            } else if (layer_ == node_layer::RIGHT) {
                prev_node = { node_layer::DOWN, grid_down, grid_right };
            } else if (layer_ == node_layer::DIAGONAL) {
                prev_node = { node_layer::RIGHT, grid_down, grid_right };
            } else {
                if constexpr (debug_mode) {
                    throw std::runtime_error("This should never happen");
                }
            }
            return prev_node;
        }

    public:
        auto slice_nodes(INDEX grid_down) const {
            return slice_nodes(grid_down, get_root_node(), get_leaf_node());
        }

        auto slice_nodes(INDEX grid_down, const N& root_node, const N& leaf_node) const {
            if constexpr (debug_mode) {
                if (!has_node(root_node) || !has_node(leaf_node)) {
                    throw std::runtime_error("Bad node");
                }
                if (!(root_node.down <= leaf_node.down)) {
                    throw std::runtime_error("Bad node");
                }
                if (!(root_node.right <= leaf_node.right)) {
                    throw std::runtime_error("Bad node");
                }
                // if single node in graph, make sure depth order is satisifed same: DOWN RIGHT DIAGONAL
                if (
                    (root_node.down == leaf_node.down)
                    && (root_node.right == leaf_node.right)
                    && !(root_node.layer <= leaf_node.layer)
                ) {
                    throw std::runtime_error("Bad node");
                }
                if (!(grid_down >= root_node.down && grid_down <= leaf_node.down)) {
                    throw std::runtime_error("Bad node");
                }
            }

            // order of nodes:
            //    d   Rd   Rd
            //   Dd  DRd  DRd
            //   Dd  DRd  DRd
            const auto& [root_layer, root_down, root_right] { root_node };
            N begin_node;
            {
                bool walking_first_row { grid_down == root_down };
                if (walking_first_row) {
                    begin_node = root_node;
                } else {
                    if (grid_down == 0u && root_right == 0u) {
                        begin_node = { node_layer::DIAGONAL, grid_down, root_right };
                    } else {
                        begin_node = { node_layer::DOWN, grid_down, root_right };
                    }
                }
            }

            const auto& [leaf_layer, leaf_down, leaf_right] { leaf_node };
            N end_node;
            {
                bool walking_last_row { grid_down == leaf_down };
                if (walking_last_row) {
                    end_node = leaf_node;
                } else {
                    end_node = { node_layer::DIAGONAL, grid_down, leaf_right };
                }
                end_node = slice_next_node_(end_node);  // +1, because iterator.end() should be 1 past the last element
            }

            struct state {
                N value_;

                void to_prev() { value_ = slice_prev_node_(value_); }
                void to_next() { value_ = slice_next_node_(value_); }
                N value() const { return value_; }

                bool operator==(const state& other) const = default;
            };
            return simple_value_bidirectional_view<state> {
                state { begin_node },
                state { end_node }
            };
        }

        bool is_reachable(const N& n1, const N& n2) const {
            if constexpr (debug_mode) {
                if (!has_node(n1) || !has_node(n2)) {
                    throw std::runtime_error("Bad node");
                }
            }

            const auto& [n1_layer, n1_down, n1_right] { n1 };
            const auto& [n2_layer, n2_down, n2_right] { n2 };
            if (n1_layer == n2_layer && n1_down == n2_down && n1_right == n2_right) {
                return true;
            } else if (n1_down == n2_down && n1_right == n2_right) {
                return n2_layer == node_layer::DIAGONAL;
            } else if (n1_down > n2_down || n1_right > n2_right) [[unlikely]] {
                return false;
            } else if (n1_right == n2_right) {
                return n2_layer != node_layer::RIGHT;
            } else if (n1_down == n2_down) {
                return n2_layer != node_layer::DOWN;
            } else {
                return true;
            }
        }

        auto resident_nodes() const {
            return std::views::empty<N>;
        }

        auto outputs_to_residents(const N& node) const {
            return std::views::empty<E>;
        }

        auto inputs_from_residents(const N& node) const {
            return std::views::empty<E>;
        }
    };
}

// Struct must be defined outside of namespace block above, otherwise compiler will treat it as part of that namespace.
template<offbynull::concepts::widenable_to_size_t INDEX>
struct std::formatter<offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::node<INDEX>> : std::formatter<std::string> {
    auto format(const offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::node<INDEX>& n, std::format_context& ctx) const {
        const char* layer_str;
        switch (n.layer) {
            case offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::node_layer::DOWN:
                layer_str = "v";
                break;
            case offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::node_layer::RIGHT:
                layer_str = ">";
                break;
            case offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::node_layer::DIAGONAL:
                layer_str = "\\";
                break;
            default:
                std::unreachable();
        }
        return std::format_to(ctx.out(), "[{},{},{}]", n.down, n.right, layer_str);
    }
};

template<offbynull::concepts::widenable_to_size_t INDEX>
std::ostream& operator<<(std::ostream& os, const offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::node<INDEX>& n) {
    return os << std::format("{}", n);
}

// Struct must be defined outside of namespace block above, otherwise compiler will treat it as part of that namespace.
template<offbynull::concepts::widenable_to_size_t INDEX>
struct std::formatter<offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::edge<INDEX>> : std::formatter<std::string> {
    auto format(const offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::edge<INDEX>& e, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{}->{}", e.source, e.destination);
    }
};

template<offbynull::concepts::widenable_to_size_t INDEX>
std::ostream& operator<<(std::ostream& os, const offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::edge<INDEX>& e) {
    return os << std::format("{}", e);
}

#endif //OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_EXTENDED_GAP_ALIGNMENT_GRAPH_H
