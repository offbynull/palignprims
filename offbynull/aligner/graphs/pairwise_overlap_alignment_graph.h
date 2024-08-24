#ifndef OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_OVERLAP_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_OVERLAP_ALIGNMENT_GRAPH_H

#include <compare>
#include <cstdint>
#include <cstddef>
#include <ranges>
#include <tuple>
#include <stdexcept>
#include <utility>
#include <optional>
#include <functional>
#include <string>
#include <string_view>
#include <array>
#include <format>
#include <type_traits>
#include <ostream>
#include "offbynull/aligner/graphs/grid_graph.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/scorer/scorer.h"
#include "offbynull/concepts.h"
#include "offbynull/helpers/concat_view.h"
#include "offbynull/helpers/blankable_bidirectional_view.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::graphs::pairwise_overlap_alignment_graph {
    using offbynull::aligner::graphs::grid_graph::grid_graph;
    using offbynull::aligner::graphs::grid_graph::empty_type;
    using offbynull::aligner::graphs::grid_graph::edge;
    using offbynull::aligner::graphs::grid_graph::node;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::scorer::scorer::scorer;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::helpers::concat_view::concat_view;
    using offbynull::helpers::blankable_bidirectional_view::blankable_bidirectional_view;
    using offbynull::utils::static_vector_typer;

    enum class edge_type : std::uint8_t {
        FREE_RIDE,
        NORMAL
    };

    template<widenable_to_size_t INDEX>
    class overlap_edge {
    public:
        edge_type type;
        edge<INDEX> inner_edge;
        std::strong_ordering operator<=>(const overlap_edge& rhs) const = default;
    };

    // A scorer that translates calls from the grid_graph backing the pairwise_local_alignment_graph, such that the edge type becomes the
    // edge type of pairwise_local_alignment_graph.
    template<
        bool debug_mode,
        widenable_to_size_t INDEX,
        typename DOWN_ELEM,
        typename RIGHT_ELEM,
        typename WEIGHT,
        scorer<
            overlap_edge<INDEX>,
            DOWN_ELEM,
            RIGHT_ELEM,
            WEIGHT
        > GRID_GRAPH_SCORER
    >
    class grid_scorer_to_overlap_scorer_proxy {
    private:
        GRID_GRAPH_SCORER grid_graph_scorer;
    public:
        grid_scorer_to_overlap_scorer_proxy(
            const GRID_GRAPH_SCORER& grid_graph_scorer_
        )
        : grid_graph_scorer { grid_graph_scorer_ } {}

        WEIGHT operator()(
            const edge<INDEX>& edge,
            const std::optional<std::reference_wrapper<const DOWN_ELEM>> down_elem_ref_opt,
            const std::optional<std::reference_wrapper<const RIGHT_ELEM>> right_elem_ref_opt
        ) const {
            return grid_graph_scorer(
                overlap_edge<INDEX> { edge_type::NORMAL, edge },
                down_elem_ref_opt,
                right_elem_ref_opt
            );
        }
    };

    template<
        bool debug_mode,
        widenable_to_size_t INDEX_,
        weight WEIGHT,
        sequence DOWN_SEQ,
        sequence RIGHT_SEQ,
        scorer<
            overlap_edge<INDEX_>,
            std::decay_t<decltype(std::declval<DOWN_SEQ>()[0zu])>,
            std::decay_t<decltype(std::declval<RIGHT_SEQ>()[0zu])>,
            WEIGHT
        > SUBSTITUTION_SCORER,
        scorer<
            overlap_edge<INDEX_>,
            std::decay_t<decltype(std::declval<DOWN_SEQ>()[0zu])>,
            std::decay_t<decltype(std::declval<RIGHT_SEQ>()[0zu])>,
            WEIGHT
        > GAP_SCORER,
        scorer<
            overlap_edge<INDEX_>,
            std::decay_t<decltype(std::declval<DOWN_SEQ>()[0zu])>,
            std::decay_t<decltype(std::declval<RIGHT_SEQ>()[0zu])>,
            WEIGHT
        > FREERIDE_SCORER
    >
    class pairwise_overlap_alignment_graph {
    public:
        using DOWN_ELEM = std::decay_t<decltype(std::declval<DOWN_SEQ>()[0u])>;
        using RIGHT_ELEM = std::decay_t<decltype(std::declval<RIGHT_SEQ>()[0u])>;
        using INDEX = INDEX_;
        using N = node<INDEX>;
        using E = overlap_edge<INDEX>;
        using ND = empty_type;
        using ED = WEIGHT;

    private:
        const grid_graph<
            debug_mode,
            INDEX_,
            WEIGHT,
            DOWN_SEQ,
            RIGHT_SEQ,
            grid_scorer_to_overlap_scorer_proxy<debug_mode, INDEX_, DOWN_ELEM, RIGHT_ELEM, WEIGHT, SUBSTITUTION_SCORER>,
            grid_scorer_to_overlap_scorer_proxy<debug_mode, INDEX_, DOWN_ELEM, RIGHT_ELEM, WEIGHT, GAP_SCORER>
        > g;
        const FREERIDE_SCORER freeride_scorer;

    public:
        const INDEX grid_down_cnt;
        const INDEX grid_right_cnt;
        static constexpr INDEX grid_depth_cnt { decltype(g)::grid_depth_cnt };
        const std::size_t resident_nodes_capacity;
        const std::size_t path_edge_capacity;

        // Scorer params are not being made into universal references because there's a high chance of enabling a subtle bug: There's a
        // non-trivial possibility that the user will submit the same object for both scorers, and so if the universal reference ends up
        // being an rvalue reference it'll try to move the same object twice.
        pairwise_overlap_alignment_graph(
            const DOWN_SEQ& _down_seq,
            const RIGHT_SEQ& _right_seq,
            const SUBSTITUTION_SCORER _substitution_scorer,
            const GAP_SCORER _gap_scorer,
            const FREERIDE_SCORER _freeride_scorer
        )
        : g {
            _down_seq,
            _right_seq,
            { _substitution_scorer } /* grid_scorer_to_local_scorer_proxy */,
            { _gap_scorer } /* grid_scorer_to_local_scorer_proxy */
        }
        , freeride_scorer { _freeride_scorer }
        , grid_down_cnt { g.grid_down_cnt }
        , grid_right_cnt { g.grid_right_cnt }
        , resident_nodes_capacity { 2zu }
        , path_edge_capacity { g.path_edge_capacity } {}

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
                    throw std::runtime_error { "Edge doesn't exist" };
                }
            }
            if (edge.type == edge_type::FREE_RIDE) {
                const auto& [n1, n2] { edge.inner_edge };
                return std::tuple<N, N, ED> { n1, n2, freeride_scorer(edge, { std::nullopt }, { std::nullopt }) };
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
                std::views::iota(0u, g.grid_right_cnt)
                | ( std::views::reverse | std::views::drop(1) | std::views::reverse ) // drop last
                | std::views::transform([&](const auto & grid_right) noexcept {
                    N n1 { g.grid_down_cnt - 1u, grid_right };
                    N n2 { g.grid_down_cnt - 1u, g.grid_right_cnt - 1u };
                    return E { edge_type::FREE_RIDE, { n1, n2 } };
                })
            };
            auto real_range {
                g.get_edges()
                | std::views::transform([&](const edge<INDEX>& e) noexcept {
                    return E { edge_type::NORMAL, e };
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
                    } else if (n1_grid_down == g.grid_down_cnt - 1u && n1_grid_right <= g.grid_right_cnt - 1u) {
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
                    return std::tuple<E, N, N, ED> { e, n1, n2, freeride_scorer(e, { std::nullopt }, { std::nullopt }) };
                })
            };
            bool has_freeride_to_leaf { node.down == grid_down_cnt - 1u && node.right < grid_right_cnt - 1u };
            blankable_bidirectional_view freeride_set_1 {
                has_freeride_to_leaf,  // passthru if condition is met, otherwise blank
                std::views::single(get_leaf_node())
                | std::views::transform([node, this](const N& n2) noexcept {
                    N n1 { node };
                    E e { edge_type::FREE_RIDE, { n1, n2 } };
                    return std::tuple<E, N, N, ED> { e, n1, n2, freeride_scorer(e, { std::nullopt }, { std::nullopt }) };
                })
            };
            bool has_freeride_from_root { node == get_root_node() };
            blankable_bidirectional_view freeride_set_2 {
                has_freeride_from_root,  // passthru if condition is met, otherwise blank
                std::views::iota(1u, grid_down_cnt)
                | std::views::transform([this](const INDEX& grid_down) {
                    return N { grid_down, 0u };
                })
                | std::views::transform([this](const N& n2) {
                    N n1 { 0, 0 };
                    E e { edge_type::FREE_RIDE, { n1, n2 } };
                    return std::tuple<E, N, N, ED> { e, n1, n2, freeride_scorer(e, { std::nullopt }, { std::nullopt }) };
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
                    return std::tuple<E, N, N, ED> { e, n1, n2, freeride_scorer(e, { std::nullopt }, { std::nullopt }) };
                })
            };
            bool has_freeride_to_leaf { node == get_leaf_node() };
            blankable_bidirectional_view freeride_set_1 {
                has_freeride_to_leaf,  // passthru if condition is met, otherwise blank
                std::views::iota(0u, grid_right_cnt - 1u)
                | std::views::transform([this](const INDEX& grid_right) {
                    return N { grid_down_cnt - 1u, grid_right };
                })
                | std::views::transform([this](const N& n1) {
                    N n2 { get_leaf_node() };
                    E e { edge_type::FREE_RIDE, { n1, n2 } };
                    return std::tuple<E, N, N, ED> { e, n1, n2, freeride_scorer(e, { std::nullopt }, { std::nullopt }) };
                })
            };
            bool has_freeride_from_root { node.down > 0u && node.right == 0u };
            blankable_bidirectional_view freeride_set_2 {
                has_freeride_from_root,  // passthru if condition is met, otherwise blank
                std::views::single(get_root_node())
                | std::views::transform([node, this](const N& n1) {
                    N n2 { node };
                    E e { edge_type::FREE_RIDE, { n1, n2 } };
                    return std::tuple<E, N, N, ED> { e, n1, n2, freeride_scorer(e, { std::nullopt }, { std::nullopt }) };
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
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return get_outputs_full(node)
                | std::views::transform([this](auto v) -> E { return std::get<0>(v); });
        }

        auto get_inputs(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return this->get_inputs_full(node)
                | std::views::transform([this](auto v) -> E { return std::get<0>(v); });
        }

        bool has_outputs(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            auto outputs { this->get_outputs(node) };
            return outputs.begin() != outputs.end();
        }

        bool has_inputs(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            auto inputs { this->get_inputs(node) };
            return inputs.begin() != inputs.end();
        }

        std::size_t get_out_degree(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            auto outputs { std::ranges::common_view { this->get_outputs(node) } };
            auto dist { std::ranges::distance(outputs) };
            return static_cast<std::size_t>(dist);
        }

        std::size_t get_in_degree(const N& node) const {
            if constexpr (debug_mode) {
                if (!has_node(node)) {
                    throw std::runtime_error { "Node doesn't exist" };
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
            const auto& [n1, n2] { edge.inner_edge };
            const auto& [n1_grid_down, n1_grid_right] { n1 };
            const auto& [n2_grid_down, n2_grid_right] { n2 };
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
                    throw std::runtime_error { "Node doesn't exist" };
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
            using CONTAINER = static_vector_typer<debug_mode, E, 2zu>::type;
            CONTAINER ret {};
            const N& leaf_node { get_leaf_node() };
            const auto& [leaf_grid_down, leaf_grid_right] { leaf_node };
            const auto& [grid_down, grid_right] { node };
            if (grid_down == leaf_grid_down && grid_right < leaf_grid_right) {
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
            using CONTAINER = static_vector_typer<debug_mode, E, 2zu>::type;
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

// Struct must be defined outside of namespace block above, otherwise compiler will treat it as part of that namespace.
// NOTE: Inheriting from std::formatter<std::string_view> instead of std::formatter<std::string> because -Wabi-tag warning.
template<offbynull::concepts::widenable_to_size_t INDEX>
struct std::formatter<offbynull::aligner::graphs::pairwise_overlap_alignment_graph::overlap_edge<INDEX>>
    : std::formatter<std::string_view> {
    auto format(
        const offbynull::aligner::graphs::pairwise_overlap_alignment_graph::overlap_edge<INDEX>& e,
        std::format_context& ctx
    ) const {
        return std::format_to(
            ctx.out(),
            "{}-{}->{}",
            e.inner_edge.source,
            e.type == offbynull::aligner::graphs::pairwise_overlap_alignment_graph::edge_type::FREE_RIDE ? "fr" : "--",
            e.inner_edge.destination
        );
    }
};

template<offbynull::concepts::widenable_to_size_t INDEX>
std::ostream& operator<<(std::ostream& os, const offbynull::aligner::graphs::pairwise_overlap_alignment_graph::overlap_edge<INDEX>& e) {
    return os << std::format("{}", e);
}
#endif //OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_OVERLAP_ALIGNMENT_GRAPH_H
