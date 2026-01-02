#ifndef OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_LOCAL_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_LOCAL_ALIGNMENT_GRAPH_H

#include <compare>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <tuple>
#include <stdexcept>
#include <utility>
#include <optional>
#include <type_traits>
#include <functional>
#include <array>
#include <string>
#include <string_view>
#include <format>
#include <ostream>
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/graphs/grid_graph.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/scorer/scorer.h"
#include "offbynull/concepts.h"
#include "offbynull/helpers/concat_bidirectional_view.h"
#include "offbynull/helpers/blankable_bidirectional_view.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::graphs::pairwise_local_alignment_graph {
    using offbynull::aligner::graphs::grid_graph::grid_graph;
    using offbynull::aligner::graphs::grid_graph::empty_node_data;
    using offbynull::aligner::graphs::grid_graph::edge;
    using offbynull::aligner::graphs::grid_graph::node;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::scorer::scorer::scorer;
    using offbynull::aligner::scorer::scorer::scorer_without_explicit_weight;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::helpers::concat_bidirectional_view::concat_bidirectional_view;
    using offbynull::helpers::blankable_bidirectional_view::blankable_bidirectional_view;
    using offbynull::utils::static_vector_typer;
    using offbynull::concepts::unqualified_object_type;
    using offbynull::concepts::bidirectional_range_of_non_cvref;
    using offbynull::aligner::graph::graph::full_input_output_range;

    /**
     * @ref offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph's edge type.
     */
    enum class edge_type : std::uint8_t {
        /** Free-ride edge. */
        FREE_RIDE,
        /** Normal edge (substitution or indel). */
        NORMAL
    };

    PACK_STRUCT_START
    /**
     * Edge identifier type for @ref offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph.
     *
     * Struct is packed when `OBN_PACK_STRUCTS` macro is defined (and platform supports struct packing).
     *
     * @tparam N_INDEX Node coordinate type.
     */
    template<widenable_to_size_t N_INDEX>
    struct local_edge {
    public:
        /** Edge type. */
        edge_type type;
        /**
         * Edge source and destination nodes. Note that the type here is the backing
         * @ref offbynull::aligner::graphs::grid_graph::grid_graph's edge identifier type, which is comprised of a source node ID and a
         * destination node identifier.
         */
        edge<N_INDEX> inner_edge;
        /** Enable spaceship operator. */
        std::strong_ordering operator<=>(const local_edge& rhs) const = default;
    }
    PACK_STRUCT_STOP;

    // A scorer that translates calls from the grid_graph backing the pairwise_local_alignment_graph, such that the edge type becomes the
    // edge type of pairwise_local_alignment_graph.
    /**
     * Pipes offbynull::aligner::scorer::scorer::scorer invocations from
     * @ref offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph to its backing
     * @ref offbynull::aligner::graphs::grid_graph::grid_graph instance. Used only for
     * @ref offbynull::aligner::graphs::pairwise_local_alignment_graph::edge_type::NORMAL edge types.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam N_INDEX Node coordinate type.
     * @tparam DOWN_ELEM Downward sequence type's element.
     * @tparam RIGHT_ELEM Rightward sequence type's element.
     * @tparam WEIGHT_ Edge data type (edge's weight).
     * @tparam GRID_GRAPH_SCORER Backing scorer type.
     */
    template<
        bool debug_mode,
        widenable_to_size_t N_INDEX,
        unqualified_object_type DOWN_ELEM,
        unqualified_object_type RIGHT_ELEM,
        weight WEIGHT_,
        scorer<
            N_INDEX,
            DOWN_ELEM,
            RIGHT_ELEM,
            WEIGHT_
        > GRID_GRAPH_SCORER
    >
    class grid_scorer_to_local_scorer_proxy {
    private:
        GRID_GRAPH_SCORER grid_graph_scorer;
    public:
        /** @copydoc offbynull::aligner::scorer::scorer::unimplemented_scorer::WEIGHT */
        using WEIGHT = WEIGHT_;
        /** @copydoc offbynull::aligner::scorer::scorer::unimplemented_scorer::SEQ_INDEX */
        using SEQ_INDEX = N_INDEX;

        /**
         * Construct an @ref offbynull::aligner::graphs::pairwise_local_alignment_graph::grid_scorer_to_local_scorer_proxy instance.
         *
         * @param grid_graph_scorer_ Backing @ref offbynull::aligner::scorer::scorer::scorer.
         */
        grid_scorer_to_local_scorer_proxy(
            const GRID_GRAPH_SCORER& grid_graph_scorer_
        )
        : grid_graph_scorer { grid_graph_scorer_ } {}

        /**
         * Score edge.
         *
         * @param down_elem Downward element associated with `edge`, if any.
         * @param right_elem Rightward element associated with `edge`, if any.
         * @return Score for edge (edge weight).
         */
        WEIGHT_ operator()(
            const std::optional<
                std::pair<
                    N_INDEX,
                    std::reference_wrapper<const char>
                >
            > down_elem,
            const std::optional<
                std::pair<
                    N_INDEX,
                    std::reference_wrapper<const char>
                >
            > right_elem
        ) const {
            return grid_graph_scorer(
                down_elem,
                right_elem
            );
        }
    };

    /**
     * @ref offbynull::aligner::graph::sliceable_pairwise_alignment_graph::sliceable_pairwise_alignment_graph implementation of a
     * pairwise local sequence alignment graph.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam N_INDEX_ Node coordinate type.
     * @tparam WEIGHT Edge data type (edge's weight).
     * @tparam DOWN_SEQ Downward sequence type.
     * @tparam RIGHT_SEQ Rightward sequence type.
     * @tparam SUBSTITUTION_SCORER Scorer type used to score sequence alignment substitutions.
     * @tparam GAP_SCORER Scorer type to score for sequence alignment gaps (indels).
     * @tparam FREERIDE_SCORER Scorer type to score for sequence alignment free rides.
     */
    template<
        bool debug_mode,
        widenable_to_size_t N_INDEX_,
        weight WEIGHT,
        sequence DOWN_SEQ,
        sequence RIGHT_SEQ,
        scorer<
            N_INDEX_,
            std::remove_cvref_t<decltype(std::declval<DOWN_SEQ>()[0zu])>,
            std::remove_cvref_t<decltype(std::declval<RIGHT_SEQ>()[0zu])>,
            WEIGHT
        > SUBSTITUTION_SCORER,
        scorer<
            N_INDEX_,
            std::remove_cvref_t<decltype(std::declval<DOWN_SEQ>()[0zu])>,
            std::remove_cvref_t<decltype(std::declval<RIGHT_SEQ>()[0zu])>,
            WEIGHT
        > GAP_SCORER,
        scorer<
            N_INDEX_,
            std::remove_cvref_t<decltype(std::declval<DOWN_SEQ>()[0zu])>,
            std::remove_cvref_t<decltype(std::declval<RIGHT_SEQ>()[0zu])>,
            WEIGHT
        > FREERIDE_SCORER
    >
    class pairwise_local_alignment_graph {
    public:
        /** Element object type of downward sequence (CV-qualification and references removed). */
        using DOWN_ELEM = std::remove_cvref_t<decltype(std::declval<DOWN_SEQ>()[0zu])>;
        /** Element object type of rightward sequence (CV-qualification and references removed). */
        using RIGHT_ELEM = std::remove_cvref_t<decltype(std::declval<RIGHT_SEQ>()[0zu])>;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::N_INDEX */
        using N_INDEX = N_INDEX_;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::N */
        using N = node<N_INDEX>;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::E */
        using E = local_edge<N_INDEX>;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::ND */
        using ND = empty_node_data;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::ED */
        using ED = WEIGHT;

    private:
        static constexpr N_INDEX I0 { static_cast<N_INDEX>(0zu) };
        static constexpr N_INDEX I1 { static_cast<N_INDEX>(1zu) };
        static constexpr N_INDEX I2 { static_cast<N_INDEX>(2zu) };

        const grid_graph<
            debug_mode,
            N_INDEX_,
            WEIGHT,
            DOWN_SEQ,
            RIGHT_SEQ,
            grid_scorer_to_local_scorer_proxy<debug_mode, N_INDEX_, DOWN_ELEM, RIGHT_ELEM, WEIGHT, SUBSTITUTION_SCORER>,
            grid_scorer_to_local_scorer_proxy<debug_mode, N_INDEX_, DOWN_ELEM, RIGHT_ELEM, WEIGHT, GAP_SCORER>
        > g;
        const FREERIDE_SCORER freeride_scorer;

    public:
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::grid_down_cnt */
        const N_INDEX grid_down_cnt;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::grid_right_cnt */
        const N_INDEX grid_right_cnt;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::grid_depth_cnt */
        static constexpr N_INDEX grid_depth_cnt { decltype(g)::grid_depth_cnt };
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::resident_nodes_capacity */
        const std::size_t resident_nodes_capacity;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::path_edge_capacity */
        const std::size_t path_edge_capacity;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::node_incoming_edge_capacity */
        const std::size_t node_incoming_edge_capacity;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::node_outgoing_edge_capacity */
        const std::size_t node_outgoing_edge_capacity;

        // Scorer params are not being made into universal references because there's a high chance of enabling a subtle bug: There's a
        // non-trivial possibility that the user will submit the same object for both scorers, and so if the universal reference ends up
        // being an rvalue reference it'll try to move the same object twice.
        /**
         * Construct an @ref offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph instance.
         *
         * @param down_seq_ Downward sequence.
         * @param right_seq_ Rightward sequence.
         * @param substitution_scorer_ Scorer for sequence alignment substitutions.
         * @param gap_scorer_ Scorer for sequence alignment gaps (indels).
         * @param freeride_scorer_ Score for sequence alignment freerides.
         */
        pairwise_local_alignment_graph(
            const DOWN_SEQ& down_seq_,
            const RIGHT_SEQ& right_seq_,
            const SUBSTITUTION_SCORER substitution_scorer_,
            const GAP_SCORER gap_scorer_,
            const FREERIDE_SCORER freeride_scorer_
        )
        : g {
            down_seq_,
            right_seq_,
            { substitution_scorer_ }, /* grid_scorer_to_local_scorer_proxy */
            { gap_scorer_ } /* grid_scorer_to_local_scorer_proxy */
        }
        , freeride_scorer { freeride_scorer_ }
        , grid_down_cnt { g.grid_down_cnt }
        , grid_right_cnt { g.grid_right_cnt }
        , resident_nodes_capacity { 2zu }
        , path_edge_capacity { g.path_edge_capacity }
        , node_incoming_edge_capacity { g.node_incoming_edge_capacity + (grid_down_cnt * grid_right_cnt) - 1zu }
        , node_outgoing_edge_capacity { g.node_outgoing_edge_capacity + (grid_down_cnt * grid_right_cnt) - 1zu } {}

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_node_data */
        ND get_node_data(const N& n) const {
            return g.get_node_data(n);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_edge_data */
        ED get_edge_data(const E& e) const {
            return std::get<2zu>(get_edge(e));
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_edge_from */
        N get_edge_from(const E& e) const {
            return std::get<0zu>(get_edge(e));
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_edge_to */
        N get_edge_to(const E& e) const {
            return std::get<1zu>(get_edge(e));
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_edge */
        std::tuple<N, N, ED> get_edge(const E& e) const {
            if constexpr (debug_mode) {
                if (!has_edge(e)) {
                    throw std::runtime_error { "Edge doesn't exist" };
                }
            }
            if (e.type == edge_type::FREE_RIDE) {
                const auto& [n1, n2] { e.inner_edge };
                return std::tuple<N, N, ED> { n1, n2, freeride_scorer({ std::nullopt }, { std::nullopt }) };
            } else {
                return g.get_edge(e.inner_edge);
            }
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_root_nodes */
        bidirectional_range_of_non_cvref<N> auto get_root_nodes() const {
            return g.get_root_nodes();
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_root_node */
        N get_root_node() const {
            return g.get_root_node();
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_leaf_nodes */
        bidirectional_range_of_non_cvref<N> auto get_leaf_nodes() const {
            return g.get_leaf_nodes();
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_leaf_node */
        N get_leaf_node() const {
            return g.get_leaf_node();
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_nodes */
        bidirectional_range_of_non_cvref<N> auto get_nodes() const {
            return g.get_nodes();
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_edges */
        bidirectional_range_of_non_cvref<E> auto get_edges() const {
            auto from_src_range {
                std::views::cartesian_product(
                    std::views::iota(I0, g.grid_down_cnt),
                    std::views::iota(I0, g.grid_right_cnt)
                )
                | std::views::drop(1zu) // drop 0,0
                | std::views::transform([&](const auto & p) {
                    N n1 { I0, I0 };
                    N n2 { std::get<0zu>(p), std::get<1zu>(p) };
                    return E { edge_type::FREE_RIDE, { n1, n2 } };
                })
            };
            auto to_sink_range {
                std::views::cartesian_product(
                    std::views::iota(I0, g.grid_down_cnt),
                    std::views::iota(I0, g.grid_right_cnt)
                )
                | ( std::views::reverse | std::views::drop(1zu) | std::views::reverse ) // drop bottom right
                | std::views::transform([&](const auto & p) {
                    N n1 {
                        std::get<0zu>(p),
                        std::get<1zu>(p)
                    };
                    N n2 {
                        static_cast<N_INDEX>(g.grid_down_cnt - I1),  // Cast to prevent narrowing warning
                        static_cast<N_INDEX>(g.grid_right_cnt - I1)  // Cast to prevent narrowing warning
                    };
                    return E { edge_type::FREE_RIDE, { n1, n2 } };
                })
            };
            auto real_range {
                g.get_edges()
                | std::views::transform([&](const auto & p) {
                    return E { edge_type::NORMAL, p };
                })
            };
            // This should be using std::views::concat, but it wasn't included in this version of the C++ standard
            // library. The concat implementation below lacks several features (e.g. doesn't support the pipe operator)
            // and forcefully returns copies (concat_bidirectional_view::iterator::value_type ==
            // concat_bidirectional_view::iterator::reference_type).
            return concat_bidirectional_view {
                std::move(real_range),
                concat_bidirectional_view {
                    std::move(from_src_range),
                    std::move(to_sink_range)
                }
            };
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::has_node */
        bool has_node(const N& n) const {
            return g.has_node(n);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::has_edge */
        bool has_edge(const E& e) const {
            if (e.type == edge_type::NORMAL) {
                return g.has_edge(e.inner_edge);
            } else {
                const auto & [n1, n2] { e.inner_edge };
                const N root { g.get_root_node() };
                const N leaf { g.get_leaf_node() };
                if (n1.down == root.down && n1.right == root.right) {
                    if (n2.down == root.down && n2.right == root.right) {
                        return false;
                    } else if (n2.down <= leaf.down && n2.right <= leaf.right) {
                        return true;
                    } else {
                        return false;
                    }
                } else if (n2.down == leaf.down && n2.right == leaf.right) {
                    if (n1.down == leaf.down && n1.right == leaf.right) {
                        return false;
                    } else if (n1.down <= leaf.down && n1.right <= leaf.right) {
                        return true;
                    } else {
                        return false;
                    }
                } else {
                    return false;
                }
            }
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_outputs_full */
        full_input_output_range<N, E, ED> auto get_outputs_full(const N& n) const {
            auto standard_outputs {
                g.get_outputs_full(n)
                | std::views::transform([this](const auto& raw_full_edge) {
                    N n1 { std::get<1zu>(raw_full_edge) };
                    N n2 { std::get<2zu>(raw_full_edge) };
                    E e { edge_type::NORMAL, { n1, n2 } };
                    return std::tuple<E, N, N, ED> { e, n1, n2, freeride_scorer({ std::nullopt }, { std::nullopt }) };
                })
            };
            bool has_freeride_to_leaf { n != get_leaf_node() };
            blankable_bidirectional_view freeride_set_1 {
                has_freeride_to_leaf,  // passthru if condition is met, otherwise blank
                std::views::single(get_leaf_node())
                | std::views::transform([n, this](const N& n2) {
                    N n1 { n };
                    E e { edge_type::FREE_RIDE, { n1, n2 } };
                    return std::tuple<E, N, N, ED> { e, n1, n2, freeride_scorer({ std::nullopt }, { std::nullopt }) };
                })
            };
            bool has_freeride_from_root { n == get_root_node() };
            blankable_bidirectional_view freeride_set_2 {
                has_freeride_from_root,  // passthru if condition is met, otherwise blank
                std::views::cartesian_product(
                    std::views::iota(I0, grid_down_cnt),
                    std::views::iota(I0, grid_right_cnt)
                )
                | std::views::take(grid_down_cnt * grid_right_cnt - 1zu)  // Remove leaf (will be added by non_leaf_only_outputs)
                                                                          //    don't use "- I1" here as this must be std::size_t
                | std::views::drop(I1)  // Remove root
                | std::views::transform([this](const auto& n2_coords) {
                    N n1 { I0, I0 };
                    N n2 { std::get<0zu>(n2_coords), std::get<1zu>(n2_coords) };
                    E e { edge_type::FREE_RIDE, { n1, n2 } };
                    return std::tuple<E, N, N, ED> { e, n1, n2, freeride_scorer({ std::nullopt }, { std::nullopt }) };
                })
            };
            return concat_bidirectional_view {
                std::move(standard_outputs),
                concat_bidirectional_view {
                    std::move(freeride_set_1),
                    std::move(freeride_set_2)
                }
            };
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_inputs_full */
        full_input_output_range<N, E, ED> auto get_inputs_full(const N& n) const {
            auto standard_inputs {
                g.get_inputs_full(n)
                | std::views::transform([this](const auto& raw_full_edge) {
                    N n1 { std::get<1zu>(raw_full_edge) };
                    N n2 { std::get<2zu>(raw_full_edge) };
                    E e { edge_type::NORMAL, { n1, n2 } };
                    return std::tuple<E, N, N, ED> { e, n1, n2, freeride_scorer({ std::nullopt }, { std::nullopt }) };
                })
            };
            bool has_freeride_to_leaf { n == get_leaf_node() };
            blankable_bidirectional_view freeride_set_1 {
                has_freeride_to_leaf,
                std::views::cartesian_product(
                    std::views::iota(I0, grid_down_cnt),
                    std::views::iota(I0, grid_right_cnt)
                )
                | std::views::take(grid_down_cnt * grid_right_cnt - 1zu)  // Remove leaf - don't use "- I1" here as this must be std::size_t
                | std::views::drop(1zu)  // Remove root (will be added by non_root_only_inputs)
                | std::views::transform([this](const auto& n1_coords) {
                    N n1 {
                        std::get<0zu>(n1_coords),
                        std::get<1zu>(n1_coords)
                    };
                    N n2 {
                        static_cast<N_INDEX>(grid_down_cnt - I1),  // Cast to prevent narrowing warning
                        static_cast<N_INDEX>(grid_right_cnt - I1)  // Cast to prevent narrowing warning
                    };
                    E e { edge_type::FREE_RIDE, { n1, n2 } };
                    return std::tuple<E, N, N, ED> { e, n1, n2, freeride_scorer({ std::nullopt }, { std::nullopt }) };
                })
            };
            bool has_freeride_from_root { n != get_root_node() };
            blankable_bidirectional_view freeride_set_2 {
                has_freeride_from_root,
                std::views::single(get_root_node())
                | std::views::transform([n, this](const N& n1) {
                    N n2 { n };
                    E e { edge_type::FREE_RIDE, { n1, n2 } };
                    return std::tuple<E, N, N, ED> { e, n1, n2, freeride_scorer({ std::nullopt }, { std::nullopt }) };
                })
            };
            return concat_bidirectional_view {
                std::move(standard_inputs),
                concat_bidirectional_view {
                    std::move(freeride_set_1),
                    std::move(freeride_set_2)
                }
            };
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_outputs */
        bidirectional_range_of_non_cvref<E> auto get_outputs(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return get_outputs_full(n)
                | std::views::transform([this](auto v) -> E { return std::get<0zu>(v); });
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_inputs */
        bidirectional_range_of_non_cvref<E> auto get_inputs(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return this->get_inputs_full(n)
                | std::views::transform([this](auto v) -> E { return std::get<0zu>(v); });
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::has_outputs */
        bool has_outputs(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            auto outputs { this->get_outputs(n) };
            return outputs.begin() != outputs.end();
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::has_inputs */
        bool has_inputs(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            auto inputs { this->get_inputs(n) };
            return inputs.begin() != inputs.end();
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_out_degree */
        std::size_t get_out_degree(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            auto outputs { this->get_outputs(n) };
            auto dist { std::ranges::distance(outputs) };
            return static_cast<std::size_t>(dist);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_in_degree */
        std::size_t get_in_degree(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            auto inputs { this->get_inputs(n) };
            auto dist { std::ranges::distance(inputs) };
            return static_cast<std::size_t>(dist);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::edge_to_element_offsets */
        std::optional<
            std::pair<
                std::optional<N_INDEX>,
                std::optional<N_INDEX>
            >
        > edge_to_element_offsets(
            const E& e
        ) const {
            if constexpr (debug_mode) {
                if (!has_edge(e)) {
                    throw std::runtime_error { "Edge doesn't exist" };
                }
            }
            using OPT_INDEX = std::optional<N_INDEX>;
            using RET = std::optional<std::pair<OPT_INDEX, OPT_INDEX>>;

            if (e.type == edge_type::FREE_RIDE) {
                // Returning nullopt directly means a conversion to RET happens behind the scene, and that makes the concept check fail.
                return RET { std::nullopt };
            }
            const auto& [n1, n2] { e.inner_edge };
            const auto& [n1_grid_down, n1_grid_right] { n1 };
            const auto& [n2_grid_down, n2_grid_right] { n2 };
            if (n1_grid_down + I1 == n2_grid_down && n1_grid_right + I1 == n2_grid_right) {
                return RET { { { n1_grid_down }, { n1_grid_right } } };
            } else if (n1_grid_down + I1 == n2_grid_down && n1_grid_right == n2_grid_right) {
                return RET { { { n1_grid_down }, std::nullopt } };
            } else if (n1_grid_down == n2_grid_down && n1_grid_right + I1 == n2_grid_right) {
                return RET { { std::nullopt, { n1_grid_right } } };
            }
            if constexpr (debug_mode) {
                throw std::runtime_error { "This should never happen" };
            }
            std::unreachable();
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::node_to_grid_offset */
        std::tuple<N_INDEX, N_INDEX, std::size_t> node_to_grid_offset(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return g.node_to_grid_offset(n);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::grid_offset_to_nodes */
        bidirectional_range_of_non_cvref<N> auto grid_offset_to_nodes(N_INDEX grid_down, N_INDEX grid_right) const {
            return g.grid_offset_to_nodes(grid_down, grid_right);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::row_nodes */
        bidirectional_range_of_non_cvref<N> auto row_nodes(N_INDEX grid_down) const {
            return g.row_nodes(grid_down);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::row_nodes */
        bidirectional_range_of_non_cvref<N> auto row_nodes(N_INDEX grid_down, const N& root_node, const N& leaf_node) const {
            return g.row_nodes(grid_down, root_node, leaf_node);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::is_reachable */
        bool is_reachable(const N& n1, const N& n2) const {
            return g.is_reachable(n1, n2);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::resident_nodes */
        bidirectional_range_of_non_cvref<N> auto resident_nodes() const {
            return std::array<N, 2zu> { g.get_root_node(), g.get_leaf_node() };
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::outputs_to_residents */
        bidirectional_range_of_non_cvref<E> auto outputs_to_residents(const N& n) const {
            using CONTAINER = static_vector_typer<debug_mode, E, 2zu>::type;
            CONTAINER ret {};
            const N& leaf_node { get_leaf_node() };
            if (n != leaf_node) {
                ret.push_back(E { edge_type::FREE_RIDE, { n, leaf_node } });
            }
            const auto& [leaf_grid_down, leaf_grid_right] { leaf_node };
            const auto& [grid_down, grid_right] { n };
            if ((grid_down + I1 == leaf_grid_down && grid_right + I1 == leaf_grid_right)
                    || (grid_down == leaf_grid_down && grid_right + I1 == leaf_grid_right)
                    || (grid_down + I1 == leaf_grid_down && grid_right == leaf_grid_right)) {
                ret.push_back(E { edge_type::NORMAL, { n, leaf_node } });
            }
            return ret;
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::inputs_from_residents */
        bidirectional_range_of_non_cvref<E> auto inputs_from_residents(const N& n) const {
            using CONTAINER = static_vector_typer<debug_mode, E, 2zu>::type;
            CONTAINER ret {};
            const N& root_node { get_root_node() };
            if (n != root_node) {
                ret.push_back(E { edge_type::FREE_RIDE, { root_node, n } });
            }
            const auto& [root_grid_down, root_grid_right] { root_node };
            const auto& [grid_down, grid_right] { n };
            if ((grid_down - I1 == root_grid_down && grid_right - I1 == root_grid_right)
                    || (grid_down - I1 == root_grid_down && grid_right == root_grid_right)
                    || (grid_down == root_grid_down && grid_right - I1 == root_grid_right)) {
                ret.push_back(E { edge_type::NORMAL, { root_node, n } });
            }
            return ret;
        }
    };

    /**
     * Create @ref offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph instance, where template
     * parameters are deduced / inferred from arguments passed in.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam N_INDEX Node coordinate type.
     * @param down_seq Downward sequence.
     * @param right_seq Rightward sequence.
     * @param substitution_scorer Scorer for sequence alignment substitutions.
     * @param gap_scorer Scorer for sequence alignment gaps (indels).
     * @param freeride_scorer Score for sequence alignment freerides.
     * @return New @ref offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph instance.
     */
    template<
        bool debug_mode,
        widenable_to_size_t N_INDEX
    >
    auto create_pairwise_local_alignment_graph(
        const sequence auto& down_seq,
        const sequence auto& right_seq,
        const scorer_without_explicit_weight<
            N_INDEX,
            std::remove_cvref_t<decltype(down_seq[0zu])>,
            std::remove_cvref_t<decltype(right_seq[0zu])>
        > auto& substitution_scorer,
        const scorer_without_explicit_weight<
            N_INDEX,
            std::remove_cvref_t<decltype(down_seq[0zu])>,
            std::remove_cvref_t<decltype(right_seq[0zu])>
        > auto& gap_scorer,
        const scorer_without_explicit_weight<
            N_INDEX,
            std::remove_cvref_t<decltype(down_seq[0zu])>,
            std::remove_cvref_t<decltype(right_seq[0zu])>
        > auto& freeride_scorer
    ) {
        using DOWN_SEQ = std::remove_cvref_t<decltype(down_seq)>;
        using DOWN_ELEM = std::remove_cvref_t<decltype(down_seq[0zu])>;
        using RIGHT_SEQ = std::remove_cvref_t<decltype(right_seq)>;
        using RIGHT_ELEM = std::remove_cvref_t<decltype(right_seq[0zu])>;
        using WEIGHT_1 = decltype(
            substitution_scorer(
                std::declval<
                    const std::optional<
                        std::pair<
                            N_INDEX,
                            std::reference_wrapper<const DOWN_ELEM>
                        >
                    >
                >(),
                std::declval<
                    const std::optional<
                        std::pair<
                            N_INDEX,
                            std::reference_wrapper<const RIGHT_ELEM>
                        >
                    >
                >()
            )
        );
        using WEIGHT_2 = decltype(
            gap_scorer(
                std::declval<
                    const std::optional<
                        std::pair<
                            N_INDEX,
                            std::reference_wrapper<const DOWN_ELEM>
                        >
                    >
                >(),
                std::declval<
                    const std::optional<
                        std::pair<
                            N_INDEX,
                            std::reference_wrapper<const RIGHT_ELEM>
                        >
                    >
                >()
            )
        );
        using WEIGHT_3 = decltype(
            freeride_scorer(
                std::declval<
                    const std::optional<
                        std::pair<
                            N_INDEX,
                            std::reference_wrapper<const DOWN_ELEM>
                        >
                    >
                >(),
                std::declval<
                    const std::optional<
                        std::pair<
                            N_INDEX,
                            std::reference_wrapper<const RIGHT_ELEM>
                        >
                    >
                >()
            )
        );
        static_assert(std::is_same_v<WEIGHT_1, WEIGHT_2>, "Scorers must return the same weight type");
        static_assert(std::is_same_v<WEIGHT_1, WEIGHT_3>, "Scorers must return the same weight type");
        return pairwise_local_alignment_graph<
            debug_mode,
            N_INDEX,
            WEIGHT_1,
            DOWN_SEQ,
            RIGHT_SEQ,
            std::remove_cvref_t<decltype(substitution_scorer)>,
            std::remove_cvref_t<decltype(gap_scorer)>,
            std::remove_cvref_t<decltype(freeride_scorer)>
        > {
            down_seq,
            right_seq,
            substitution_scorer,
            gap_scorer,
            freeride_scorer
        };
    }
}

// Struct must be defined outside of namespace block above, otherwise compiler will treat it as part of that namespace.
// NOTE: Inheriting from std::formatter<std::string_view> instead of std::formatter<std::string> because -Wabi-tag warning.
template<offbynull::concepts::widenable_to_size_t N_INDEX>
struct std::formatter<offbynull::aligner::graphs::pairwise_local_alignment_graph::local_edge<N_INDEX>> : std::formatter<std::string_view> {
    auto format(
        const offbynull::aligner::graphs::pairwise_local_alignment_graph::local_edge<N_INDEX>& e,
        std::format_context& ctx
    ) const {
        return std::format_to(
            ctx.out(),
            "{}-{}->{}",
            e.inner_edge.source,
            e.type == offbynull::aligner::graphs::pairwise_local_alignment_graph::edge_type::FREE_RIDE ? "fr" : "--",
            e.inner_edge.destination
        );
    }
};

template<offbynull::concepts::widenable_to_size_t N_INDEX>
std::ostream& operator<<(std::ostream& os, const offbynull::aligner::graphs::pairwise_local_alignment_graph::local_edge<N_INDEX>& e) {
    return os << std::format("{}", e);
}

#endif //OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_LOCAL_ALIGNMENT_GRAPH_H
