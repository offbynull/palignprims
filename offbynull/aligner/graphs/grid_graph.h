#ifndef OFFBYNULL_ALIGNER_GRAPHS_GRID_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_GRID_GRAPH_H

#include <cstddef>
#include <ranges>
#include <tuple>
#include <stdexcept>
#include <utility>
#include <stdfloat>

#include "offbynull/concepts.h"
#include "offbynull/utils.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/sequence/sequence.h"

namespace offbynull::aligner::graphs::grid_graph {
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::utils::static_vector_typer;

    using empty_type = std::tuple<>;

    template<
        bool debug_mode,
        sequence DOWN_SEQ,
        sequence RIGHT_SEQ,
        widenable_to_size_t INDEX_ = std::size_t,
        weight WEIGHT = std::float64_t
    >
    class grid_graph {
    public:
        using DOWN_ELEM = std::decay_t<decltype(std::declval<DOWN_SEQ>()[0u])>;
        using RIGHT_ELEM = std::decay_t<decltype(std::declval<RIGHT_SEQ>()[0u])>;
        using INDEX = INDEX_;
        using N = std::pair<INDEX, INDEX>;
        using ND = empty_type;
        using E = std::pair<N, N>;
        using ED = WEIGHT;  // Differs from backing grid_graph because these values are derived at time of access

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
        > gap_lookup;

