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
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/concepts.h"
#include "offbynull/helpers/concat_view.h"
#include "offbynull/aligner/graph/utils.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph {
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::helpers::concat_view::concat_view;
    using offbynull::utils::static_vector_typer;
    using offbynull::aligner::graph::utils::generic_slicable_pairwise_alignment_graph_limits;

    using empty_type = std::tuple<>;

    enum class layer : std::uint8_t {
        DOWN,
        RIGHT,
        DIAGONAL  // Make sure this is last, because, while in the same grid offset, this is the layer that the other two feed into. Algorithms expect this.
    };

    template<
        sequence DOWN_SEQ,
        sequence RIGHT_SEQ,
        widenable_to_size_t INDEX_ = std::size_t,
        weight WEIGHT = std::float64_t,
        bool error_check = true
    >
    class pairwise_extended_gap_alignment_graph {
    public:
        using DOWN_ELEM = std::decay_t<decltype(std::declval<DOWN_SEQ>()[0u])>;
        using RIGHT_ELEM = std::decay_t<decltype(std::declval<RIGHT_SEQ>()[0u])>;
        using INDEX = INDEX_;
        using N = std::tuple<layer, INDEX, INDEX>;
        using ND = empty_type;
        using E = std::pair<N, N>;
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
        : down_seq{_down_seq}
        , right_seq{_right_seq}
        , substitution_lookup{_substitution_lookup}
        , initial_gap_lookup{_initial_gap_lookup}
        , extended_gap_lookup{_extended_gap_lookup}
        , freeride_lookup{_freeride_lookup}
        , grid_down_cnt{_down_seq.size() + 1zu}
        , grid_right_cnt{_right_seq.size() + 1zu} {}

        ND get_node_data(const N& node) const {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return {};
        }

        ED get_edge_data(const E& edge) const {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            const auto& [n1_layer, n1_grid_down, n1_grid_right] { edge.first };
            const auto& [n2_layer, n2_grid_down, n2_grid_right] { edge.second };
            if (n1_layer == layer::DIAGONAL && n2_layer == layer::DIAGONAL) {  // match
                return substitution_lookup(
                    edge,
                    { { down_seq[n1_grid_down] } },
                    { { right_seq[n1_grid_right] } }
                );
            } else if (n1_layer == layer::DOWN && n2_layer == layer::DOWN) {  // gap
                return extended_gap_lookup(
                    edge,
                    { { down_seq[n1_grid_down] } },
                    { std::nullopt }
                );
            } else if (n1_layer == layer::RIGHT && n2_layer == layer::RIGHT) {  // gap
                return extended_gap_lookup(
                    edge,
                    { std::nullopt },
                    { { right_seq[n1_grid_right] } }
                );
            } else if (n1_layer == layer::DIAGONAL && n2_layer == layer::DOWN) {  // indel
                return initial_gap_lookup(
                    edge,
                    { { down_seq[n1_grid_down] } },
                    { std::nullopt }
                );
            } else if (n1_layer == layer::DIAGONAL && n2_layer == layer::RIGHT) {  // indel
                return initial_gap_lookup(
                    edge,
                    { std::nullopt },
                    { { right_seq[n1_grid_right] } }
                );
            } else if (n1_layer == layer::DOWN && n2_layer == layer::DIAGONAL) {  // freeride
                return freeride_lookup(
                    edge,
                    { std::nullopt },
                    { std::nullopt }
                );
            } else if (n1_layer == layer::RIGHT && n2_layer == layer::DIAGONAL) {  // freeride
                return freeride_lookup(
                    edge,
                    { std::nullopt },
                    { std::nullopt }
                );
            }
            if constexpr (error_check) {
                throw std::runtime_error("Bad edge");
            }
            std::unreachable();
        }

        N get_edge_from(const E& edge) const {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            return edge.first;
        }

        N get_edge_to(const E& edge) const {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            return edge.second;
        }

        std::tuple<N, N, ED> get_edge(const E& edge) const {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            return std::tuple<N, N, ED> {get_edge_from(edge), get_edge_from(edge), get_edge_data(edge)};
        }

        auto get_root_nodes() const {
            return std::ranges::single_view { N { layer::DIAGONAL, 0u, 0u } };
        }

        N get_root_node() const {
            return N { layer::DIAGONAL, 0u, 0u };
        }

        auto get_leaf_nodes() const {
            return std::ranges::single_view { N { layer::DIAGONAL, grid_down_cnt - 1u, grid_right_cnt - 1u } };
        }

        N get_leaf_node() const {
            return N { layer::DIAGONAL, grid_down_cnt - 1u, grid_right_cnt - 1u };
        }

        auto get_nodes() const {
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

        std::ranges::bidirectional_range auto get_edges() const {
            auto diagonal_layer_edges {
                std::views::cartesian_product(
                    std::array<layer, 3zu> { layer::DIAGONAL, layer::DOWN, layer::RIGHT },
                    std::views::iota(0u, grid_down_cnt),
                    std::views::iota(0u, grid_right_cnt)
                )
                | std::views::transform([](const auto& tuple) {
                    const auto& [n2_layer, n1_grid_down, n1_grid_right] { tuple };
                    const N n1 { layer::DIAGONAL, n1_grid_down, n1_grid_right };
                    if (n2_layer == layer::DIAGONAL) {
                        return E { n1, N { n2_layer, n1_grid_down + 1u, n1_grid_right + 1u } };
                    } else if (n2_layer == layer::DOWN) {
                        return E { n1, N { n2_layer, n1_grid_down + 1u, n1_grid_right } };
                    } else if (n2_layer == layer::RIGHT) {
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
                    std::array<layer, 2zu> { layer::DIAGONAL, layer::DOWN },
                    std::views::iota(1u, grid_down_cnt),
                    std::views::iota(0u, grid_right_cnt)
                )
                | std::views::transform([](const auto& tuple) {
                    const auto& [n2_layer, n1_grid_down, n1_grid_right] { tuple };
                    const N n1 { layer::DOWN, n1_grid_down, n1_grid_right };
                    if (n2_layer == layer::DIAGONAL) {
                        return E { n1, N { n2_layer, n1_grid_down, n1_grid_right } };
                    } else if (n2_layer == layer::DOWN) {
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
                    std::array<layer, 2zu> { layer::DIAGONAL, layer::RIGHT },
                    std::views::iota(0u, grid_down_cnt),
                    std::views::iota(1u, grid_right_cnt)
                )
                | std::views::transform([](const auto& tuple) {
                    const auto& [n2_layer, n1_grid_down, n1_grid_right] { tuple };
                    const N n1 { layer::RIGHT, n1_grid_down, n1_grid_right };
                    if (n2_layer == layer::DIAGONAL) {
                        return E { n1, N { n2_layer, n1_grid_down, n1_grid_right } };
                    } else if (n2_layer == layer::RIGHT) {
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
            return (n_layer == layer::DIAGONAL && grid_down < grid_down_cnt && grid_down >= 0u && grid_right < grid_right_cnt && grid_right >= 0u)
                || (n_layer == layer::DOWN && grid_down < grid_down_cnt && grid_down >= 1u && grid_right < grid_right_cnt && grid_right >= 0u)
                || (n_layer == layer::RIGHT && grid_down < grid_down_cnt && grid_down >= 0u && grid_right < grid_right_cnt && grid_right >= 1u);
        }

        bool has_edge(const E& edge) const {
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

        auto get_outputs_full(const N& node) const {
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

        auto get_inputs_full(const N& node) const {
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

        auto get_outputs(const N& node) const {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_outputs_full(node) | std::views::transform([this](const auto& v) noexcept -> E { return std::get<0>(v); });
        }

        auto get_inputs(const N& node) const {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_inputs_full(node) | std::views::transform([this](const auto& v) noexcept -> E { return std::get<0>(v); });
        }

        bool has_outputs(const N& node) const {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_outputs(node).size() > 0zu;
        }

        bool has_inputs(const N& node) const {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_inputs(node).size() > 0zu;
        }

        std::size_t get_out_degree(const N& node) const {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_outputs(node).size();
        }

        std::size_t get_in_degree(const N& node) const {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
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

        std::tuple<INDEX, INDEX, std::size_t> node_to_grid_offsets(const N& node) const {
            const auto& [layer, down_offset, right_offset] { node };
            return { down_offset, right_offset, static_cast<std::size_t>(layer) };
        }

        constexpr static auto limits(
            INDEX _grid_down_cnt,
            INDEX _grid_right_cnt
        ) {
            std::size_t max_node_cnt {};
            max_node_cnt += _grid_down_cnt * _grid_right_cnt; // Middle layer
            max_node_cnt += (_grid_down_cnt - 1u) * _grid_right_cnt; // Down gap layer
            max_node_cnt += _grid_down_cnt * (_grid_right_cnt - 1u); // Right gap layer
            std::size_t max_node_depth { 3zu };
            std::size_t max_path_edge_cnt { (_grid_right_cnt - 1zu) * 2zu + (_grid_down_cnt - 1zu) * 2zu };
            std::size_t max_slice_nodes_cnt { 1zu + 3zu * (_grid_right_cnt - 1zu) }; // THIS IS NOT SUPPOSED TO BE THE CAPACITY AT EACH SLICE, this is supposed to be the max capacity across all slices, meaning the implementation below is correct.
            std::size_t max_resident_nodes_cnt { 0zu };
            return generic_slicable_pairwise_alignment_graph_limits {
                max_node_cnt,
                max_node_depth,
                max_path_edge_cnt,
                max_slice_nodes_cnt,
                max_resident_nodes_cnt
            };
        }

        auto slice_nodes(INDEX grid_down) const {
            return slice_nodes(grid_down, grid_right_cnt);
        }

        std::ranges::bidirectional_range auto slice_nodes(INDEX grid_down, INDEX override_grid_right_cnt) const {
            return
                std::views::cartesian_product(
                    std::views::iota(1u, override_grid_right_cnt),
                    std::array<layer, 3zu> { layer::DIAGONAL, layer::RIGHT, layer::DOWN }
                )
                | std::views::filter(
                    [grid_down](const auto& tuple) {
                        const auto& [grid_right, layer] { tuple };
                        return layer == layer::DIAGONAL
                            || (grid_down != 0u && grid_right != 0u && (layer == layer::DOWN || layer == layer::RIGHT));
                    }
                )
                | std::views::transform(
                    [grid_down](const auto& tuple) {
                        const auto& [grid_right, layer] { tuple };
                        return N { layer::DIAGONAL, grid_down, grid_right };
                    }
                );
        }

        N slice_first_node(INDEX grid_down) const {
            return slice_first_node(grid_down, 0u);
        }

        N slice_first_node(INDEX grid_down, INDEX grid_right) const {
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

        N slice_last_node(INDEX grid_down) const {
            return slice_last_node(grid_down, grid_right_cnt - 1u);
        }

        N slice_last_node(INDEX grid_down, INDEX grid_right) const {
            N last_node { layer::DOWN, grid_down, grid_right };
            if constexpr (error_check) {
                if (std::get<1>(last_node) >= grid_down_cnt) {
                    throw std::runtime_error("Node too far down");
                }
            }
            return last_node;
        }

        N slice_next_node(const N& node) const {
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

        N slice_prev_node(const N& node) const {
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
#endif //OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_EXTENDED_GAP_ALIGNMENT_GRAPH_H
