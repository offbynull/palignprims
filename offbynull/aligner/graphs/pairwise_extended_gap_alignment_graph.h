#ifndef OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_EXTENDED_GAP_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_EXTENDED_GAP_ALIGNMENT_GRAPH_H

#include <cstdint>
#include <cstddef>
#include <ranges>
#include <tuple>
#include <optional>
#include <stdexcept>
#include <utility>
#include <string>
#include <string_view>
#include <array>
#include <ostream>
#include <format>
#include <type_traits>
#include <functional>
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/scorer/scorer.h"
#include "offbynull/concepts.h"
#include "offbynull/helpers/concat_bidirectional_view.h"
#include "offbynull/utils.h"
#include "offbynull/helpers/simple_value_bidirectional_view.h"

namespace offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph {
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::scorer::scorer::scorer;
    using offbynull::aligner::scorer::scorer::scorer_without_explicit_weight;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::helpers::concat_bidirectional_view::concat_bidirectional_view;
    using offbynull::utils::static_vector_typer;
    using offbynull::helpers::simple_value_bidirectional_view::simple_value_bidirectional_view;
    using offbynull::concepts::bidirectional_range_of_non_cvref;
    using offbynull::aligner::graph::graph::full_input_output_range;

    /**
     * Node data type used by
     * @ref offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::pairwise_extended_gap_alignment_graph, which is an empty type
     * (no data kept for nodes).
     */
    using empty_node_data = std::tuple<>;

    /**
     * Layer within @ref offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::pairwise_extended_gap_alignment_graph.
     */
    enum class node_layer : std::uint8_t {
        /**
         * Extended gap layer for downward sequence.
         */
        DOWN,
        /**
         * Extended gap layer for rightward sequence.
         */
        RIGHT,
        /**
         * Substitution layer.
         */
        DIAGONAL  // Diag is last - while in same grid offset, this is the layer that the other two feed into. Algorithms expect this.
    };

    PACK_STRUCT_START
    /**
     * Node identifier type for
     * @ref offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::pairwise_extended_gap_alignment_graph. Instances of this type
     * uniquely identify a position on the grid.
     *
     * Struct is packed when `OBN_PACK_STRUCTS` macro is defined (and platform supports struct packing).
     *
     * @tparam N_INDEX Node coordinate type (smaller integer types may reduce memory consumption).
     */
    template<widenable_to_size_t N_INDEX>
    struct node {
        /** Layer node sits in. */
        node_layer layer;
        /** Row / vertical position of node, starting from the top going downward. */
        N_INDEX down;
        /** Column / horizontal position of node, starting from the left going rightward. */
        N_INDEX right;
        /** Enable spaceship operator. */
        auto operator<=>(const node&) const = default;
    }
    PACK_STRUCT_STOP;

    PACK_STRUCT_START
    /**
     * Edge identifier type for
     * @ref offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::pairwise_extended_gap_alignment_graph.
     *
     * Struct is packed when `OBN_PACK_STRUCTS` macro is defined (and platform supports struct packing).
     *
     * @tparam N_INDEX Node coordinate type.
     */
    template<widenable_to_size_t N_INDEX>
    struct edge {
        /** Source node's identifier. */
        node<N_INDEX> source;
        /** Destination node's identifier. */
        node<N_INDEX> destination;
        /** Enable spaceship operator. */
        auto operator<=>(const edge&) const = default;
    }
    PACK_STRUCT_STOP;

    /**
     * @ref offbynull::aligner::graph::sliceable_pairwise_alignment_graph::sliceable_pairwise_alignment_graph implementation of a
     * pairwise extended gap sequence alignment graph.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam N_INDEX_ Node coordinate type.
     * @tparam WEIGHT Edge data type (edge's weight).
     * @tparam DOWN_SEQ Downward sequence type.
     * @tparam RIGHT_SEQ Rightward sequence type.
     * @tparam SUBSTITUTION_SCORER Scorer type used to score sequence alignment substitutions.
     * @tparam INITIAL_GAP_SCORER Scorer type to score for initial sequence alignment gaps (initial indel), going from the substitution
     *     layer to one of the extended gap layers.
     * @tparam EXTENDED_GAP_SCORER Scorer type to score subsequent sequence alignment gaps (extended indels), staying within the same
     *     extended gap layer.
     * @tparam FREERIDE_SCORER Scorer type to score for sequence alignment free rides, going from one of the extended gap layers to the
     *     substitution layer.
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
        > INITIAL_GAP_SCORER,
        scorer<
            N_INDEX_,
            std::remove_cvref_t<decltype(std::declval<DOWN_SEQ>()[0zu])>,
            std::remove_cvref_t<decltype(std::declval<RIGHT_SEQ>()[0zu])>,
            WEIGHT
        > EXTENDED_GAP_SCORER,
        scorer<
            N_INDEX_,
            std::remove_cvref_t<decltype(std::declval<DOWN_SEQ>()[0zu])>,
            std::remove_cvref_t<decltype(std::declval<RIGHT_SEQ>()[0zu])>,
            WEIGHT
        > FREERIDE_SCORER
    >
    class pairwise_extended_gap_alignment_graph {
    public:
        /** Element object type of downward sequence (CV-qualification and references removed). */
        using DOWN_ELEM = std::remove_cvref_t<decltype(std::declval<DOWN_SEQ>()[0zu])>;
        /** Element object type of rightward sequence (CV-qualification and references removed). */
        using RIGHT_ELEM = std::remove_cvref_t<decltype(std::declval<RIGHT_SEQ>()[0zu])>;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::N_INDEX */
        using N_INDEX = N_INDEX_;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::N */
        using N = node<N_INDEX>;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::ND */
        using ND = empty_node_data;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::E */
        using E = edge<N_INDEX>;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::ED */
        using ED = WEIGHT;

