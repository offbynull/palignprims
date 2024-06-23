#ifndef OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_LOCAL_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_LOCAL_ALIGNMENT_GRAPH_H


#include <ranges>
#include <tuple>
#include <stdexcept>
#include <format>
#include <utility>
#include <functional>
#include <stdfloat>
#include "offbynull/aligner/graphs/grid_graph.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/concepts.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::graphs::pairwise_local_alignment_graph {
    using offbynull::aligner::graphs::grid_graph::grid_graph;
    using offbynull::aligner::graphs::grid_graph::empty_type;
    using offbynull::aligner::concepts::weight;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::utils::concat_view;
    using offbynull::utils::pair_counter_view;
    using offbynull::utils::static_vector_typer;

    enum class edge_type : uint8_t {
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
        std::ranges::random_access_range DOWN_SEQ,
        std::ranges::random_access_range RIGHT_SEQ,
        widenable_to_size_t INDEX_ = std::size_t,
        weight WEIGHT = std::float64_t,
        bool error_check = true
    >
    class pairwise_local_alignment_graph {
    public:
        using INDEX = INDEX_;
        using N = std::pair<INDEX, INDEX>;
        using E = edge<INDEX>;
        using ND = empty_type;
        using ED = WEIGHT;  // Differs from backing grid_graph because these values are derived at time of access

    private:
        grid_graph<
            DOWN_SEQ,
            RIGHT_SEQ,
            INDEX,
            WEIGHT,
            error_check
        > g;
        std::function<
            WEIGHT(
                const E& edge
            )
        > freeride_lookup;

    public:
        const INDEX grid_down_cnt;
        const INDEX grid_right_cnt;

        pairwise_local_alignment_graph(
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
            > _indel_lookup,
            std::function<
                WEIGHT(
                    const E&
                )
            > _freeride_lookup
        )
        : g{
            _down_seq,
            _right_seq,
            [_match_lookup](
                const typename decltype(g)::E& edge,
                const std::decay_t<decltype(_down_seq[0u])>& down_elem,
                const std::decay_t<decltype(_right_seq[0u])>& right_elem
            ) {
               return _match_lookup(
                   {edge_type::NORMAL, edge},
                   down_elem,
                   right_elem
                );
            },
            [_indel_lookup] (
                const typename decltype(g)::E& edge
            ) {
                return _indel_lookup(
                    {edge_type::NORMAL, edge}
                 );
            }
        }
        , freeride_lookup{_freeride_lookup}
        , grid_down_cnt{g.grid_down_cnt}
        , grid_right_cnt{g.grid_right_cnt} {}

        ND get_node_data(const N& node) {
            return g.get_node_data(node);
        }

        ED get_edge_data(const E& edge) {
            return std::get<2>(get_edge(edge));
        }

        N get_edge_from(const E& edge) {
            return std::get<0>(get_edge(edge));
        }

        N get_edge_to(const E& edge) {
            return std::get<1>(get_edge(edge));
        }

        std::tuple<N, N, ED> get_edge(const E& edge) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            if (edge.type == edge_type::FREE_RIDE) {
                const auto& [n1, n2] { edge.inner_edge };
                return std::tuple<N, N, ED> {n1, n2, freeride_lookup(edge)};
            } else {
                return g.get_edge(edge.inner_edge);
            }
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

        N get_leaf_node() {
            return g.get_leaf_node();
        }

        auto get_nodes() {
            return g.get_nodes();
        }

        auto get_edges() {
            auto from_src_range {
                std::views::cartesian_product(
                    std::views::iota(0u, g.grid_down_cnt),
                    std::views::iota(0u, g.grid_right_cnt)
                )
                | std::views::drop(1) // drop 0,0
                | std::views::transform([&](const auto & p) noexcept {
                    N n1 { 0u, 0u };
                    N n2 { std::get<0>(p), std::get<1>(p) };
                    return E { edge_type::FREE_RIDE, { n1, n2 } };
                })
            };
            auto to_sink_range {
                std::views::cartesian_product(
                    std::views::iota(0u, g.grid_down_cnt),
                    std::views::iota(0u, g.grid_right_cnt)
                )
                | ( std::views::reverse | std::views::drop(1) | std::views::reverse ) // drop bottom right
                | std::views::transform([&](const auto & p) noexcept {
                    N n1 { std::get<0>(p), std::get<1>(p) };
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
            // This should be using std::views::conat, but it wasn't included in this version of the C++ standard
            // library. The concat implementation below lacks several features (e.g. doesn't support the pipe operator)
            // and forcefully returns copies (concat_view::iterator::value_type ==
            // concat_view::iterator::reference_type).
            return concat_view {
                std::move(real_range),
                concat_view { from_src_range, to_sink_range }
            };
        }

        bool has_node(const N& node) {
            return g.has_node(node);
        }

        bool has_edge(const E& edge) {
            if (edge.type == edge_type::NORMAL) {
                return g.has_edge(edge.inner_edge);
            } else {
                const auto & [n1, n2] { edge.inner_edge };
                const auto & [n1_grid_down, n1_grid_right] { n1 };
                const auto & [n2_grid_down, n2_grid_right] { n2 };
                if (n1_grid_down == 0u && n1_grid_right == 0u) {
                    if (n2_grid_down == 0u && n2_grid_right == 0u) {
                        return false;
                    } else if (n2_grid_down <= g.grid_down_cnt - 1u && n2_grid_right <= g.grid_right_cnt - 1u) {
                        return true;
                    } else {
                        return false;
                    }
                } else if (n2_grid_down == g.grid_down_cnt - 1u && n2_grid_right == g.grid_right_cnt - 1u) {
                    if (n1_grid_down == g.grid_down_cnt - 1u && n1_grid_right == g.grid_right_cnt - 1u) {
                        return false;
                    } else if (n1_grid_down <= g.grid_down_cnt - 1u && n1_grid_right <= g.grid_right_cnt - 1u) {
                        return true;
                    } else {
                        return false;
                    }
                } else {
                    return false;
                }
            }
        }

        auto get_outputs_full(const N& node) {
            auto standard_outputs {
                g.get_outputs_full(node)
                | std::views::transform([this](const auto& raw_full_edge) noexcept {
                    N n1 { std::get<1>(raw_full_edge) };
                    N n2 { std::get<2>(raw_full_edge) };
                    E e { edge_type::NORMAL, { n1, n2 } };
                    return std::tuple<E, N, N, ED> {e, n1, n2, freeride_lookup(e)};
                })
            };
            bool has_freeride_to_leaf { node != get_leaf_node() };
            auto freeride_set_1 {
                std::views::single(get_leaf_node())
                | std::views::transform([node, this](const N& n2) noexcept {
                    N n1 { node };
                    E e { edge_type::FREE_RIDE, { n1, n2 } };
                    return std::tuple<E, N, N, ED> {e, n1, n2, freeride_lookup(e)};
                })
                | std::views::filter([has_freeride_to_leaf](const auto&) noexcept {
                    return has_freeride_to_leaf;
                })
            };
            bool has_freeride_from_root { node == get_root_node() };
            auto freeride_set_2 {
                pair_counter_view {
                    grid_down_cnt,
                    grid_right_cnt
                }
                | std::views::take(grid_down_cnt * grid_right_cnt - 1u)  // Remove leaf (will be added by non_leaf_only_outputs)
                | std::views::drop(1u)  // Remove root
                | std::views::transform([this](const N& n2) {
                    N n1 { 0, 0 };
                    E e { edge_type::FREE_RIDE, { n1, n2 } };
                    return std::tuple<E, N, N, ED> {e, n1, n2, freeride_lookup(e)};
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

        auto get_inputs_full(const N& node) {
            auto standard_inputs {
                g.get_inputs_full(node)
                | std::views::transform([this](const auto& raw_full_edge) {
                    N n1 { std::get<1>(raw_full_edge) };
                    N n2 { std::get<2>(raw_full_edge) };
                    E e { edge_type::NORMAL, { n1, n2 } };
                    return std::tuple<E, N, N, ED> {e, n1, n2, freeride_lookup(e)};
                })
            };
            bool has_freeride_to_leaf { node == get_leaf_node() };
            auto freeride_set_1 {
                pair_counter_view {
                    grid_down_cnt,
                    grid_right_cnt
                }
                | std::views::take(grid_down_cnt * grid_right_cnt - 1u)  // Remove leaf
                | std::views::drop(1u)  // Remove root (will be added by non_root_only_inputs)
                | std::views::transform([this](const N& n1) {
                    N n2 { grid_down_cnt - 1u, grid_right_cnt - 1u };
                    E e { edge_type::FREE_RIDE, { n1, n2 } };
                    return std::tuple<E, N, N, ED> {e, n1, n2, freeride_lookup(e)};
                })
                | std::views::filter([has_freeride_to_leaf](const auto&) {
                    return has_freeride_to_leaf;
                })
            };
            bool has_freeride_from_root { node != get_root_node() };
            auto freeride_set_2 {
                std::views::single(get_root_node())
                | std::views::transform([node, this](const N& n1) {
                    N n2 { node };
                    E e { edge_type::FREE_RIDE, { n1, n2 } };
                    return std::tuple<E, N, N, ED> {e, n1, n2, freeride_lookup(e)};
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

        auto get_outputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return get_outputs_full(node)
                | std::views::transform([this](auto v) -> E { return std::get<0>(v); });
        }

        auto get_inputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_inputs_full(node)
                | std::views::transform([this](auto v) -> E { return std::get<0>(v); });
        }

        bool has_outputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto outputs { this->get_outputs(node) };
            return outputs.begin() != outputs.end();
        }

        bool has_inputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto inputs { this->get_inputs(node) };
            return inputs.begin() != inputs.end();
        }

        std::size_t get_out_degree(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto outputs { std::ranges::common_view { this->get_outputs(node) } };
            auto dist { std::distance(outputs.begin(), outputs.end()) };
            return static_cast<std::size_t>(dist);
        }

        std::size_t get_in_degree(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto inputs { std::ranges::common_view { this->get_inputs(node) } };
            auto dist { std::distance(inputs.begin(), inputs.end()) };
            return static_cast<std::size_t>(dist);
        }

        auto edge_to_element_offsets(
            const E& edge
        ) {
            using OPT_INDEX = std::optional<INDEX>;
            using RET = std::optional<std::pair<OPT_INDEX, OPT_INDEX>>;

            if (edge.type == edge_type::FREE_RIDE) {
                return RET { std::nullopt };  // Returning nullopt directly means a conversion to RET happens behind the scene, and that makes the concept check fail.
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
            return g.node_to_grid_offsets(node);
        }

        constexpr static INDEX node_count(
            INDEX _grid_down_cnt,
            INDEX _grid_right_cnt
        ) {
            return decltype(g)::node_count(_grid_down_cnt, _grid_right_cnt);
        }

        constexpr static INDEX longest_path_edge_count(
            INDEX _grid_down_cnt,
            INDEX _grid_right_cnt
        ) {
            return decltype(g)::longest_path_edge_count(_grid_down_cnt, _grid_right_cnt);
        }

        constexpr static std::size_t slice_nodes_capacity(INDEX _grid_down_cnt, INDEX _grid_right_cnt) {
            return decltype(g)::slice_nodes_capacity(_grid_down_cnt, _grid_right_cnt);
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
            return 2zu;
        }

        auto resident_nodes() {
            return std::array<N, 2u> { g.get_root_node(), g.get_leaf_node() };
        }

        auto outputs_to_residents(const N& node) {
            using CONTAINER = static_vector_typer<E, 2zu, error_check>::type;
            CONTAINER ret {};
            const N& leaf_node { get_leaf_node() };
            if (node != leaf_node) {
                ret.push_back(E { edge_type::FREE_RIDE, { node, leaf_node } });
            }
            const auto& [leaf_grid_down, leaf_grid_right] { leaf_node };
            const auto& [grid_down, grid_right] { node };
            if ((grid_down + 1u == leaf_grid_down && grid_right + 1u == leaf_grid_right)
                    || (grid_down == leaf_grid_down && grid_right + 1u == leaf_grid_right)
                    || (grid_down + 1u == leaf_grid_down && grid_right == leaf_grid_right)) {
                ret.push_back(E { edge_type::NORMAL, { node, leaf_node } });
            }
            return ret;
        }

        auto inputs_from_residents(const N& node) {
            using CONTAINER = static_vector_typer<E, 2zu, error_check>::type;
            CONTAINER ret {};
            const N& root_node { get_root_node() };
            if (node != root_node) {
                ret.push_back(E { edge_type::FREE_RIDE, { root_node, node } });
            }
            const auto& [root_grid_down, root_grid_right] { root_node };
            const auto& [grid_down, grid_right] { node };
            if ((grid_down - 1u == root_grid_down && grid_right - 1u == root_grid_right)
                    || (grid_down - 1u == root_grid_down && grid_right == root_grid_right)
                    || (grid_down == root_grid_down && grid_right - 1u == root_grid_right)) {
                ret.push_back(E { edge_type::NORMAL, { root_node, node } });
            }
            return ret;
        }
    };
}
#endif //OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_LOCAL_ALIGNMENT_GRAPH_H
