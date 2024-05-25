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
#include "offbynull/aligner/graph/grid_container_creator.h"
#include "offbynull/aligner/graph/grid_container_creators.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::graphs::pairwise_local_alignment_graph {
    using offbynull::aligner::graphs::grid_graph::grid_graph;
    using offbynull::aligner::graph::grid_container_creator::grid_container_creator;
    using offbynull::aligner::graph::grid_container_creators::vector_grid_container_creator;
    using offbynull::aligner::concepts::weight;
    using offbynull::utils::concat_view;

    enum class edge_type : uint8_t {
        FREE_RIDE,
        NORMAL
    };

    template<std::unsigned_integral T>
    class edge {
    public:
        using N = std::pair<T, T>;
        edge_type type;
        std::pair<N, N> inner_edge;

        std::strong_ordering operator<=>(const edge& rhs) const = default;
    };

    template<
        typename ND_,
        typename ED_,
        std::unsigned_integral T = unsigned int,
        grid_container_creator<T> ND_ALLOCATOR_ = vector_grid_container_creator<ND_, T, false>,
        grid_container_creator<T> ED_ALLOCATOR_ = vector_grid_container_creator<ED_, T, false>,
        bool error_check = true
    >
        requires std::is_integral_v<T> && std::is_unsigned_v<T>
    class pairwise_local_alignment_graph {
    public:
        using N = std::pair<T, T>;
        using E = edge<T>;
        using ED = ED_;
        using ND = ND_;

    private:
        grid_graph<ND, ED, T, ND_ALLOCATOR_, ED_ALLOCATOR_, error_check> g;
        ED freeride_ed;

    public:
        const T down_node_cnt;
        const T right_node_cnt;

        pairwise_local_alignment_graph(
            T _down_node_cnt,
            T _right_node_cnt,
            ED indel_data = {},
            ED freeride_data = {},
            ND_ALLOCATOR_ nd_container_creator = {},
            ED_ALLOCATOR_ ed_container_creator = {}
        )
        : g{_down_node_cnt, _right_node_cnt, indel_data, nd_container_creator, ed_container_creator}
        , freeride_ed{freeride_data}
        , down_node_cnt{_down_node_cnt}
        , right_node_cnt{_right_node_cnt} {}

        void update_node_data(const N& node, ND&& data) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            g.update_node_data(node, std::forward<ND>(data));
        }

        ND& get_node_data(const N& node) {
            return g.get_node_data(node);
        }

        void update_edge_data(const E& edge, ED&& data) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            if (edge.type == edge_type::FREE_RIDE) {
                freeride_ed = std::forward<ED>(data);
            } else {
                g.update_edge_data(edge.inner_edge, data);
            }
        }

        ED& get_edge_data(const E& edge) {
            return std::get<2>(get_edge(edge));
        }

        N get_edge_from(const E& edge) {
            return std::get<0>(get_edge(edge));
        }

        N get_edge_to(const E& edge) {
            return std::get<1>(get_edge(edge));
        }

        std::tuple<N, N, ED&> get_edge(const E& edge) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            if (edge.type == edge_type::FREE_RIDE) {
                auto [n1, n2] = edge.inner_edge;
                return std::tuple<N, N, ED&> {n1, n2, freeride_ed};
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

        auto get_leaf_node() {
            return g.get_leaf_node();
        }

        auto get_nodes() {
            return g.get_nodes();
        }

        auto get_edges() {
            auto from_src_range {
                std::views::cartesian_product(
                    std::views::iota(0u, g.down_node_cnt),
                    std::views::iota(0u, g.right_node_cnt)
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
                    std::views::iota(0u, g.down_node_cnt),
                    std::views::iota(0u, g.right_node_cnt)
                )
                | ( std::views::reverse | std::views::drop(1) | std::views::reverse ) // drop bottom right
                | std::views::transform([&](const auto & p) noexcept {
                    N n1 { std::get<0>(p), std::get<1>(p) };
                    N n2 { g.down_node_cnt - 1u, g.right_node_cnt - 1u };
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
                const auto & [n1_down, n1_right] { n1 };
                const auto & [n2_down, n2_right] { n2 };
                if (n1_down == 0u && n1_right == 0u) {
                    if (n2_down == 0u && n2_right == 0u) {
                        return false;
                    } else if (n2_down <= g.down_node_cnt - 1u && n2_right <= g.right_node_cnt - 1u) {
                        return true;
                    } else {
                        return false;
                    }
                } else if (n2_down == g.down_node_cnt - 1u && n2_right == g.right_node_cnt - 1u) {
                    if (n1_down == g.down_node_cnt - 1u && n1_right == g.right_node_cnt - 1u) {
                        return false;
                    } else if (n1_down <= g.down_node_cnt - 1u && n1_right <= g.right_node_cnt - 1u) {
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
            // auto normals {
            //     g.get_outputs(node)
            //     | std::views::transform([](const auto& e) noexcept { return E { edge_type::NORMAL, e }; })
            // };
            // boost::container::static_vector<E, 1> freerides;
            // if (node == N{ g.down_node_cnt - 1u, g.right_node_cnt - 1u }) {
            //     // do nothing
            // } else {
            //     freerides.push_back({ edge_type::FREE_RIDE, { node, { g.down_node_cnt - 1u, g.right_node_cnt - 1u } } });
            // }
            // auto freerides_no_ref {
            //     std::move(freerides)
            //     | std::views::transform([](const auto& e) noexcept -> E { return e; })
            // };
            // return concat_view(
            //     std::move(normals),
            //     std::move(freerides_no_ref)
            // );
            // // COMMENTED OUT BECAUSE concat_view DOESN'T SUPPORT PIPE OPERATOR, WHICH CALLERS USE.
            boost::container::static_vector<std::tuple<E, N, N, ED&>, 4> ret {};
            for (const auto& [e, n1, n2, ed_ptr] : g.get_outputs_full(node)) { // will iterate at-most 3 times
                E new_e { edge_type::NORMAL, e };
                ret.push_back(std::tuple<E, N, N, ED&> {new_e, n1, n2, ed_ptr});
            }
            // I had to use the for-loop above because g.get_outputs_full(node) doesn't allow pipe operator?
            const auto & [n_down, n_right] { node };
            if (!(n_down == g.down_node_cnt - 1u && n_right == g.right_node_cnt - 1u)) {
                ret.push_back(
                    {
                        { edge_type::FREE_RIDE, { node, { g.down_node_cnt - 1u, g.right_node_cnt - 1u } } },
                        node,
                        {g.down_node_cnt - 1u, g.right_node_cnt - 1u},
                        freeride_ed
                    }
                );
            }
            return ret;
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
            boost::container::static_vector<std::tuple<E, N, N, ED&>, 4> ret {};
            for (const auto& [e, n1, n2, ed_ptr] : g.get_inputs_full(node)) { // will iterate at-most 3 times
                E new_e { edge_type::NORMAL, e };
                ret.push_back(std::tuple<E, N, N, ED&> {new_e, n1, n2, ed_ptr});
            }
            // I had to use the for-loop above because g.get_outputs_full(node) doesn't allow pipe operator?
            const auto & [n_down, n_right] { node };
            if (!(n_down == 0u && n_right == 0u)) {
                ret.push_back(
                    {
                        { edge_type::FREE_RIDE, { { 0u, 0u }, node } },
                        { 0u, 0u },
                        node,
                        freeride_ed
                    }
                );
            }
            return ret;
        }

        std::tuple<E, N, N, ED&> get_input_full(const N& node) {
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
            return get_outputs_full(node) | std::views::transform([this](auto v) noexcept -> E { return std::get<0>(v); });
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
            return this->get_inputs_full(node) | std::views::transform([this](auto v) noexcept -> E { return std::get<0>(v); });
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
            return !this->get_outputs(node).empty();
        }

        bool has_inputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return !this->get_inputs(node).empty();
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
                WEIGHT weight;
                if (edge.type == edge_type::FREE_RIDE) {
                    weight = freeride_weight;
                } else {
                    const auto& [n1, n2] { edge.inner_edge };
                    const auto& [n1_down, n1_right] { n1 };
                    const auto& [n2_down, n2_right] { n2 };
                    std::optional<std::reference_wrapper<const V_ELEM>> v_elem { std::nullopt };
                    if (n1_down + 1u == n2_down) {
                        v_elem = { v[n1_down] };
                    }
                    std::optional<std::reference_wrapper<const W_ELEM>> w_elem { std::nullopt };
                    if (n1_right + 1u == n2_right) {
                        w_elem = { w[n1_right] };
                    }
                    weight = weight_lookup(v_elem, w_elem);
                }
                ED& ed { get_edge_data(edge) };
                weight_setter(ed, weight);
            }
        }

        static auto edge_to_elements(
            const E& edge,
            const auto& v,  // random access container
            const auto& w   // random access container
        )
        requires requires(decltype(v) v_, decltype(w) w_) { { v[0] } -> std::same_as<decltype(w_)>; }
        {
            using ELEM = std::decay_t<decltype(v[0])>;
            using OPT_ELEM_REF = std::optional<std::reference_wrapper<const ELEM>>;
            using RET = std::optional<std::pair<OPT_ELEM_REF, OPT_ELEM_REF>>;

            if (edge.type == edge_type::FREE_RIDE) {
                return std::nullopt;
            }
            const auto& [n1, n2] {edge};
            const auto& [n1_down, n1_right] {n1.inner_edge};
            const auto& [n2_down, n2_right] {n2.inner_edge};
            if (n1_down + 1u == n2_down && n1_right + 1u == n2_right) {
                if constexpr (error_check) {
                    if (n1_down >= v.size() or n1_right >= w.size()) {
                        throw std::runtime_error("Out of bounds");
                    }
                }
                return RET { { { v[n1_down] }, { w[n1_right] } } };
            } else if (n1_down + 1u == n2_down && n1_right == n2_right) {
                if constexpr (error_check) {
                    if (n1_down >= v.size()) {
                        throw std::runtime_error("Out of bounds");
                    }
                }
                return RET { { { v[n1_down] }, std::nullopt } };
            } else if (n1_down == n2_down && n1_right + 1u == n2_right) {
                if constexpr (error_check) {
                    if (n1_right >= w.size()) {
                        throw std::runtime_error("Out of bounds");
                    }
                }
                return RET { { std::nullopt, { w[n1_right] } } };
            }
            if constexpr (error_check) {
                throw std::runtime_error("Bad edge");
            }
        }

        constexpr static T node_count(
            T _down_node_cnt,
            T _right_node_cnt
        ) {
            return grid_graph<ND, ED, T, ND_ALLOCATOR_, ED_ALLOCATOR_, error_check>::node_count(
                _down_node_cnt, _right_node_cnt
            );
        }

        constexpr static T edge_count(
            T _down_node_cnt,
            T _right_node_cnt
        ) {
            return grid_graph<ND, ED, T, ND_ALLOCATOR_, ED_ALLOCATOR_, error_check>::edge_count(
                _down_node_cnt, _right_node_cnt
            );
        }

        constexpr static T longest_path_edge_count(
            T _down_node_cnt,
            T _right_node_cnt
        ) {
            return grid_graph<ND, ED, T, ND_ALLOCATOR_, ED_ALLOCATOR_, error_check>::longest_path_edge_count(
                _down_node_cnt, _right_node_cnt
            );
        }
    };
}
#endif //OFFBYNULL_ALIGNER_GRAPHS_PAIRWISE_LOCAL_ALIGNMENT_GRAPH_H
