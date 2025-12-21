#ifndef OFFBYNULL_ALIGNER_GRAPHS_GRID_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_GRID_GRAPH_H

#include <cstddef>
#include <string>
#include <string_view>
#include <ranges>
#include <tuple>
#include <stdexcept>
#include <utility>
#include <optional>
#include <type_traits>
#include <functional>
#include <ostream>
#include <format>
#include <limits>
#include "offbynull/concepts.h"
#include "offbynull/utils.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/sequence/sequence.h"
#include "offbynull/aligner/scorer/scorer.h"

namespace offbynull::aligner::graphs::grid_graph {
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::scorer::scorer::scorer;
    using offbynull::aligner::scorer::scorer::scorer_without_explicit_weight;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::utils::static_vector_typer;

    /** Node data type used by @ref offbynull::aligner::graphs::grid_graph::grid_graph, which is an empty type (no data kept for nodes). */
    using empty_node_data = std::tuple<>;

    PACK_STRUCT_START
    /**
     * Node ID type for @ref offbynull::aligner::graphs::grid_graph::grid_graph. Instances of this type uniquely identify a position on the
     * grid.
     *
     * Struct is packed when `OBN_PACK_STRUCTS` macro is defined (and platform supports struct packing).
     *
     * @tparam INDEX Node coordinate type (smaller integer types may reduce memory consumption).
     */
    template<widenable_to_size_t INDEX>
    struct node {
        /** Row / vertical position of node, starting from the top going downward. */
        INDEX down;
        /** Column / horizontal position of node, starting from the left going rightward. */
        INDEX right;
        /** Enable spaceship operator. */
        auto operator<=>(const node&) const = default;
    }
    PACK_STRUCT_STOP;

    PACK_STRUCT_START
    /**
     * Edge ID type for @ref offbynull::aligner::graphs::grid_graph::grid_graph.
     *
     * Struct is packed when `OBN_PACK_STRUCTS` macro is defined (and platform supports struct packing).
     *
     * @tparam INDEX Node coordinate type.
     */
    template<widenable_to_size_t INDEX>
    struct edge {
        /** Source node's ID. */
        node<INDEX> source;
        /** Destination node's ID. */
        node<INDEX> destination;
        /** Enable spaceship operator. */
        auto operator<=>(const edge&) const = default;
    }
    PACK_STRUCT_STOP;

    /**
     * @ref offbynull::aligner::graph::graph::graph implementation comprising the grid-like structure of a pairwise global sequence
     * alignment graph, intended to be used as a base for other more complex pairwise alignment graphs.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam INDEX_ Node coordinate type.
     * @tparam WEIGHT Edge weight type.
     * @tparam DOWN_SEQ Downward sequence type.
     * @tparam RIGHT_SEQ Rightward sequence type.
     * @tparam SUBSTITUTION_SCORER Scorer type used to score sequence alignment substitutions.
     * @tparam GAP_SCORER Scorer type to score for sequence alignment gaps (indels).
     */
    template<
        bool debug_mode,
        widenable_to_size_t INDEX_,
        weight WEIGHT,
        sequence DOWN_SEQ,
        sequence RIGHT_SEQ,
        scorer<
            edge<INDEX_>,
            INDEX_,
            std::remove_cvref_t<decltype(std::declval<DOWN_SEQ>()[0zu])>,
            std::remove_cvref_t<decltype(std::declval<RIGHT_SEQ>()[0zu])>,
            WEIGHT
        > SUBSTITUTION_SCORER,
        scorer<
            edge<INDEX_>,
            INDEX_,
            std::remove_cvref_t<decltype(std::declval<DOWN_SEQ>()[0zu])>,
            std::remove_cvref_t<decltype(std::declval<RIGHT_SEQ>()[0zu])>,
            WEIGHT
        > GAP_SCORER
    >
    class grid_graph {
    public:
        /** Element object type of downward sequence (CV-qualification and references removed). */
        using DOWN_ELEM = std::remove_cvref_t<decltype(std::declval<DOWN_SEQ>()[0u])>;
        /** Element object type of rightward sequence (CV-qualification and references removed). */
        using RIGHT_ELEM = std::remove_cvref_t<decltype(std::declval<RIGHT_SEQ>()[0u])>;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::INDEX */
        using INDEX = INDEX_;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::N */
        using N = node<INDEX>;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::ND */
        using ND = empty_node_data;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::E */
        using E = edge<INDEX>;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::ED */
        using ED = WEIGHT;