        auto construct_full_edge(N n1, N n2) const {
            return std::tuple<E, N, N, ED> {
                E { n1, n2 },
                n1,
                n2,
                this->get_edge_data(
                    E { n1, n2 }
                )
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
        static constexpr INDEX grid_depth_cnt { 1u };
        static constexpr std::size_t resident_nodes_capacity { 0zu };
        const std::size_t path_edge_capacity;

        grid_graph(
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
            > _gap_lookup
        )
        : down_seq{_down_seq}
        , right_seq{_right_seq}
        , substitution_lookup{_substitution_lookup}
        , gap_lookup{_gap_lookup}
        , grid_down_cnt{_down_seq.size() + 1zu}
        , grid_right_cnt{_right_seq.size() + 1zu}
        , path_edge_capacity{(grid_right_cnt - 1u) + (grid_down_cnt - 1u)} {}

        ND get_node_data(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return {};
        }

        ED get_edge_data(const E& edge) const  {
            if constexpr (debug_mode) {
                if (!has_edge(edge)) {
                    throw std::runtime_error("Edge doesn't exist");
                }
            }
            const auto& [n1_grid_down, n1_grid_right] { edge.first };
            const auto& [n2_grid_down, n2_grid_right] { edge.second };
            if (n1_grid_down == n2_grid_down && n1_grid_right + 1u == n2_grid_right) {
                return gap_lookup(
                    edge,
                    { std::nullopt },
                    { { right_seq[n1_grid_right] } }
                );
            } else if (n1_grid_down + 1u == n2_grid_down && n1_grid_right == n2_grid_right) {
                return gap_lookup(
                    edge,
                    { { down_seq[n1_grid_down] } },
                    { std::nullopt }
                );
            } else if (n1_grid_down + 1u == n2_grid_down && n1_grid_right + 1u == n2_grid_right) {
                return substitution_lookup(
                    edge,
                    { { down_seq[n1_grid_down] } },
                    { { right_seq[n1_grid_right] } }
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
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            return edge.first;
        }

        N get_edge_to(const E& edge) const {
            if constexpr (debug_mode) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            return edge.second;
        }

        std::tuple<N, N, ED> get_edge(const E& edge) const {
            if constexpr (debug_mode) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            return std::tuple<N, N, ED> {this->get_edge_from(edge), this->get_edge_to(edge), this->get_edge_data(edge)};
        }

        auto get_root_nodes() const {
            return std::ranges::single_view { N { 0u, 0u } };
        }

        N get_root_node() const {
            return N { 0u, 0u };
        }

        auto get_leaf_nodes() const {
            return std::ranges::single_view { N { grid_down_cnt - 1u, grid_right_cnt - 1u } };
        }

        N get_leaf_node() const {
            return N { grid_down_cnt - 1u, grid_right_cnt - 1u };
        }

        auto get_nodes() const {
            return
                std::views::cartesian_product(
                    std::views::iota(0u, grid_down_cnt),
                    std::views::iota(0u, grid_right_cnt)
                )
                | std::views::transform([](const auto & p) {
                    return N { std::get<0>(p), std::get<1>(p) };
                });
        }

        std::ranges::bidirectional_range auto get_edges() const {
            return
                std::views::cartesian_product(
                    std::views::iota(0u, grid_down_cnt),
                    std::views::iota(0u, grid_right_cnt),
                    std::views::iota(static_cast<INDEX>(0), static_cast<INDEX>(2)),
                    std::views::iota(static_cast<INDEX>(0), static_cast<INDEX>(2))
                )
                | std::views::filter([](const auto& tuple) {
                    const auto& [grid_down_idx, grid_right_idx, down_offset, right_offset] { tuple };
                    return !(down_offset == 0u && right_offset == 0u);
                })
                | std::views::transform([](const auto& tuple) {
                    const auto& [grid_down_idx, grid_right_idx, down_offset, right_offset] { tuple };
                    return E {
                        N { grid_down_idx, grid_right_idx },
                        N { grid_down_idx + down_offset, grid_right_idx + right_offset }
                    };
                })
                | std::views::filter([this](const E& edge) {
                    return has_edge(edge);
                });
        }

        bool has_node(const N& node) const {
            const auto& [n1_grid_down, n1_grid_right] { node };
            return n1_grid_down < grid_down_cnt && n1_grid_down >= 0u && n1_grid_right < grid_right_cnt && n1_grid_right >= 0u;
        }

        bool has_edge(const E& edge) const {
            const auto& [n1_grid_down, n1_grid_right] { edge.first };
            const auto& [n2_grid_down, n2_grid_right] { edge.second };
            return (n1_grid_down == n2_grid_down && n1_grid_right + 1u == n2_grid_right && n2_grid_right < grid_right_cnt)
                || (n1_grid_down + 1u == n2_grid_down && n1_grid_right == n2_grid_right && n2_grid_down < grid_down_cnt)
                || (n1_grid_down + 1u == n2_grid_down && n1_grid_right + 1u == n2_grid_right && n2_grid_down < grid_down_cnt && n2_grid_right < grid_right_cnt);
        }

        std::ranges::bidirectional_range auto get_outputs_full(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            // Cartesian product has some issues with bloat, so not using it here:
            return
                std::views::cartesian_product(
                    std::views::iota(static_cast<INDEX>(0), static_cast<INDEX>(2)),
                    std::views::iota(static_cast<INDEX>(0), static_cast<INDEX>(2))
                )
                | std::views::drop(1)
                | std::views::filter([node = node, this](const auto& offset) {
                    const auto& [down_offset, right_offset] { offset };
                    const auto& [grid_down, grid_right] { node };
                    if (down_offset == 1u && grid_down == grid_down_cnt - 1u) {
                        return false;
                    }
                    if (right_offset == 1u && grid_right == grid_right_cnt - 1u) {
                        return false;
                    }
                    return true;
                })
                | std::views::transform([node = node, this](const auto& offset) {
                    const auto& [down_offset, right_offset] { offset };
                    const auto& [grid_down, grid_right] { node };
                    N n2 { grid_down + down_offset, grid_right + right_offset };
                    return this->construct_full_edge(node, n2);
                });
        }

        auto get_inputs_full(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return
                std::views::cartesian_product(
                    std::views::iota(static_cast<INDEX>(0), static_cast<INDEX>(2)),
                    std::views::iota(static_cast<INDEX>(0), static_cast<INDEX>(2))
                )
                | std::views::drop(1)
                | std::views::filter([node = node, this](const auto& offset) {
                    const auto& [down_offset, right_offset] { offset };
                    const auto& [grid_down, grid_right] { node };
                    if (down_offset == 1u && grid_down == 0u) {
                        return false;
                    }
                    if (right_offset == 1u && grid_right == 0u) {
                        return false;
                    }
                    return true;
                })
                | std::views::transform([node = node, this](const auto& offset) {
                    const auto& [down_offset, right_offset] { offset };
                    const auto& [grid_down, grid_right] { node };
                    N n1 { grid_down - down_offset, grid_right - right_offset };
                    return this->construct_full_edge(n1, node);
                });
        }

        std::ranges::bidirectional_range auto get_outputs(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_outputs_full(node)
                | std::views::transform([](const auto& v) -> E { return std::get<0>(v); });
        }

        auto get_inputs(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_inputs_full(node)
                | std::views::transform([](const auto& v) -> E { return std::get<0>(v); });
        }

        bool has_outputs(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_out_degree(node) > 0zu;
        }

        bool has_inputs(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_in_degree(node) > 0zu;
        }

        std::size_t get_out_degree(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto outputs { this->get_outputs(node) };
            auto dist { std::ranges::distance(outputs) };
            return static_cast<std::size_t>(dist);
        }

        std::size_t get_in_degree(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto inputs { this->get_inputs(node) };
            auto dist { std::ranges::distance(inputs) };
            return static_cast<std::size_t>(dist);
        }

        std::tuple<INDEX, INDEX, std::size_t> node_to_grid_offsets(const N& node) const {
            const auto& [down_offset, right_offset] { node };
            return { down_offset, right_offset, 0zu };
        }

        auto slice_nodes(INDEX grid_down) const {
            return slice_nodes(grid_down, get_root_node(), get_leaf_node());
        }

        auto slice_nodes(INDEX grid_down, const N& root_node, const N& leaf_node) const {
            if constexpr (debug_mode) {
                if (!has_node(root_node) || !has_node(leaf_node)) {
                    throw std::runtime_error("Bad node");
                }
                if (!(root_node <= leaf_node)) {
                    throw std::runtime_error("Bad node");
                }
                if (!(grid_down >= std::get<0>(root_node) && grid_down <= std::get<0>(leaf_node))) {
                    throw std::runtime_error("Bad node");
                }
            }
            return std::views::iota(std::get<1>(root_node), std::get<1>(leaf_node) + 1u)
                | std::views::transform([grid_down](const auto& grid_right) { return N { grid_down, grid_right }; });
        }

        bool is_reachable(const N& n1, const N& n2) const {
            if constexpr (debug_mode) {
                if (!has_node(n1) || !has_node(n2)) {
                    throw std::runtime_error("Bad node");
                }
            }
            return n1 <= n2;
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
#endif //OFFBYNULL_ALIGNER_GRAPHS_GRID_GRAPH_H
