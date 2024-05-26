#ifndef OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_EXTENDED_GAP_ALIGNMENT_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_EXTENDED_GAP_ALIGNMENT_GRAPH_H

#include <ranges>
#include <tuple>
#include <stdexcept>
#include <utility>
#include <functional>
#include <stdfloat>
#include "boost/container/static_vector.hpp"
#include "boost/container/small_vector.hpp"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/graph/grid_container_creator.h"
#include "offbynull/aligner/graph/grid_container_creators.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph {
    using offbynull::aligner::graph::grid_container_creator::grid_container_creator;
    using offbynull::aligner::graph::grid_container_creators::vector_grid_container_creator;
    using offbynull::aligner::concepts::weight;
    using offbynull::utils::concat_view;

    enum class layer : uint8_t {
        DIAGONAL,
        DOWN,
        RIGHT
    };

    template<typename ND, typename ED, std::unsigned_integral T>
    struct slot {
        ND down_nd;
        ND diagonal_nd;
        ND right_nd;
        ED to_next_diagonal_ed;  // match edges between nodes in the diagonal layer
        // gap edges between nodes in down/right layers all use the same ED obj, so not placed here
        // indel edges from diagonal layer to down/right layers all use the same ED obj, so not placed here
        // freeride edges from down/right layers to diagonal layer all use the same ED obj, so not placed here
    };

    template<
        typename ND_,
        typename ED_,
        std::unsigned_integral INDEX_ = unsigned int,
        grid_container_creator<INDEX_> SLOT_ALLOCATOR_ = vector_grid_container_creator<slot<ND_, ED_, INDEX_>, INDEX_, false>,
        bool error_check = true
    >
    class pairwise_extended_gap_alignment_graph {
    public:
        using INDEX = INDEX_;
        using N = std::tuple<layer, INDEX, INDEX>;
        using ND = ND_;
        using E = std::pair<N, N>;
        using ED = ED_;

    private:
        decltype(std::declval<SLOT_ALLOCATOR_>().create_objects(0u, 0u)) slots;
        ED extended_indel_ed;
        ED initial_indel_ed;
        ED freeride_ed;

        auto construct_full_edge(N n1, N n2) {
            return std::tuple<E, N, N, ED*> {
                E { n1, n2 },
                n1,
                n2,
                &this->get_edge_data(E { n1, n2 })
            };
        }

        size_t to_raw_idx(size_t n_down, size_t n_right) {
            return (n_down * right_node_cnt) + n_right;
        }

    public:
        const INDEX down_node_cnt;
        const INDEX right_node_cnt;

        pairwise_extended_gap_alignment_graph(
            INDEX _down_node_cnt,
            INDEX _right_node_cnt,
            ED initial_indel_data = {},
            ED extended_indel_data = {},
            ED freeride_data = {},
            SLOT_ALLOCATOR_ slot_container_creator = {}
        )
        : slots{slot_container_creator.create_objects(_down_node_cnt, _right_node_cnt)}
        , extended_indel_ed{extended_indel_data}
        , initial_indel_ed{initial_indel_data}
        , freeride_ed{freeride_data}
        , down_node_cnt{_down_node_cnt}
        , right_node_cnt{_right_node_cnt} {}

        void update_node_data(const N& node, ND&& data) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto [n_down, n_right] = node;
            if (node.type == layer::DIAGONAL) {
                this->slots[to_raw_idx(n_down, n_right)].diagonal_nd = std::forward<ND>(data);
            } else if (node.type == layer::DOWN) {
                this->slots[to_raw_idx(n_down, n_right)].down_nd = std::forward<ND>(data);
            } else if (node.type == layer::RIGHT) {
                this->slots[to_raw_idx(n_down, n_right)].right_nd = std::forward<ND>(data);
            }
        }

        ND& get_node_data(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto [n_down, n_right] = node;
            if (node.type == layer::DIAGONAL) {
                return this->slots[to_raw_idx(n_down, n_right)].diagonal_nd;
            } else if (node.type == layer::DOWN) {
                return this->slots[to_raw_idx(n_down, n_right)].down_nd;
            } else if (node.type == layer::RIGHT) {
                return this->slots[to_raw_idx(n_down, n_right)].right_nd;
            }
            std::unreachable();
        }

        void update_edge_data(const E& edge, ED&& data) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            const auto& [n1_layer, n1_down, n1_right] = edge.first;
            auto [n2_layer, n2_down, n2_right] = edge.second;
            if (n1_layer == layer::DIAGONAL && n2_layer == layer::DIAGONAL) {  // match
                this->slots[to_raw_idx(n1_down, n1_right)].to_next_diagonal_ed = std::forward<ED>(data);
            } else if (n1_layer == layer::DOWN && n2_layer == layer::DOWN) {  // gap
                extended_indel_ed = std::forward<ED>(data);
            } else if (n1_layer == layer::RIGHT && n2_layer == layer::RIGHT) {  // gap
                extended_indel_ed = std::forward<ED>(data);
            } else if (n1_layer == layer::DIAGONAL && n2_layer == layer::DOWN) {  // indel
                initial_indel_ed = std::forward<ED>(data);
            } else if (n1_layer == layer::DIAGONAL && n2_layer == layer::RIGHT) {  // indel
                initial_indel_ed = std::forward<ED>(data);
            } else if (n1_layer == layer::DOWN && n2_layer == layer::DIAGONAL) {  // freeride
                freeride_ed = std::forward<ED>(data);
            } else if (n1_layer == layer::RIGHT && n2_layer == layer::DIAGONAL) {  // freeride
                freeride_ed = std::forward<ED>(data);
            }
        }

        ED& get_edge_data(const E& edge) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            const auto& [n1_layer, n1_down, n1_right] = edge.first;
            auto [n2_layer, n2_down, n2_right] = edge.second;
            if (n1_layer == layer::DIAGONAL && n2_layer == layer::DIAGONAL) {  // match
                return this->slots[to_raw_idx(n1_down, n1_right)].to_next_diagonal_ed;
            } else if (n1_layer == layer::DOWN && n2_layer == layer::DOWN) {  // gap
                return extended_indel_ed;
            } else if (n1_layer == layer::RIGHT && n2_layer == layer::RIGHT) {  // gap
                return extended_indel_ed;
            } else if (n1_layer == layer::DIAGONAL && n2_layer == layer::DOWN) {  // indel
                return initial_indel_ed;
            } else if (n1_layer == layer::DIAGONAL && n2_layer == layer::RIGHT) {  // indel
                return initial_indel_ed;
            } else if (n1_layer == layer::DOWN && n2_layer == layer::DIAGONAL) {  // freeride
                return freeride_ed;
            } else if (n1_layer == layer::RIGHT && n2_layer == layer::DIAGONAL) {  // freeride
                return freeride_ed;
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

        std::tuple<N, N, ED&> get_edge(const E& edge) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            return std::tuple<N, N, ED&> {this->get_edge_from(edge), this->get_edge_from(edge), this->get_edge_data(edge)};
        }

        auto get_root_nodes() {
            return std::ranges::single_view { N { layer::DIAGONAL, 0u, 0u } };
        }

        N get_root_node() {
            return N { layer::DIAGONAL, 0u, 0u };
        }

        auto get_leaf_nodes() {
            return std::ranges::single_view { N { layer::DIAGONAL, down_node_cnt - 1u, right_node_cnt - 1u } };
        }

        auto get_leaf_node() {
            return N { layer::DIAGONAL, down_node_cnt - 1u, right_node_cnt - 1u };
        }

        auto get_nodes() {
            auto diagonal_layer_nodes {
                std::views::cartesian_product(
                    std::views::iota(0u, down_node_cnt),
                    std::views::iota(0u, right_node_cnt)
                )
                | std::views::transform([](const auto & p) noexcept {
                    const auto &[n_down, n_right] { p };
                    return N { layer::DIAGONAL, n_down, n_right };
                })
            };
            auto down_layer_nodes {
                std::views::cartesian_product(
                    std::views::iota(1u, down_node_cnt),
                    std::views::iota(0u, right_node_cnt)
                )
                | std::views::transform([](const auto & p) noexcept {
                    const auto &[n_down, n_right] { p };
                    return N { layer::DOWN, n_down, n_right };
                })
            };
            auto right_layer_nodes {
                std::views::cartesian_product(
                    std::views::iota(0u, down_node_cnt),
                    std::views::iota(1u, right_node_cnt)
                )
                | std::views::transform([](const auto & p) noexcept {
                    const auto &[n_down, n_right] { p };
                    return N { layer::RIGHT, n_down, n_right };
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
                    std::views::iota(0u, down_node_cnt),
                    std::views::iota(0u, right_node_cnt)
                )
                | std::views::transform([&](const auto & p) noexcept {
                    const auto &[n_down, n_right] { p };
                    return this->get_outputs(N { layer::DIAGONAL, n_down, n_right });
                })
                | std::views::join
            };
            auto down_layer_edges {
                std::views::cartesian_product(
                    std::views::iota(1u, down_node_cnt),
                    std::views::iota(0u, right_node_cnt)
                )
                | std::views::transform([&](const auto & p) noexcept {
                    const auto &[n_down, n_right] { p };
                    return this->get_outputs(N { layer::DOWN, n_down, n_right });
                })
                | std::views::join
            };
            auto right_layer_edges {
                std::views::cartesian_product(
                    std::views::iota(0u, down_node_cnt),
                    std::views::iota(1u, right_node_cnt)
                )
                | std::views::transform([&](const auto & p) noexcept {
                    const auto &[n_down, n_right] { p };
                    return this->get_outputs(N { layer::RIGHT, n_down, n_right });
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
            const auto& [n_layer, n_down, n_right] { node };
            return (n_layer == layer::DIAGONAL && n_down < down_node_cnt && n_down >= 0u && n_right < right_node_cnt && n_right >= 0u)
                || (n_layer == layer::DOWN && n_down < down_node_cnt && n_down >= 1u && n_right < right_node_cnt && n_right >= 0u)
                || (n_layer == layer::RIGHT && n_down < down_node_cnt && n_down >= 0u && n_right < right_node_cnt && n_right >= 1u);
        }

        bool has_edge(const E& edge) {
            const auto& [n1_layer, n1_down, n1_right] { edge.first };
            const auto& [n2_layer, n2_down, n2_right] { edge.second };
            return (n1_layer == layer::DIAGONAL && n2_layer == layer::DIAGONAL && n1_down + 1u == n2_down && n1_right + 1u == n2_right && n2_down < down_node_cnt && n2_right < right_node_cnt)  // match
                || (n1_layer == layer::DIAGONAL && n2_layer == layer::DOWN && n1_down + 1u == n2_down && n1_right == n2_right && n2_down < down_node_cnt && n2_right < right_node_cnt)  // initial gap (down)
                || (n1_layer == layer::DIAGONAL && n2_layer == layer::RIGHT && n1_down == n2_down && n1_right + 1u == n2_right && n2_down < down_node_cnt && n2_right < right_node_cnt) // initial gap (right)
                || (n1_layer == layer::DOWN && n2_layer == layer::DOWN && n1_down > 0u && n1_down + 1u == n2_down && n1_right == n2_right && n2_down < down_node_cnt && n2_right < right_node_cnt)  // extended gap (down)
                || (n1_layer == layer::RIGHT && n2_layer == layer::RIGHT && n1_right > 0u && n1_down == n2_down && n1_right + 1u == n2_right && n2_down < down_node_cnt && n2_right < right_node_cnt) // extended gap (right)
                || (n1_layer == layer::DOWN && n2_layer == layer::DIAGONAL && n1_down > 0u && n1_down == n2_down && n1_right == n2_right && n2_down < down_node_cnt && n2_right < right_node_cnt)  // freeride (down)
                || (n1_layer == layer::RIGHT && n2_layer == layer::DIAGONAL && n1_right > 0u && n1_down == n2_down && n1_right == n2_right && n2_down < down_node_cnt && n2_right < right_node_cnt); // freeride (right)
        }

        auto get_outputs_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            boost::container::static_vector<std::tuple<E, N, N, ED*>, 3> ret {};
            const auto& [n1_layer, n1_down, n1_right] { node };
            if (n1_layer == layer::DIAGONAL) {
                if (n1_down == down_node_cnt - 1u && n1_right == right_node_cnt - 1u) {
                    // do nothing
                } else if (n1_down < down_node_cnt - 1u && n1_right < right_node_cnt - 1u) {
                    ret.push_back(construct_full_edge(node, { layer::DIAGONAL, n1_down + 1u, n1_right + 1u }));
                    ret.push_back(construct_full_edge(node, { layer::DOWN, n1_down + 1u, n1_right }));
                    ret.push_back(construct_full_edge(node, { layer::RIGHT, n1_down, n1_right + 1u }));
                } else if (n1_right == right_node_cnt - 1u) {
                    ret.push_back(construct_full_edge(node, { layer::DOWN, n1_down + 1u, n1_right }));
                } else if (n1_down == down_node_cnt - 1u) {
                    ret.push_back(construct_full_edge(node, { layer::RIGHT, n1_down, n1_right + 1u }));
                }
            } else if (n1_layer == layer::DOWN) {
                ret.push_back(construct_full_edge(node, { layer::DIAGONAL, n1_down, n1_right }));
                if (n1_down < down_node_cnt - 1u) {
                    ret.push_back(construct_full_edge(node, { layer::DOWN, n1_down + 1u, n1_right }));
                }
            } else if (n1_layer == layer::RIGHT) {
                ret.push_back(construct_full_edge(node, { layer::DIAGONAL, n1_down, n1_right }));
                if (n1_right < right_node_cnt - 1u) {
                    ret.push_back(construct_full_edge(node, { layer::RIGHT, n1_down, n1_right + 1u }));
                }
            }
            return std::move(ret)
                | std::views::transform([](const auto& edge_full) noexcept {
                    const auto& [e, n1, n2, ed_ptr] = edge_full;
                    return std::tuple<E, N, N, ED&>(e, n1, n2, *ed_ptr);
                });
        }

        std::tuple<E, N, N, ED&> get_output_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto it = this->get_outputs_full(node).begin();
            const auto& ret = *it;
            ++it;
            if (it != this->get_outputs_full(node).end()) {
                throw std::runtime_error {"More than one edge exists"};
            }
            return ret;
        }

        auto get_inputs_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            boost::container::static_vector<std::tuple<E, N, N, ED*>, 3> ret {};
            const auto& [n2_layer, n2_down, n2_right] { node };
            if (n2_layer == layer::DIAGONAL) {
                if (n2_down == 0u && n2_right == 0u) {
                    // do nothing
                } else if (n2_down > 0u && n2_right > 0u) {
                    ret.push_back(construct_full_edge({ layer::DIAGONAL, n2_down - 1u, n2_right - 1u }, node));
                    ret.push_back(construct_full_edge({ layer::DOWN, n2_down, n2_right }, node));
                    ret.push_back(construct_full_edge({ layer::RIGHT, n2_down, n2_right }, node));
                } else if (n2_right > 0u) {
                    ret.push_back(construct_full_edge({ layer::RIGHT, n2_down, n2_right }, node));
                } else if (n2_down > 0u) {
                    ret.push_back(construct_full_edge({ layer::DOWN, n2_down, n2_right }, node));
                }
            } else if (n2_layer == layer::DOWN) {
                if (n2_down > 0u) {
                    ret.push_back(construct_full_edge({ layer::DIAGONAL, n2_down - 1u, n2_right }, node));
                }
                if (n2_down > 1u) {
                    ret.push_back(construct_full_edge({ layer::DOWN, n2_down - 1u, n2_right }, node));
                }
            } else if (n2_layer == layer::RIGHT) {
                if (n2_right > 0u) {
                    ret.push_back(construct_full_edge({ layer::DIAGONAL, n2_down, n2_right - 1u }, node));
                }
                if (n2_right > 1u) {
                    ret.push_back(construct_full_edge({ layer::RIGHT, n2_down, n2_right - 1u }, node));
                }
            }
            return std::move(ret)
                | std::views::transform([](const auto& edge_full) noexcept {
                    const auto& [e, n1, n2, ed_ptr] = edge_full;
                    return std::tuple<E, N, N, ED&>(e, n1, n2, *ed_ptr);
                });
        }

        std::tuple<const E&, const N&, const N&, ED&> get_input_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto it = this->get_inputs_full(node).begin();
            const auto& ret = *it;
            ++it;
            if (it != this->get_inputs_full(node).end()) {
                throw std::runtime_error {"More than one edge exists"};
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

        E get_output(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto it = this->get_outputs(node).begin();
            const auto& ret = *it;
            ++it;
            if (it != this->get_outputs_full(node).end()) {
                throw std::runtime_error {"More than one edge exists"};
            }
            return ret;
        }

        auto get_inputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_inputs_full(node) | std::views::transform([this](const auto& v) noexcept -> E { return std::get<0>(v); });
        }

        E get_input(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto it = this->get_inputs(node).begin();
            const auto& ret = *it;
            ++it;
            if (it != this->get_outputs_full(node).end()) {
                throw std::runtime_error {"More than one edge exists"};
            }
            return ret;
        }

        bool has_outputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_outputs(node).size() > 0u;
        }

        bool has_inputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_inputs(node).size() > 0u;
        }

        size_t get_out_degree(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_outputs(node).size();
        }

        size_t get_in_degree(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_inputs(node).size();
        }

        template<weight WEIGHT=std::float64_t>
        void assign_weights(
            const auto& v,  // random access container
            const auto& w,  // random access container
            std::function<
                WEIGHT(
                    const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(v[0u])>>>&,
                    const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(w[0u])>>>&
                )
            > weight_lookup,
            std::function<void(ED&, WEIGHT weight)> weight_setter,
            const WEIGHT gap_weight = {},
            const WEIGHT freeride_weight = {}
        ) {
            using V_ELEM = std::decay_t<decltype(*v.begin())>;
            using W_ELEM = std::decay_t<decltype(*w.begin())>;
            if constexpr (error_check) {
                if (down_node_cnt != v.size() + 1u || right_node_cnt != w.size() + 1u) {
                    throw std::runtime_error("Mismatching node count");
                }
            }
            for (const auto& edge : get_edges()) {
                const auto& [n1, n2] { edge };
                const auto& [n1_layer, n1_down, n1_right] { n1 };
                const auto& [n2_layer, n2_down, n2_right] { n2 };
                if ((n1_layer == layer::DOWN && n2_layer == layer::DOWN)
                        || n1_layer == layer::RIGHT && n2_layer == layer::RIGHT) {  // gap
                    ED& ed { get_edge_data(edge) };
                    weight_setter(ed, gap_weight);
                } else if ((n1_layer == layer::DOWN && n2_layer == layer::DIAGONAL)
                        || (n1_layer == layer::RIGHT && n2_layer == layer::DIAGONAL)) {  // freeride
                    ED& ed { get_edge_data(edge) };
                    weight_setter(ed, freeride_weight);
                } else {
                    std::optional<std::reference_wrapper<const V_ELEM>> v_elem { std::nullopt };
                    if (n1_down + 1u == n2_down) {
                        v_elem = { v[n1_down] };
                    }
                    std::optional<std::reference_wrapper<const W_ELEM>> w_elem { std::nullopt };
                    if (n1_right + 1u == n2_right) {
                        w_elem = { w[n1_right] };
                    }
                    ED& ed { get_edge_data(edge) };
                    weight_setter(ed, weight_lookup(v_elem, w_elem));
                }
            }
        }

        static auto edge_to_elements(
            const E& edge,
            const auto& v,  // random access container
            const auto& w   // random access container
        ) {
            using ELEM = std::decay_t<decltype(v[0])>;
            using OPT_ELEM_REF = std::optional<std::reference_wrapper<const ELEM>>;
            using RET = std::optional<std::pair<OPT_ELEM_REF, OPT_ELEM_REF>>;

            const auto& [n1, n2] { edge };
            const auto& [n1_layer, n1_down, n1_right] { n1 };
            const auto& [n2_layer, n2_down, n2_right] { n2 };
            if (n1_layer == layer::DIAGONAL && n2_layer == layer::DIAGONAL) {  // match
                if (n1_down + 1u == n2_down && n1_right + 1u == n2_right) {
                    if constexpr (error_check) {
                        if (n1_down >= v.size() or n1_right >= w.size()) {
                            throw std::runtime_error("Out of bounds");
                        }
                    }
                    return RET { { { v[n1_down] }, { w[n1_right] } } };
                }
            } else if ((n1_layer == layer::DOWN && n2_layer == layer::DOWN)  // extended indel
                || (n1_layer == layer::DIAGONAL && n2_layer == layer::DOWN)) {  // indel
                if (n1_down + 1u == n2_down && n1_right == n2_right) {
                    if constexpr (error_check) {
                        if (n1_down >= v.size()) {
                            throw std::runtime_error("Out of bounds");
                        }
                    }
                    return RET { { { v[n1_down] }, std::nullopt } };
                }
            } else if ((n1_layer == layer::RIGHT && n2_layer == layer::RIGHT)  // extended indel
                || (n1_layer == layer::DIAGONAL && n2_layer == layer::RIGHT)) {  // indel
                if (n1_down == n2_down && n1_right + 1u == n2_right) {
                    if constexpr (error_check) {
                        if (n1_right >= w.size()) {
                            throw std::runtime_error("Out of bounds");
                        }
                    }
                    return RET { { std::nullopt, { w[n1_right] } } };
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

        constexpr static INDEX node_count(
            INDEX _down_node_cnt,
            INDEX _right_node_cnt
        ) {
            INDEX node_cnt {};
            node_cnt += _down_node_cnt * _right_node_cnt; // Middle layer
            node_cnt += (_down_node_cnt - 1u) * _right_node_cnt; // Down gap layer
            node_cnt += _down_node_cnt * (_right_node_cnt - 1u); // Right gap layer
            return node_cnt;
        }

        constexpr static INDEX edge_count(
            INDEX _down_node_cnt,
            INDEX _right_node_cnt
        ) {
            INDEX edge_cnt {};
            // Middle layer
            {
                // Start off by assuming each node has 3 outgoing edges.
                INDEX middle_edge_cnt { (_down_node_cnt * _right_node_cnt) * 3u };
                // The leaf node doesn't have any outgoing edges, so adjust for that.
                middle_edge_cnt -= 3u;
                // The right-most column (not counting the leaf node) only has down-ward edges, so adjust for that.
                if (_down_node_cnt > 1u) {
                    middle_edge_cnt -= (_down_node_cnt - 1u) * 2u;
                }
                // The down-most column (not counting the leaf node) only has down-ward edges, so adjust for that.
                if (_right_node_cnt > 1u) {
                    middle_edge_cnt -= (_right_node_cnt - 1u) * 2u;
                }
                edge_cnt += middle_edge_cnt;
            }
            // Down gap layer
            {
                // Start off by assuming each node has 2 outgoing edges (freeride + gap).
                // Technically the nodes start at down index of 1 (down index 0 has no nodes), so adjust for that.
                INDEX down_gap_edge_cnt { ((_down_node_cnt - 1u) * _right_node_cnt) * 2u };
                edge_cnt += down_gap_edge_cnt;
            }
            // Right gap layer
            {
                // Start off by assuming each node has 2 outgoing edges (freeride + gap).
                // Technically the nodes start at right index of 1 (right index 0 has no nodes), so adjust for that.
                INDEX right_gap_edge_cnt { (_down_node_cnt * (_right_node_cnt - 1u)) * 2u };
                edge_cnt += right_gap_edge_cnt;
            }
            return edge_cnt;
        }

        constexpr static INDEX longest_path_edge_count(
            INDEX _down_node_cnt,
            INDEX _right_node_cnt
        ) {
            return (_right_node_cnt - 1u) * 2u + (_down_node_cnt - 1u) * 2u;
        }
    };
}
#endif //OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_EXTENDED_GAP_ALIGNMENT_GRAPH_H