    private:
        /** Downward sequence. */
        const DOWN_SEQ& down_seq;
        /** Rightward sequence. */
        const RIGHT_SEQ& right_seq;
        /** Scorer used to score sequence alignment substitutions. */
        const SUBSTITUTION_SCORER substitution_scorer;
        /** Scorer used tos score sequence alignment gaps (indels). */
        const GAP_SCORER gap_scorer;

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
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::grid_down_cnt */
        const INDEX grid_down_cnt;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::grid_right_cnt */
        const INDEX grid_right_cnt;
        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::grid_depth_cnt */
        static constexpr INDEX grid_depth_cnt { 1u };
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
         * Construct an @ref offbynull::aligner::graphs::grid_graph::grid_graph instance.
         *
         * The behavior of this function / class is undefined if `down_seq_` or `right_seq_` has more elements than `INDEX_`'s maximum
         * possible value.
         *
         * @param down_seq_ Downward sequence.
         * @param right_seq_ Rightward sequence.
         * @param substitution_scorer_ Scorer for sequence alignment substitutions.
         * @param gap_scorer_ Scorer for sequence alignment gaps (indels).
         */
        grid_graph(
            const DOWN_SEQ& down_seq_,
            const RIGHT_SEQ& right_seq_,
            const SUBSTITUTION_SCORER& substitution_scorer_,
            const GAP_SCORER& gap_scorer_
        )
        : down_seq { down_seq_ }
        , right_seq { right_seq_ }
        , substitution_scorer { substitution_scorer_ } // Copying object, not the ref
        , gap_scorer { gap_scorer_ } // Copying object, not the ref
        , grid_down_cnt { down_seq_.size() + 1zu }
        , grid_right_cnt { right_seq_.size() + 1zu }
        , path_edge_capacity { (grid_right_cnt - 1u) + (grid_down_cnt - 1u) }
        , node_incoming_edge_capacity { grid_down_cnt == 1zu || grid_right_cnt == 1zu ? 1zu : 3zu }
        , node_outgoing_edge_capacity { grid_down_cnt == 1zu || grid_right_cnt == 1zu ? 1zu : 3zu } {
            if constexpr (debug_mode) {
                if (down_seq_.size() + 1zu >= std::numeric_limits<INDEX>::max()
                        || right_seq_.size() + 1zu  >= std::numeric_limits<INDEX>::max()) {
                    throw std::runtime_error { "Sequence too large for index type" };
                }
            }
        }

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
        ED get_edge_data(const E& e) const  {
            if constexpr (debug_mode) {
                if (!has_edge(e)) {
                    throw std::runtime_error { "Edge doesn't exist" };
                }
            }
            const N& n1 { e.source };
            const N& n2 { e.destination };
            if (n1.down == n2.down && n1.right + 1u == n2.right) {
                return gap_scorer(
                    e,
                    { std::nullopt },
                    { { n1.right, { right_seq[n1.right] } } }
                );
            } else if (n1.down + 1u == n2.down && n1.right == n2.right) {
                return gap_scorer(
                    e,
                    { { n1.down, { down_seq[n1.down] } } },
                    { std::nullopt }
                );
            } else if (n1.down + 1u == n2.down && n1.right + 1u == n2.right) {
                return substitution_scorer(
                    e,
                    { { n1.down, { down_seq[n1.down] } } },
                    { { n1.right, { right_seq[n1.right] } } }
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
            return std::tuple<N, N, ED> { this->get_edge_from(e), this->get_edge_to(e), this->get_edge_data(e) };
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_root_nodes */
        auto get_root_nodes() const {
            return std::ranges::single_view { N { 0u, 0u } };
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_root_node */
        N get_root_node() const {
            return N { 0u, 0u };
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_leaf_nodes */
        auto get_leaf_nodes() const {
            return std::ranges::single_view { N { grid_down_cnt - 1u, grid_right_cnt - 1u } };
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_leaf_node */
        N get_leaf_node() const {
            return N { grid_down_cnt - 1u, grid_right_cnt - 1u };
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_nodes */
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

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_edges */
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

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::has_node */
        bool has_node(const N& n) const {
            return n.down < grid_down_cnt && n.down >= 0u && n.right < grid_right_cnt && n.right >= 0u;
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::has_edge */
        bool has_edge(const E& e) const {
            const N& n1 { e.source };
            const N& n2 { e.destination };
            return (n1.down == n2.down && n1.right + 1u == n2.right && n2.right < grid_right_cnt)
                || (n1.down + 1u == n2.down && n1.right == n2.right && n2.down < grid_down_cnt)
                || (n1.down + 1u == n2.down && n1.right + 1u == n2.right && n2.down < grid_down_cnt && n2.right < grid_right_cnt);
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_outputs_full */
        std::ranges::bidirectional_range auto get_outputs_full(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            // Cartesian product has some issues with bloat, so not using it here:
            return
                std::views::cartesian_product(
                    std::views::iota(static_cast<INDEX>(0), static_cast<INDEX>(2)),
                    std::views::iota(static_cast<INDEX>(0), static_cast<INDEX>(2))
                )
                | std::views::drop(1)
                | std::views::filter([node = n, this](const auto& offset) {
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
                | std::views::transform([node = n, this](const auto& offset) {
                    const auto& [down_offset, right_offset] { offset };
                    const auto& [grid_down, grid_right] { node };
                    N n2 { grid_down + down_offset, grid_right + right_offset };
                    return this->construct_full_edge(node, n2);
                });
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_inputs_full */
        auto get_inputs_full(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return
                std::views::cartesian_product(
                    std::views::iota(static_cast<INDEX>(0), static_cast<INDEX>(2)),
                    std::views::iota(static_cast<INDEX>(0), static_cast<INDEX>(2))
                )
                | std::views::drop(1)
                | std::views::filter([node = n, this](const auto& offset) {
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
                | std::views::transform([node = n, this](const auto& offset) {
                    const auto& [down_offset, right_offset] { offset };
                    const auto& [grid_down, grid_right] { node };
                    N n1 { grid_down - down_offset, grid_right - right_offset };
                    return this->construct_full_edge(n1, node);
                });
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_outputs */
        std::ranges::bidirectional_range auto get_outputs(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return this->get_outputs_full(n)
                | std::views::transform([](const auto& v) -> E { return std::get<0>(v); });
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::get_inputs */
        auto get_inputs(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return this->get_inputs_full(n)
                | std::views::transform([](const auto& v) -> E { return std::get<0>(v); });
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::has_outputs */
        bool has_outputs(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return this->get_out_degree(n) > 0zu;
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::has_inputs */
        bool has_inputs(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return this->get_in_degree(n) > 0zu;
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

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::node_to_grid_offset */
        std::tuple<INDEX, INDEX, std::size_t> node_to_grid_offset(const N& n) const {
            const auto& [down_offset, right_offset] { n };
            return { down_offset, right_offset, 0zu };
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::grid_offset_to_nodes */
        auto grid_offset_to_nodes(INDEX grid_down, INDEX grid_right) const {
            if constexpr (debug_mode) {
                if (grid_down >= grid_down_cnt || grid_right >= grid_right_cnt) {
                    throw std::runtime_error { "Out of bounds" };
                }
            }
            return std::views::single(N { grid_down, grid_right });
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::row_nodes */
        auto row_nodes(INDEX grid_down) const {
            return row_nodes(grid_down, get_root_node(), get_leaf_node());
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::row_nodes */
        auto row_nodes(INDEX grid_down, const N& root_node, const N& leaf_node) const {
            if constexpr (debug_mode) {
                if (!has_node(root_node) || !has_node(leaf_node)) {
                    throw std::runtime_error { "Bad node" };
                }
                if (!(root_node <= leaf_node)) {
                    throw std::runtime_error { "Bad node" };
                }
                if (!(grid_down >= root_node.down && grid_down <= leaf_node.down)) {
                    throw std::runtime_error { "Bad node" };
                }
            }
            return std::views::iota(root_node.right, leaf_node.right + 1u)
                | std::views::transform([grid_down](const auto& grid_right) { return N { grid_down, grid_right }; });
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::is_reachable */
        bool is_reachable(const N& n1, const N& n2) const {
            if constexpr (debug_mode) {
                if (!has_node(n1) || !has_node(n2)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return n1 <= n2;
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::resident_nodes */
        auto resident_nodes() const {
            return std::views::empty<N>;
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::outputs_to_residents */
        auto outputs_to_residents([[maybe_unused]] const N& n) const {
            return std::views::empty<E>;
        }

        /** @copydoc offbynull::aligner::graph::sliceable_pairwise_alignment_graph::unimplemented_sliceable_pairwise_alignment_graph::inputs_from_residents */
        auto inputs_from_residents([[maybe_unused]] const N& n) const {
            return std::views::empty<E>;
        }
    };

    /**
     * Create @ref offbynull::aligner::graphs::grid_graph::grid_graph instance, where template parameters are deduced / inferred from
     * arguments passed in.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam INDEX Node coordinate type.
     * @param down_seq Downward sequence.
     * @param right_seq Rightward sequence.
     * @param substitution_scorer Scorer for sequence alignment substitutions.
     * @param gap_scorer Scorer for sequence alignment gaps (indels).
     * @return New @ref offbynull::aligner::graphs::grid_graph::grid_graph instance.
     */
    template<
        bool debug_mode,
        widenable_to_size_t INDEX
    >
    auto create_grid_graph(
        const sequence auto& down_seq,
        const sequence auto& right_seq,
        const scorer_without_explicit_weight<
            edge<INDEX>,
            INDEX,
            std::remove_cvref_t<decltype(down_seq[0zu])>,
            std::remove_cvref_t<decltype(right_seq[0zu])>
        > auto& substitution_scorer,
        const scorer_without_explicit_weight<
            edge<INDEX>,
            INDEX,
            std::remove_cvref_t<decltype(down_seq[0zu])>,
            std::remove_cvref_t<decltype(right_seq[0zu])>
        > auto& gap_scorer
    ) {
        using DOWN_SEQ = std::remove_cvref_t<decltype(down_seq)>;
        using DOWN_ELEM = std::remove_cvref_t<decltype(down_seq[0zu])>;
        using RIGHT_SEQ = std::remove_cvref_t<decltype(down_seq)>;
        using RIGHT_ELEM = std::remove_cvref_t<decltype(right_seq[0zu])>;
        using WEIGHT_1 = decltype(
            substitution_scorer(
                std::declval<const edge<INDEX>&>(),
                std::declval<
                    const std::optional<
                        std::pair<
                            INDEX,
                            std::reference_wrapper<const DOWN_ELEM>
                        >
                    >
                >(),
                std::declval<
                    const std::optional<
                        std::pair<
                            INDEX,
                            std::reference_wrapper<const RIGHT_ELEM>
                        >
                    >
                >()
            )
        );
        using WEIGHT_2 = decltype(
            gap_scorer(
                std::declval<const edge<INDEX>&>(),
                std::declval<
                    const std::optional<
                        std::pair<
                            INDEX,
                            std::reference_wrapper<const DOWN_ELEM>
                        >
                    >
                >(),
                std::declval<
                    const std::optional<
                        std::pair<
                            INDEX,
                            std::reference_wrapper<const RIGHT_ELEM>
                        >
                    >
                >()
            )
        );
        static_assert(std::is_same_v<WEIGHT_1, WEIGHT_2>, "Scorers must return the same weight type");
        return grid_graph<
            debug_mode,
            INDEX,
            WEIGHT_1,
            DOWN_SEQ,
            RIGHT_SEQ,
            std::remove_cvref_t<decltype(substitution_scorer)>,
            std::remove_cvref_t<decltype(gap_scorer)>
        > {
            down_seq,
            right_seq,
            substitution_scorer,
            gap_scorer
        };
    }
}


// Struct must be defined outside of namespace block above, otherwise compiler will treat it as part of that namespace.
// NOTE: Inheriting from std::formatter<std::string_view> instead of std::formatter<std::string> because -Wabi-tag warning.
template<offbynull::concepts::widenable_to_size_t INDEX>
struct std::formatter<offbynull::aligner::graphs::grid_graph::node<INDEX>> : std::formatter<std::string_view> {
    auto format(
        const offbynull::aligner::graphs::grid_graph::node<INDEX>& n,
        std::format_context& ctx
    ) const {
        return std::format_to(ctx.out(), "[{},{}]", n.down, n.right);
    }
};

template<offbynull::concepts::widenable_to_size_t INDEX>
std::ostream& operator<<(std::ostream& os, const offbynull::aligner::graphs::grid_graph::node<INDEX>& n) {
    return os << std::format("{}", n);
}

// Struct must be defined outside of namespace block above, otherwise compiler will treat it as part of that namespace.
// NOTE: Inheriting from std::formatter<std::string_view> instead of std::formatter<std::string> because -Wabi-tag warning.
template<offbynull::concepts::widenable_to_size_t INDEX>
struct std::formatter<offbynull::aligner::graphs::grid_graph::edge<INDEX>> : std::formatter<std::string_view> {
    auto format(
        const offbynull::aligner::graphs::grid_graph::edge<INDEX>& e,
        std::format_context& ctx
    ) const {
        return std::format_to(ctx.out(), "{}->{}", e.source, e.destination);
    }
};

template<offbynull::concepts::widenable_to_size_t INDEX>
std::ostream& operator<<(std::ostream& os, const offbynull::aligner::graphs::grid_graph::edge<INDEX>& e) {
    return os << std::format("{}", e);
}

#endif //OFFBYNULL_ALIGNER_GRAPHS_GRID_GRAPH_H