    private:
        static constexpr N_INDEX I0 { static_cast<N_INDEX>(0zu) };
        static constexpr N_INDEX I1 { static_cast<N_INDEX>(1zu) };
        static constexpr N_INDEX I2 { static_cast<N_INDEX>(2zu) };
        static constexpr N_INDEX I3 { static_cast<N_INDEX>(3zu) };
        
        const DOWN_SEQ& down_seq;
        const RIGHT_SEQ& right_seq;
        const SUBSTITUTION_SCORER substitution_scorer;
        const INITIAL_GAP_SCORER initial_gap_scorer;
        const EXTENDED_GAP_SCORER extended_gap_scorer;
        const FREERIDE_SCORER freeride_scorer;

        std::tuple<E, N, N, ED> construct_full_edge(N n1, N n2) const {
            return std::tuple<E, N, N, ED> {
                E { n1, n2 },
                n1,
                n2,
                get_edge_data(E { n1, n2 })
            };
        }

        std::size_t to_raw_idx(N_INDEX down_idx, N_INDEX right_idx) const {
            std::size_t down_idx_widened { down_idx };
            std::size_t right_idx_widened { right_idx };
            return (down_idx_widened * grid_right_cnt) + right_idx_widened;
        }

    public:
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::grid_down_cnt */
        const N_INDEX grid_down_cnt;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::grid_right_cnt */
        const N_INDEX grid_right_cnt;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::grid_depth_cnt */
        static constexpr N_INDEX grid_depth_cnt { I3 };
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::resident_nodes_capacity */
        static constexpr std::size_t resident_nodes_capacity { 0zu };
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
         * Construct an @ref offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::pairwise_extended_gap_alignment_graph
         * instance.
         *
         * @param down_seq_ Downward sequence.
         * @param right_seq_ Rightward sequence.
         * @param substitution_scorer_ Scorer for sequence alignment substitutions.
         * @param initial_gap_scorer_ Scorer for initial sequence alignment gaps (indels).
         * @param extended_gap_scorer_ Scorer for extended sequence alignment gaps (indels).
         * @param freeride_scorer_ Score for sequence alignment freerides.
         */
        pairwise_extended_gap_alignment_graph(
            const DOWN_SEQ& down_seq_,
            const RIGHT_SEQ& right_seq_,
            const SUBSTITUTION_SCORER& substitution_scorer_,
            const INITIAL_GAP_SCORER& initial_gap_scorer_,
            const EXTENDED_GAP_SCORER& extended_gap_scorer_,
            const FREERIDE_SCORER& freeride_scorer_
        )
        : down_seq { down_seq_ }
        , right_seq { right_seq_ }
        , substitution_scorer { substitution_scorer_ }
        , initial_gap_scorer { initial_gap_scorer_ }
        , extended_gap_scorer { extended_gap_scorer_ }
        , freeride_scorer { freeride_scorer_ }
        , grid_down_cnt { static_cast<N_INDEX>(down_seq.size() + I1) }  // Cast to prevent narrowing warning
        , grid_right_cnt { static_cast<N_INDEX>(right_seq.size() + I1) }  // Cast to prevent narrowing warning
        , path_edge_capacity { (grid_right_cnt - 1zu) * 2zu + (grid_down_cnt - 1zu) * 2zu }
        , node_incoming_edge_capacity { 3zu }
        , node_outgoing_edge_capacity { 3zu } {}

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_node_data */
        ND get_node_data(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return {};
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_edge_data */
        ED get_edge_data(const E& e) const {
            if constexpr (debug_mode) {
                if (!has_edge(e)) {
                    throw std::runtime_error { "Edge doesn't exist" };
                }
            }
            const auto& [n1_layer, n1_grid_down, n1_grid_right] { e.source };
            const auto& [n2_layer, n2_grid_down, n2_grid_right] { e.destination };
            if (n1_layer == node_layer::DIAGONAL && n2_layer == node_layer::DIAGONAL) {  // match
                return substitution_scorer(
                    { { n1_grid_down, { down_seq[n1_grid_down] } } },
                    { { n1_grid_right, { right_seq[n1_grid_right] } } }
                );
            } else if (n1_layer == node_layer::DOWN && n2_layer == node_layer::DOWN) {  // gap
                return extended_gap_scorer(
                    { { n1_grid_down, { down_seq[n1_grid_down] } } },
                    { std::nullopt }
                );
            } else if (n1_layer == node_layer::RIGHT && n2_layer == node_layer::RIGHT) {  // gap
                return extended_gap_scorer(
                    { std::nullopt },
                    { { n1_grid_right, { right_seq[n1_grid_right] } } }
                );
            } else if (n1_layer == node_layer::DIAGONAL && n2_layer == node_layer::DOWN) {  // indel
                return initial_gap_scorer(
                    { { n1_grid_down, { down_seq[n1_grid_down] } } },
                    { std::nullopt }
                );
            } else if (n1_layer == node_layer::DIAGONAL && n2_layer == node_layer::RIGHT) {  // indel
                return initial_gap_scorer(
                    { std::nullopt },
                    { { n1_grid_right, { right_seq[n1_grid_right] } } }
                );
            } else if (n1_layer == node_layer::DOWN && n2_layer == node_layer::DIAGONAL) {  // freeride
                return freeride_scorer(
                    { std::nullopt },
                    { std::nullopt }
                );
            } else if (n1_layer == node_layer::RIGHT && n2_layer == node_layer::DIAGONAL) {  // freeride
                return freeride_scorer(
                    { std::nullopt },
                    { std::nullopt }
                );
            }
            if constexpr (debug_mode) {
                throw std::runtime_error { "This should never happen" };
            }
            std::unreachable();
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_edge_from */
        N get_edge_from(const E& e) const {
            if constexpr (debug_mode) {
                if (!has_edge(e)) {
                    throw std::runtime_error { "Edge doesn't exist" };
                }
            }
            return e.source;
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_edge_to */
        N get_edge_to(const E& e) const {
            if constexpr (debug_mode) {
                if (!has_edge(e)) {
                    throw std::runtime_error { "Edge doesn't exist" };
                }
            }
            return e.destination;
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_edge */
        std::tuple<N, N, ED> get_edge(const E& e) const {
            if constexpr (debug_mode) {
                if (!has_edge(e)) {
                    throw std::runtime_error { "Edge doesn't exist" };
                }
            }
            return std::tuple<N, N, ED> { get_edge_from(e), get_edge_from(e), get_edge_data(e) };
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_root_nodes */
        bidirectional_range_of_non_cvref<N> auto get_root_nodes() const {
            return std::ranges::single_view {
                N {
                    node_layer::DIAGONAL,
                    I0,
                    I0
                }
            };
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_root_node */
        N get_root_node() const {
            return N {
                node_layer::DIAGONAL,
                I0,
                I0
            };
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_leaf_nodes */
        bidirectional_range_of_non_cvref<N> auto get_leaf_nodes() const {
            return std::ranges::single_view {
                N {
                    node_layer::DIAGONAL,
                    static_cast<N_INDEX>(grid_down_cnt - I1),  // Cast to prevent narrowing warning
                    static_cast<N_INDEX>(grid_right_cnt - I1)  // Cast to prevent narrowing warning
                }
            };
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_leaf_node */
        N get_leaf_node() const {
            return N {
                node_layer::DIAGONAL,
                static_cast<N_INDEX>(grid_down_cnt - I1),  // Cast to prevent narrowing warning
                static_cast<N_INDEX>(grid_right_cnt - I1)  // Cast to prevent narrowing warning
            };
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_nodes */
        bidirectional_range_of_non_cvref<N> auto get_nodes() const {
            auto diagonal_layer_nodes {
                std::views::cartesian_product(
                    std::views::iota(I0, grid_down_cnt),
                    std::views::iota(I0, grid_right_cnt)
                )
                | std::views::transform([](const auto & p) {
                    const auto &[grid_down, grid_right] { p };
                    return N { node_layer::DIAGONAL, grid_down, grid_right };
                })
            };
            auto down_layer_nodes {
                std::views::cartesian_product(
                    std::views::iota(I1, grid_down_cnt),
                    std::views::iota(I0, grid_right_cnt)
                )
                | std::views::transform([](const auto & p) {
                    const auto &[grid_down, grid_right] { p };
                    return N { node_layer::DOWN, grid_down, grid_right };
                })
            };
            auto right_layer_nodes {
                std::views::cartesian_product(
                    std::views::iota(I0, grid_down_cnt),
                    std::views::iota(I1, grid_right_cnt)
                )
                | std::views::transform([](const auto & p) {
                    const auto &[grid_down, grid_right] { p };
                    return N { node_layer::RIGHT, grid_down, grid_right };
                })
            };
            return concat_bidirectional_view(
                std::move(diagonal_layer_nodes),
                concat_bidirectional_view(
                    std::move(down_layer_nodes),
                    std::move(right_layer_nodes)
                )
            );
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_edges */
        bidirectional_range_of_non_cvref<E> auto get_edges() const {
            auto diagonal_layer_edges {
                std::views::cartesian_product(
                    std::array<node_layer, 3zu> { node_layer::DIAGONAL, node_layer::DOWN, node_layer::RIGHT },
                    std::views::iota(I0, grid_down_cnt),
                    std::views::iota(I0, grid_right_cnt)
                )
                | std::views::transform([](const auto& tuple) {
                    const auto& [n2_layer, n1_grid_down, n1_grid_right] { tuple };
                    const N n1 { node_layer::DIAGONAL, n1_grid_down, n1_grid_right };
                    if (n2_layer == node_layer::DIAGONAL) {
                        return E {
                            n1,
                            N {
                                n2_layer,
                                static_cast<N_INDEX>(n1_grid_down + I1),  // Cast to prevent narrowing warning
                                static_cast<N_INDEX>(n1_grid_right + I1)  // Cast to prevent narrowing warning
                            }
                        };
                    } else if (n2_layer == node_layer::DOWN) {
                        return E {
                            n1,
                            N {
                                n2_layer,
                                static_cast<N_INDEX>(n1_grid_down + I1),  // Cast to prevent narrowing warning
                                n1_grid_right
                            }
                        };
                    } else if (n2_layer == node_layer::RIGHT) {
                        return E {
                            n1,
                            N {
                                n2_layer,
                                n1_grid_down,
                                static_cast<N_INDEX>(n1_grid_right + I1)  // Cast to prevent narrowing warning
                            }
                        };
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
                    std::views::iota(I1, grid_down_cnt),
                    std::views::iota(I0, grid_right_cnt)
                )
                | std::views::transform([](const auto& tuple) {
                    const auto& [n2_layer, n1_grid_down, n1_grid_right] { tuple };
                    const N n1 { node_layer::DOWN, n1_grid_down, n1_grid_right };
                    if (n2_layer == node_layer::DIAGONAL) {
                        return E {
                            n1,
                            N {
                                n2_layer,
                                n1_grid_down,
                                n1_grid_right
                            }
                        };
                    } else if (n2_layer == node_layer::DOWN) {
                        return E {
                            n1,
                            N {
                                n2_layer,
                                static_cast<N_INDEX>(n1_grid_down + I1),  // Cast to prevent narrowing warning
                                n1_grid_right
                            }
                        };
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
                    std::views::iota(I0, grid_down_cnt),
                    std::views::iota(I1, grid_right_cnt)
                )
                | std::views::transform([](const auto& tuple) {
                    const auto& [n2_layer, n1_grid_down, n1_grid_right] { tuple };
                    const N n1 { node_layer::RIGHT, n1_grid_down, n1_grid_right };
                    if (n2_layer == node_layer::DIAGONAL) {
                        return E {
                            n1,
                            N {
                                n2_layer,
                                n1_grid_down,
                                n1_grid_right
                            }
                        };
                    } else if (n2_layer == node_layer::RIGHT) {
                        return E {
                            n1,
                            N {
                                n2_layer,
                                n1_grid_down,
                                static_cast<N_INDEX>(n1_grid_right + I1)  // Cast to prevent narrowing warning
                            }
                        };
                    }
                    std::unreachable();
                })
                | std::views::filter([this](const E& edge) {
                    return has_edge(edge);
                })
            };
            static_assert(std::ranges::bidirectional_range<decltype(right_layer_edges)>);
            return concat_bidirectional_view(
                std::move(diagonal_layer_edges),
                concat_bidirectional_view(
                    std::move(down_layer_edges),
                    std::move(right_layer_edges)
                )
            );
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::has_node */
        bool has_node(const N& n) const {
            const auto& [n_layer, grid_down, grid_right] { n };
            return (n_layer == node_layer::DIAGONAL
                    && grid_down < grid_down_cnt && grid_down >= I0
                    && grid_right < grid_right_cnt && grid_right >= I0)
                || (n_layer == node_layer::DOWN
                    && grid_down < grid_down_cnt && grid_down >= I1
                    && grid_right < grid_right_cnt && grid_right >= I0)
                || (n_layer == node_layer::RIGHT
                    && grid_down < grid_down_cnt && grid_down >= I0
                    && grid_right < grid_right_cnt && grid_right >= I1);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::has_edge */
        bool has_edge(const E& e) const {
            const auto& [n1_layer, n1_grid_down, n1_grid_right] { e.source };
            const auto& [n2_layer, n2_grid_down, n2_grid_right] { e.destination };
            return (n1_layer == node_layer::DIAGONAL && n2_layer == node_layer::DIAGONAL
                    && n1_grid_down + I1 == n2_grid_down && n1_grid_right + I1 == n2_grid_right
                    && n2_grid_down < grid_down_cnt && n2_grid_right < grid_right_cnt)  // match
                || (n1_layer == node_layer::DIAGONAL && n2_layer == node_layer::DOWN
                    && n1_grid_down + I1 == n2_grid_down && n1_grid_right == n2_grid_right
                    && n2_grid_down < grid_down_cnt && n2_grid_right < grid_right_cnt)  // initial gap (down)
                || (n1_layer == node_layer::DIAGONAL && n2_layer == node_layer::RIGHT
                    && n1_grid_down == n2_grid_down && n1_grid_right + I1 == n2_grid_right
                    && n2_grid_down < grid_down_cnt && n2_grid_right < grid_right_cnt) // initial gap (right)
                || (n1_layer == node_layer::DOWN && n2_layer == node_layer::DOWN
                    && n1_grid_down > I0 && n1_grid_down + I1 == n2_grid_down && n1_grid_right == n2_grid_right
                    && n2_grid_down < grid_down_cnt && n2_grid_right < grid_right_cnt)  // extended gap (down)
                || (n1_layer == node_layer::RIGHT && n2_layer == node_layer::RIGHT
                    && n1_grid_right > I0 && n1_grid_down == n2_grid_down && n1_grid_right + I1 == n2_grid_right
                    && n2_grid_down < grid_down_cnt && n2_grid_right < grid_right_cnt) // extended gap (right)
                || (n1_layer == node_layer::DOWN && n2_layer == node_layer::DIAGONAL
                    && n1_grid_down > I0 && n1_grid_down == n2_grid_down && n1_grid_right == n2_grid_right
                    && n2_grid_down < grid_down_cnt && n2_grid_right < grid_right_cnt)  // freeride (down)
                || (n1_layer == node_layer::RIGHT && n2_layer == node_layer::DIAGONAL
                    && n1_grid_right > I0 && n1_grid_down == n2_grid_down && n1_grid_right == n2_grid_right
                    && n2_grid_down < grid_down_cnt && n2_grid_right < grid_right_cnt); // freeride (right)
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_outputs_full */
        full_input_output_range<N, E, ED> auto get_outputs_full(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            typename static_vector_typer<debug_mode, std::tuple<E, N, N, ED>, 3zu>::type ret {};
            const auto& [n1_layer, n1_grid_down, n1_grid_right] { n };
            if (n1_layer == node_layer::DIAGONAL) {
                if (n1_grid_down == grid_down_cnt - I1 && n1_grid_right == grid_right_cnt - I1) {
                    // do nothing
                } else if (n1_grid_down < grid_down_cnt - I1 && n1_grid_right < grid_right_cnt - I1) {
                    ret.push_back(
                        construct_full_edge(
                            n,
                            {
                                node_layer::DIAGONAL,
                                static_cast<N_INDEX>(n1_grid_down + I1),  // Cast to prevent narrowing warning
                                static_cast<N_INDEX>(n1_grid_right + I1)  // Cast to prevent narrowing warning
                            }
                        )
                    );
                    ret.push_back(
                        construct_full_edge(
                            n,
                            {
                                node_layer::DOWN,
                                static_cast<N_INDEX>(n1_grid_down + I1),  // Cast to prevent narrowing warning
                                n1_grid_right
                            }
                        )
                    );
                    ret.push_back(
                        construct_full_edge(
                            n,
                            {
                                node_layer::RIGHT,
                                n1_grid_down,
                                static_cast<N_INDEX>(n1_grid_right + I1)
                            }
                        )
                    );
                } else if (n1_grid_right == grid_right_cnt - I1) {
                    ret.push_back(
                        construct_full_edge(
                            n,
                            {
                                node_layer::DOWN,
                                static_cast<N_INDEX>(n1_grid_down + I1),  // Cast to prevent narrowing warning
                                n1_grid_right
                            }
                        )
                    );
                } else if (n1_grid_down == grid_down_cnt - I1) {
                    ret.push_back(
                        construct_full_edge(
                            n,
                            {
                                node_layer::RIGHT,
                                n1_grid_down,
                                static_cast<N_INDEX>(n1_grid_right + I1)  // Cast to prevent narrowing warning
                            }
                        )
                    );
                }
            } else if (n1_layer == node_layer::DOWN) {
                ret.push_back(
                    construct_full_edge(
                        n,
                        {
                            node_layer::DIAGONAL,
                            n1_grid_down,
                            n1_grid_right
                        }
                    )
                );
                if (n1_grid_down < grid_down_cnt - I1) {
                    ret.push_back(
                        construct_full_edge(
                            n, {
                                node_layer::DOWN,
                                static_cast<N_INDEX>(n1_grid_down + I1),  // Cast to prevent narrowing warning
                                n1_grid_right
                            }
                        )
                    );
                }
            } else if (n1_layer == node_layer::RIGHT) {
                ret.push_back(
                    construct_full_edge(
                        n,
                        {
                            node_layer::DIAGONAL,
                            n1_grid_down,
                            n1_grid_right
                        }
                    )
                );
                if (n1_grid_right < grid_right_cnt - I1) {
                    ret.push_back(
                        construct_full_edge(
                            n,
                            {
                                node_layer::RIGHT,
                                n1_grid_down,
                                static_cast<N_INDEX>(n1_grid_right + I1)
                            }
                        )
                    );
                }
            }
            return ret;
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_inputs_full */
        full_input_output_range<N, E, ED> auto get_inputs_full(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            typename static_vector_typer<debug_mode, std::tuple<E, N, N, ED>, 3zu>::type ret {};
            const auto& [n2_layer, n2_grid_down, n2_grid_right] { n };
            if (n2_layer == node_layer::DIAGONAL) {
                if (n2_grid_down == I0 && n2_grid_right == I0) {
                    // do nothing
                } else if (n2_grid_down > I0 && n2_grid_right > I0) {
                    ret.push_back(
                        construct_full_edge(
                            {
                                node_layer::DIAGONAL,
                                static_cast<N_INDEX>(n2_grid_down - I1),  // Cast to prevent narrowing warning
                                static_cast<N_INDEX>(n2_grid_right - I1)  // Cast to prevent narrowing warning
                            },
                            n
                        )
                    );
                    ret.push_back(
                        construct_full_edge(
                            {
                                node_layer::DOWN,
                                n2_grid_down,
                                n2_grid_right
                            },
                            n
                        )
                    );
                    ret.push_back(
                        construct_full_edge(
                            {
                                node_layer::RIGHT,
                                n2_grid_down,
                                n2_grid_right
                            },
                            n
                        )
                    );
                } else if (n2_grid_right > I0) {
                    ret.push_back(
                        construct_full_edge(
                            {
                                node_layer::RIGHT,
                                n2_grid_down,
                                n2_grid_right
                            },
                            n
                        )
                    );
                } else if (n2_grid_down > I0) {
                    ret.push_back(
                        construct_full_edge(
                            {
                                node_layer::DOWN,
                                n2_grid_down,
                                n2_grid_right
                            },
                            n
                        )
                    );
                }
            } else if (n2_layer == node_layer::DOWN) {
                if (n2_grid_down > I0) {
                    ret.push_back(
                        construct_full_edge(
                            {
                                node_layer::DIAGONAL,
                                static_cast<N_INDEX>(n2_grid_down - I1),  // Cast to prevent narrowing warning
                                n2_grid_right
                            },
                            n
                        )
                    );
                }
                if (n2_grid_down > I1) {
                    ret.push_back(
                        construct_full_edge(
                            {
                                node_layer::DOWN,
                                static_cast<N_INDEX>(n2_grid_down - I1),  // Cast to prevent narrowing warning
                                n2_grid_right
                            },
                            n
                        )
                    );
                }
            } else if (n2_layer == node_layer::RIGHT) {
                if (n2_grid_right > I0) {
                    ret.push_back(
                        construct_full_edge(
                            {
                                node_layer::DIAGONAL,
                                n2_grid_down,
                                static_cast<N_INDEX>(n2_grid_right - I1)  // Cast to prevent narrowing warning
                            },
                            n
                        )
                    );
                }
                if (n2_grid_right > I1) {
                    ret.push_back(
                        construct_full_edge(
                            {
                                node_layer::RIGHT,
                                n2_grid_down,
                                static_cast<N_INDEX>(n2_grid_right - I1)  // Cast to prevent narrowing warning
                            },
                            n
                        )
                    );
                }
            }
            return ret;
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_outputs */
        bidirectional_range_of_non_cvref<E> auto get_outputs(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return this->get_outputs_full(n) | std::views::transform([this](const auto& v) -> E { return std::get<0zu>(v); });
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_inputs */
        bidirectional_range_of_non_cvref<E> auto get_inputs(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return this->get_inputs_full(n) | std::views::transform([this](const auto& v) -> E { return std::get<0zu>(v); });
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::has_outputs */
        bool has_outputs(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return this->get_outputs(n).size() > I0;
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::has_inputs */
        bool has_inputs(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return this->get_inputs(n).size() > I0;
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_out_degree */
        std::size_t get_out_degree(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return this->get_outputs(n).size();
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_in_degree */
        std::size_t get_in_degree(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return this->get_inputs(n).size();
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

            const auto& [n1, n2] { e };
            const auto& [n1_layer, n1_grid_down, n1_grid_right] { n1 };
            const auto& [n2_layer, n2_grid_down, n2_grid_right] { n2 };
            if (n1_layer == node_layer::DIAGONAL && n2_layer == node_layer::DIAGONAL) {  // match
                if (n1_grid_down + I1 == n2_grid_down && n1_grid_right + I1 == n2_grid_right) {
                    return RET { { { n1_grid_down }, { n1_grid_right } } };
                }
            } else if ((n1_layer == node_layer::DOWN && n2_layer == node_layer::DOWN)  // extended indel
                || (n1_layer == node_layer::DIAGONAL && n2_layer == node_layer::DOWN)) {  // indel
                if (n1_grid_down + I1 == n2_grid_down && n1_grid_right == n2_grid_right) {
                    return RET { { { n1_grid_down }, std::nullopt } };
                }
            } else if ((n1_layer == node_layer::RIGHT && n2_layer == node_layer::RIGHT)  // extended indel
                || (n1_layer == node_layer::DIAGONAL && n2_layer == node_layer::RIGHT)) {  // indel
                if (n1_grid_down == n2_grid_down && n1_grid_right + I1 == n2_grid_right) {
                    return RET { { std::nullopt, { n1_grid_right } } };
                }
            } else if ((n1_layer == node_layer::DOWN && n2_layer == node_layer::DIAGONAL)  // freeride
                || (n1_layer == node_layer::RIGHT && n2_layer == node_layer::DIAGONAL)) {  // freeride
                return RET { std::nullopt };
            }
            if constexpr (debug_mode) {
                throw std::runtime_error { "This should never happen" };
            }
            std::unreachable();
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::node_to_grid_offset */
        std::tuple<N_INDEX, N_INDEX, std::size_t> node_to_grid_offset(const N& n) const {
            const auto& [layer, down_offset, right_offset] { n };
            return { down_offset, right_offset, static_cast<std::size_t>(layer) };
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::grid_offset_to_nodes */
        bidirectional_range_of_non_cvref<N> auto grid_offset_to_nodes(N_INDEX grid_down, N_INDEX grid_right) const {
            if constexpr (debug_mode) {
                if (grid_down >= grid_down_cnt || grid_right >= grid_right_cnt) {
                    throw std::runtime_error { "Out of bounds" };
                }
            }
            using CONTAINER = static_vector_typer<debug_mode, N, 3zu>::type;
            CONTAINER ret {};
            N n1 { node_layer::DOWN, grid_down, grid_right };
            if (has_node(n1)) {
                ret.push_back(n1);
            }
            N n2 { node_layer::RIGHT, grid_down, grid_right };
            if (has_node(n2)) {
                ret.push_back(n2);
            }
            N n3 { node_layer::DIAGONAL, grid_down, grid_right };
            if (has_node(n3)) {
                ret.push_back(n3);
            }
            return ret;
            // THE FOLLOWING IS NOT POSSIBLE BECAUSE IT RESULTS IN A FORWARD RANGE, NOT A BIDIRECTIONAL RANGE:
            // return
            //     std::array<N, 3zu> {
            //         N { node_layer::DOWN, grid_down, grid_right },
            //         N { node_layer::RIGHT, grid_down, grid_right },
            //         N { node_layer::DIAGONAL, grid_down, grid_right }
            //     }
            //     | std::views::filter([this](const N& n) { return has_node(n); });
        }

    private:
        static N row_next_node_(const N& node) {
            const auto& [layer_, grid_down, grid_right] { node };
            N next_node;
            if (grid_down == I0 && grid_right == I0 && layer_ == node_layer::DIAGONAL) {
                next_node = {
                    node_layer::RIGHT,
                    grid_down,
                    static_cast<N_INDEX>(grid_right + I1)  // Cast to prevent narrowing warning
                };
            } else if (grid_down == I0 && layer_ == node_layer::RIGHT) {
                next_node = {
                    node_layer::DIAGONAL,
                    grid_down,
                    grid_right
                };
            } else if (grid_down == I0 && layer_ == node_layer::DIAGONAL) {
                next_node = {
                    node_layer::RIGHT,
                    grid_down,
                    static_cast<N_INDEX>(grid_right + I1)  // Cast to prevent narrowing warning
                };
            } else if (grid_right == I0 && layer_ == node_layer::DOWN) {
                next_node = {
                    node_layer::DIAGONAL,
                    grid_down,
                    grid_right
                };
            } else if (grid_right == I0 && layer_ == node_layer::DIAGONAL) {
                next_node = {
                    node_layer::DOWN,
                    grid_down,
                    static_cast<N_INDEX>(grid_right + I1)  // Cast to prevent narrowing warning
                };
            } else if (layer_ == node_layer::DOWN) {
                next_node = {
                    node_layer::RIGHT,
                    grid_down,
                    grid_right
                };
            } else if (layer_ == node_layer::RIGHT) {
                next_node = {
                    node_layer::DIAGONAL,
                    grid_down,
                    grid_right
                };
            } else if (layer_ == node_layer::DIAGONAL) {
                next_node = {
                    node_layer::DOWN,
                    grid_down,
                    static_cast<N_INDEX>(grid_right + I1)  // Cast to prevent narrowing warning
                };
            } else {
                if constexpr (debug_mode) {
                    throw std::runtime_error { "This should never happen" };
                }
            }
            return next_node;
        }

        static N row_prev_node_(const N& node) {
            const auto& [layer_, grid_down, grid_right] { node };
            N prev_node;
            if (grid_down == I0 && grid_right == I1 && layer_ == node_layer::RIGHT) {
                prev_node = {
                    node_layer::DIAGONAL,
                    grid_down,
                    static_cast<N_INDEX>(grid_right - I1)  // Cast to prevent narrowing warning
                };
            } else if (grid_down == I0 && layer_ == node_layer::DIAGONAL) {
                prev_node = {
                    node_layer::RIGHT,
                    grid_down,
                    grid_right
                };
            } else if (grid_down == I0 && layer_ == node_layer::RIGHT) {
                prev_node = {
                    node_layer::DIAGONAL,
                    grid_down,
                    static_cast<N_INDEX>(grid_right - I1)  // Cast to prevent narrowing warning
                };
            } else if (grid_right == I0 && layer_ == node_layer::DIAGONAL) {
                prev_node = {
                    node_layer::DOWN,
                    grid_down,
                    grid_right
                };
            } else if (grid_right == I0 && layer_ == node_layer::DOWN) {
                prev_node = {
                    node_layer::DIAGONAL,
                    grid_down,
                    static_cast<N_INDEX>(grid_right - I1)  // Cast to prevent narrowing warning
                };
            } else if (layer_ == node_layer::DOWN) {
                prev_node = {
                    node_layer::DIAGONAL,
                    grid_down,
                    static_cast<N_INDEX>(grid_right - I1)  // Cast to prevent narrowing warning
                };
            } else if (layer_ == node_layer::RIGHT) {
                prev_node = {
                    node_layer::DOWN,
                    grid_down,
                    grid_right
                };
            } else if (layer_ == node_layer::DIAGONAL) {
                prev_node = {
                    node_layer::RIGHT,
                    grid_down,
                    grid_right
                };
            } else {
                if constexpr (debug_mode) {
                    throw std::runtime_error { "This should never happen" };
                }
            }
            return prev_node;
        }

    public:
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::row_nodes */
        bidirectional_range_of_non_cvref<N> auto row_nodes(N_INDEX grid_down) const {
            return row_nodes(grid_down, get_root_node(), get_leaf_node());
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::row_nodes */
        bidirectional_range_of_non_cvref<N> auto row_nodes(N_INDEX grid_down, const N& root_node, const N& leaf_node) const {
            if constexpr (debug_mode) {
                if (!has_node(root_node) || !has_node(leaf_node)) {
                    throw std::runtime_error { "Bad node" };
                }
                if (!(root_node.down <= leaf_node.down)) {
                    throw std::runtime_error { "Bad node" };
                }
                if (!(root_node.right <= leaf_node.right)) {
                    throw std::runtime_error { "Bad node" };
                }
                // if single node in graph, make sure depth order is satisifed same: DOWN RIGHT DIAGONAL
                if (
                    (root_node.down == leaf_node.down)
                    && (root_node.right == leaf_node.right)
                    && !(root_node.layer <= leaf_node.layer)
                ) {
                    throw std::runtime_error { "Bad node" };
                }
                if (!(grid_down >= root_node.down && grid_down <= leaf_node.down)) {
                    throw std::runtime_error { "Bad node" };
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
                    if (grid_down == I0 && root_right == I0) {
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
                end_node = row_next_node_(end_node);  // +1, because iterator.end() should be 1 past the last element
            }

            struct state {
                N value_;

                void to_prev() { value_ = row_prev_node_(value_); }
                void to_next() { value_ = row_next_node_(value_); }
                N value() const { return value_; }

                bool operator==(const state& other) const = default;
            };
            return simple_value_bidirectional_view<state> {
                state { begin_node },
                state { end_node }
            };
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::is_reachable */
        bool is_reachable(const N& n1, const N& n2) const {
            if constexpr (debug_mode) {
                if (!has_node(n1) || !has_node(n2)) {
                    throw std::runtime_error { "Node doesn't exist" };
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

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::resident_nodes */
        bidirectional_range_of_non_cvref<N> auto resident_nodes() const {
            return std::views::empty<N>;
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::outputs_to_residents */
        bidirectional_range_of_non_cvref<E> auto outputs_to_residents([[maybe_unused]] const N& n) const {
            return std::views::empty<E>;
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::inputs_from_residents */
        bidirectional_range_of_non_cvref<E> auto inputs_from_residents([[maybe_unused]] const N& n) const {
            return std::views::empty<E>;
        }
    };


    /**
     * Create @ref offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::pairwise_extended_gap_alignment_graph instance, where
     * template parameters are deduced / inferred from arguments passed in.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam N_INDEX Node coordinate type.
     * @param down_seq Downward sequence.
     * @param right_seq Rightward sequence.
     * @param substitution_scorer Scorer for sequence alignment substitutions.
     * @param initial_gap_scorer Scorer for initial sequence alignment gaps (indels).
     * @param extended_gap_scorer Scorer for extended sequence alignment gaps (indels).
     * @param freeride_scorer Score for sequence alignment freerides.
     * @return New @ref offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::pairwise_extended_gap_alignment_graph instance.
     */
    template<
        bool debug_mode,
        widenable_to_size_t N_INDEX
    >
    auto create_pairwise_extended_gap_alignment_graph(
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
        > auto& initial_gap_scorer,
        const scorer_without_explicit_weight<
            N_INDEX,
            std::remove_cvref_t<decltype(down_seq[0zu])>,
            std::remove_cvref_t<decltype(right_seq[0zu])>
        > auto& extended_gap_scorer,
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
            initial_gap_scorer(
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
            extended_gap_scorer(
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
        using WEIGHT_4 = decltype(
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
        static_assert(std::is_same_v<WEIGHT_1, WEIGHT_4>, "Scorers must return the same weight type");
        return pairwise_extended_gap_alignment_graph<
            debug_mode,
            N_INDEX,
            WEIGHT_1,
            DOWN_SEQ,
            RIGHT_SEQ,
            std::remove_cvref_t<decltype(substitution_scorer)>,
            std::remove_cvref_t<decltype(initial_gap_scorer)>,
            std::remove_cvref_t<decltype(extended_gap_scorer)>,
            std::remove_cvref_t<decltype(freeride_scorer)>
        > {
            down_seq,
            right_seq,
            substitution_scorer,
            initial_gap_scorer,
            extended_gap_scorer,
            freeride_scorer
        };
    }
}

// Struct must be defined outside of namespace block above, otherwise compiler will treat it as part of that namespace.
// NOTE: Inheriting from std::formatter<std::string_view> instead of std::formatter<std::string> because -Wabi-tag warning.
template<offbynull::concepts::widenable_to_size_t N_INDEX>
struct std::formatter<offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::node<N_INDEX>> : std::formatter<std::string_view> {
    auto format(
        const offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::node<N_INDEX>& n,
        std::format_context& ctx
    ) const {
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

template<offbynull::concepts::widenable_to_size_t N_INDEX>
std::ostream& operator<<(std::ostream& os, const offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::node<N_INDEX>& n) {
    return os << std::format("{}", n);
}

// Struct must be defined outside of namespace block above, otherwise compiler will treat it as part of that namespace.
// NOTE: Inheriting from std::formatter<std::string_view> instead of std::formatter<std::string> because -Wabi-tag warning.
template<offbynull::concepts::widenable_to_size_t N_INDEX>
struct std::formatter<offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::edge<N_INDEX>> : std::formatter<std::string_view> {
    auto format(
        const offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::edge<N_INDEX>& e,
        std::format_context& ctx
    ) const {
        return std::format_to(ctx.out(), "{}->{}", e.source, e.destination);
    }
};

template<offbynull::concepts::widenable_to_size_t N_INDEX>
std::ostream& operator<<(std::ostream& os, const offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::edge<N_INDEX>& e) {
    return os << std::format("{}", e);
}

#endif //OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_EXTENDED_GAP_ALIGNMENT_GRAPH_H
