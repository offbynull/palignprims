#ifndef OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_FITTING_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_FITTING_ALIGNMENT_GRAPH_H

#include <cstdint>
#include <cstddef>
#include <ranges>
#include <tuple>
#include <stdexcept>
#include <utility>
#include <functional>
#include <stdfloat>
#include "offbynull/aligner/graphs/grid_graph.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/concepts.h"
#include "offbynull/helpers/concat_view.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::graphs::pairwise_fitting_alignment_graph {
    using offbynull::aligner::graphs::grid_graph::grid_graph;
    using offbynull::aligner::graphs::grid_graph::empty_type;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::helpers::concat_view::concat_view;
    using offbynull::utils::static_vector_typer;

    enum class edge_type : std::uint8_t {
        FREE_RIDE,
        NORMAL
    };

    template<widenable_to_size_t T>
    class edge {
    public:
        using N = std::pair<T, T>;
        edge_type type;
        std::pair<N, N> inner_edge;

        std::strong_ordering operator<=>(const edge& rhs) const = default;
    };

    template<
        bool debug_mode,
        sequence DOWN_SEQ,
        sequence RIGHT_SEQ,
        widenable_to_size_t INDEX_ = std::size_t,
        weight WEIGHT = std::float64_t
    >
    class pairwise_fitting_alignment_graph {
    public:
        using DOWN_ELEM = std::decay_t<decltype(std::declval<DOWN_SEQ>()[0u])>;
        using RIGHT_ELEM = std::decay_t<decltype(std::declval<RIGHT_SEQ>()[0u])>;
        using INDEX = INDEX_;
        using N = std::pair<INDEX, INDEX>;
        using E = edge<INDEX>;
        using ND = empty_type;
        using ED = WEIGHT;  // Differs from backing grid_graph because these values are derived at time of access

    private:
        const grid_graph<
            debug_mode,
            DOWN_SEQ,
            RIGHT_SEQ,
            INDEX,
            WEIGHT
        > g;
        const std::function<
            WEIGHT(
                const E&,
                const std::optional<std::reference_wrapper<const DOWN_ELEM>>,
                const std::optional<std::reference_wrapper<const RIGHT_ELEM>>
            )
        > freeride_lookup;

    public:
        const INDEX grid_down_cnt;
        const INDEX grid_right_cnt;
        static constexpr INDEX grid_depth_cnt { decltype(g)::grid_depth_cnt };
        const std::size_t max_resident_nodes_cnt;
        const std::size_t max_path_edge_cnt;

        pairwise_fitting_alignment_graph(
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
            > _gap_lookup,
            const std::function<
                WEIGHT(
                    const E&,
                    const std::optional<std::reference_wrapper<const DOWN_ELEM>>,
                    const std::optional<std::reference_wrapper<const RIGHT_ELEM>>
                )
            > _freeride_lookup
        )
        : g{
            _down_seq,
            _right_seq,
            [_substitution_lookup](
                const typename decltype(g)::E& edge,
                const std::optional<std::reference_wrapper<const DOWN_ELEM>> down_elem,
                const std::optional<std::reference_wrapper<const RIGHT_ELEM>> right_elem
            ) {
                return _substitution_lookup(
                    {edge_type::NORMAL, edge},
                    down_elem,
                    right_elem
                 );
            },
            [_gap_lookup] (
                const typename decltype(g)::E& edge,
                const std::optional<std::reference_wrapper<const DOWN_ELEM>> down_elem,
                const std::optional<std::reference_wrapper<const RIGHT_ELEM>> right_elem
            ) {
                return _gap_lookup(
                    {edge_type::NORMAL, edge},
                    down_elem,
                    right_elem
                 );
            }
        }
        , freeride_lookup{_freeride_lookup}
        , grid_down_cnt{g.grid_down_cnt}
        , grid_right_cnt{g.grid_right_cnt}
        , max_resident_nodes_cnt{2zu}
        , max_path_edge_cnt{g.max_path_edge_cnt} {}

        ND get_node_data(const N& node) const {
            return g.get_node_data(node);
        }

        ED get_edge_data(const E& edge) const {
            return std::get<2>(get_edge(edge));
        }

        N get_edge_from(const E& edge) const {
            return std::get<0>(get_edge(edge));
        }

        N get_edge_to(const E& edge) const {
            return std::get<1>(get_edge(edge));
        }

        std::tuple<N, N, ED> get_edge(const E& edge) const {
            if constexpr (debug_mode) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            if (edge.type == edge_type::FREE_RIDE) {
                const auto& [n1, n2] { edge.inner_edge };
                return std::tuple<N, N, ED> {n1, n2, freeride_lookup(edge, { std::nullopt }, { std::nullopt })};
            } else {
                return g.get_edge(edge.inner_edge);
            }
        }

        auto get_root_nodes() const {
            return g.get_root_nodes();
        }

        N get_root_node() const {
            return g.get_root_node();
        }

        auto get_leaf_nodes() const {
            return g.get_leaf_nodes();
        }

        N get_leaf_node() const {
            return g.get_leaf_node();
        }

        auto get_nodes() const {
            return g.get_nodes();
        }

        auto get_edges() const {
            auto from_src_range {
                std::views::iota(0u, g.grid_down_cnt)
                | std::views::drop(1) // drop 0
                | std::views::transform([&](const auto & grid_down) noexcept {
                    N n1 { 0u, 0u };
                    N n2 { grid_down, 0u };
                    return E { edge_type::FREE_RIDE, { n1, n2 } };
                })
            };
            auto to_sink_range {
                std::views::iota(0u, g.grid_down_cnt)
                | ( std::views::reverse | std::views::drop(1) | std::views::reverse ) // drop last
                | std::views::transform([&](const auto & grid_down) noexcept {
                    N n1 { grid_down, g.grid_right_cnt - 1u };
                    N n2 { g.grid_down_cnt - 1u, g.grid_right_cnt - 1u };
                    return E { edge_type::FREE_RIDE, { n1, n2 } };
                })
            };
            auto real_range {
                g.get_edges()
                | std::views::transform([&](const auto & p) noexcept {
                    return E { edge_type::NORMAL, p };
                })
            };
            return concat_view {
                std::move(real_range),
                concat_view {
                    std::move(from_src_range),
                    std::move(to_sink_range)
                }
            };
        }

        bool has_node(const N& node) const {
            return g.has_node(node);
        }

        bool has_edge(const E& edge) const {
            if (edge.type == edge_type::NORMAL) {
                return g.has_edge(edge.inner_edge);
            } else {
                const auto & [n1, n2] { edge.inner_edge };
                const auto & [n1_grid_down, n1_grid_right] { n1 };
                const auto & [n2_grid_down, n2_grid_right] { n2 };
                if (n1_grid_down == 0u && n1_grid_right == 0u) {
                    if (n2_grid_down == 0u && n2_grid_right == 0u) {
                        return false;
                    } else if (n2_grid_down <= g.grid_down_cnt - 1u && n2_grid_right == 0u) {
                        return true;
                    } else {
                        return false;
                    }
                } else if (n2_grid_down == g.grid_down_cnt - 1u && n2_grid_right == g.grid_right_cnt - 1u) {
                    if (n1_grid_down == g.grid_down_cnt - 1u && n1_grid_right == g.grid_right_cnt - 1u) {
                        return false;
                    } else if (n1_grid_down <= g.grid_down_cnt - 1u && n1_grid_right == g.grid_right_cnt - 1u) {
                        return true;
                    } else {
                        return false;
                    }
                } else {
                    return false;
                }
            }
        }

        auto get_outputs_full(const N& node) const {
            auto standard_outputs {
                g.get_outputs_full(node)
                | std::views::transform([this](const auto& raw_full_edge) noexcept {
                    N n1 { std::get<1>(raw_full_edge) };
                    N n2 { std::get<2>(raw_full_edge) };
                    E e { edge_type::NORMAL, { n1, n2 } };
                    return std::tuple<E, N, N, ED> {e, n1, n2, freeride_lookup(e, { std::nullopt }, { std::nullopt })};
                })
            };
            bool has_freeride_to_leaf { std::get<0>(node) < grid_down_cnt - 1u && std::get<1>(node) == grid_right_cnt - 1u };
            auto freeride_set_1 {
                std::views::single(get_leaf_node())
                | std::views::transform([node, this](const N& n2) noexcept {
                    N n1 { node };
                    E e { edge_type::FREE_RIDE, { n1, n2 } };
                    return std::tuple<E, N, N, ED> {e, n1, n2, freeride_lookup(e, { std::nullopt }, { std::nullopt })};
                })
                | std::views::filter([has_freeride_to_leaf](const auto&) noexcept {
                    return has_freeride_to_leaf;
                })
            };
            bool has_freeride_from_root { node == get_root_node() };
            auto freeride_set_2 {
                std::views::iota(1u, grid_down_cnt)
                | std::views::transform([this](const INDEX& grid_down) {
                    return N { grid_down, 0u };
                })
                | std::views::transform([this](const N& n2) {
                    N n1 { 0, 0 };
                    E e { edge_type::FREE_RIDE, { n1, n2 } };
                    return std::tuple<E, N, N, ED> {e, n1, n2, freeride_lookup(e, { std::nullopt }, { std::nullopt })};
                })
                | std::views::filter([has_freeride_from_root](const auto&) {
                    return has_freeride_from_root;
                })
            };
            return concat_view {
                std::move(standard_outputs),
                concat_view {
                    std::move(freeride_set_1),
                    std::move(freeride_set_2)
                }
            };
        }

        auto get_inputs_full(const N& node) const {
            auto standard_inputs {
                g.get_inputs_full(node)
                | std::views::transform([this](const auto& raw_full_edge) {
                    N n1 { std::get<1>(raw_full_edge) };
                    N n2 { std::get<2>(raw_full_edge) };
                    E e { edge_type::NORMAL, { n1, n2 } };
                    return std::tuple<E, N, N, ED> {e, n1, n2, freeride_lookup(e, { std::nullopt }, { std::nullopt })};
                })
            };
            bool has_freeride_to_leaf { node == get_leaf_node() };
            auto freeride_set_1 {
                std::views::iota(0u, grid_down_cnt - 1u)
                | std::views::transform([this](const INDEX& grid_down) {
                    return N { grid_down, grid_right_cnt - 1u };
                })
                | std::views::transform([this](const N& n1) {
                    N n2 { get_leaf_node() };
                    E e { edge_type::FREE_RIDE, { n1, n2 } };
                    return std::tuple<E, N, N, ED> {e, n1, n2, freeride_lookup(e, { std::nullopt }, { std::nullopt })};
                })
                | std::views::filter([has_freeride_to_leaf](const auto&) {
                    return has_freeride_to_leaf;
                })
            };
            bool has_freeride_from_root { std::get<0>(node) > 0u && std::get<1>(node) == 0u };
            auto freeride_set_2 {
                std::views::single(get_root_node())
                | std::views::transform([node, this](const N& n1) {
                    N n2 { node };
                    E e { edge_type::FREE_RIDE, { n1, n2 } };
                    return std::tuple<E, N, N, ED> {e, n1, n2, freeride_lookup(e, { std::nullopt }, { std::nullopt })};
                })
                | std::views::filter([has_freeride_from_root](const auto&) {
                    return has_freeride_from_root;
                })
            };
            return concat_view {
                std::move(standard_inputs),
                concat_view {
                    std::move(freeride_set_1),
                    std::move(freeride_set_2)
                }
            };
        }

        auto get_outputs(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return get_outputs_full(node)
                | std::views::transform([this](auto v) -> E { return std::get<0>(v); });
        }

        auto get_inputs(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_inputs_full(node)
                | std::views::transform([this](auto v) -> E { return std::get<0>(v); });
        }

        bool has_outputs(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto outputs { this->get_outputs(node) };
            return outputs.begin() != outputs.end();
        }

        bool has_inputs(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto inputs { this->get_inputs(node) };
            return inputs.begin() != inputs.end();
        }

        std::size_t get_out_degree(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto outputs { std::ranges::common_view { this->get_outputs(node) } };
            auto dist { std::ranges::distance(outputs) };
            return static_cast<std::size_t>(dist);
        }

        std::size_t get_in_degree(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto inputs { std::ranges::common_view { this->get_inputs(node) } };
            auto dist { std::ranges::distance(inputs) };
            return static_cast<std::size_t>(dist);
        }

        auto edge_to_element_offsets(
            const E& edge
        ) const {
            using OPT_INDEX = std::optional<INDEX>;
            using RET = std::optional<std::pair<OPT_INDEX, OPT_INDEX>>;

            if (edge.type == edge_type::FREE_RIDE) {
                return RET { std::nullopt };
            }
            const auto& [n1, n2] {edge.inner_edge};
            const auto& [n1_grid_down, n1_grid_right] {n1};
            const auto& [n2_grid_down, n2_grid_right] {n2};
            if (n1_grid_down + 1u == n2_grid_down && n1_grid_right + 1u == n2_grid_right) {
                return RET { { { n1_grid_down }, { n1_grid_right } } };
            } else if (n1_grid_down + 1u == n2_grid_down && n1_grid_right == n2_grid_right) {
                return RET { { { n1_grid_down }, std::nullopt } };
            } else if (n1_grid_down == n2_grid_down && n1_grid_right + 1u == n2_grid_right) {
                return RET { { std::nullopt, { n1_grid_right } } };
            }
            if constexpr (debug_mode) {
                throw std::runtime_error("Bad edge");
            }
            std::unreachable();
        }

        std::tuple<INDEX, INDEX, std::size_t> node_to_grid_offsets(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return g.node_to_grid_offsets(node);
        }

        auto slice_nodes(INDEX grid_down) const {
            return g.slice_nodes(grid_down);
        }

        auto slice_nodes(INDEX grid_down, const N& root_node, const N& leaf_node) const {
            return g.slice_nodes(grid_down, root_node, leaf_node);
        }

        bool is_reachable(const N& n1, const N& n2) const {
            return g.is_reachable(n1, n2);
        }

        auto resident_nodes() const {
            return std::array<N, 2u> { g.get_root_node(), g.get_leaf_node() };
        }

        auto outputs_to_residents(const N& node) const {
            using CONTAINER = static_vector_typer<E, 2zu, debug_mode>::type;
            CONTAINER ret {};
            const N& leaf_node { get_leaf_node() };
            const auto& [leaf_grid_down, leaf_grid_right] { leaf_node };
            const auto& [grid_down, grid_right] { node };
            if (grid_down < leaf_grid_down && grid_right == leaf_grid_right) {
                ret.push_back(E { edge_type::FREE_RIDE, { node, leaf_node } });
            }
            if ((grid_down + 1u == leaf_grid_down && grid_right + 1u == leaf_grid_right)
                    || (grid_down == leaf_grid_down && grid_right + 1u == leaf_grid_right)
                    || (grid_down + 1u == leaf_grid_down && grid_right == leaf_grid_right)) {
                ret.push_back(E { edge_type::NORMAL, { node, leaf_node } });
            }
            return ret;
        }

        auto inputs_from_residents(const N& node) const {
            using CONTAINER = static_vector_typer<E, 2zu, debug_mode>::type;
            CONTAINER ret {};
            const N& root_node { get_root_node() };
            const auto& [root_grid_down, root_grid_right] { root_node };
            const auto& [grid_down, grid_right] { node };
            if (grid_down > root_grid_down && grid_right == root_grid_right) {
                ret.push_back(E { edge_type::FREE_RIDE, { root_node, node } });
            }
            if ((grid_down - 1u == root_grid_down && grid_right - 1u == root_grid_right)
                    || (grid_down - 1u == root_grid_down && grid_right == root_grid_right)
                    || (grid_down == root_grid_down && grid_right - 1u == root_grid_right)) {
                ret.push_back(E { edge_type::NORMAL, { root_node, node } });
            }
            return ret;
        }
    };
}
#endif //OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_FITTING_ALIGNMENT_GRAPH_H
