#ifndef OFFBYNULL_ALIGNER_GRAPHS_DIRECTED_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_DIRECTED_GRAPH_H

#include <cstddef>
#include <ranges>
#include <algorithm>
#include <map>
#include <set>
#include <tuple>
#include <stdexcept>
#include <utility>
#include <vector>

namespace offbynull::aligner::graphs::directed_graph {
    template<typename N_, typename ND_, typename E_, typename ED_, bool error_check = true>
    class directed_graph {
    public:
        using N = N_;
        using ND = ND_;
        using E = E_;
        using ED = ED_;

    private:
        std::map<N, std::set<E>> node_outbound {};
        std::map<N, std::set<E>> node_inbound {};
        std::map<N, ND> node_data {};
        std::map<E, std::tuple<N, N, ED>> edges {};

        template<typename K, typename V>
        void del_map_key(std::map<K, V> &map, const K& key) {
            auto iter = map.find(key);
            if (iter != map.end()) {
                map.erase(iter);
            }
        }

        template<typename V>
        void del_set(std::set<V> &set, const V& val) {
            auto iter = set.find(val);
            if (iter != set.end()) {
                set.erase(iter);
            }
        }

    public:
        void insert_node(const N& node, ND&& data) {
            if constexpr (error_check) {
                if (has_node(node)) {
                    throw std::runtime_error {"Node already exists"};
                }
            }
            this->node_outbound[node] = {};
            this->node_inbound[node] = {};
            this->node_data[node] = std::forward<ND>(data);
        }

        void delete_node(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            std::set<E> outbound_copy = this->node_outbound[node]; // done to prevent concurrent modification problems
            for (const auto& edge_id : outbound_copy) {
                auto& [from_node, to_node, _] = this->edges[edge_id];
                del_set(this->node_outbound[from_node], edge_id);
                del_set(this->node_inbound[to_node], edge_id);
                del_map_key(this->edges, edge_id);
            }
            std::set<E> inbound_copy = this->node_inbound[node]; // done to prevent concurrent modification problems
            for (const auto& edge_id : inbound_copy) {
                auto& [from_node, to_node, _] = this->edges[edge_id];
                del_set(this->node_outbound[from_node], edge_id);
                del_set(this->node_inbound[to_node], edge_id);
                del_map_key(this->edges, edge_id);
            }
            del_map_key(this->node_inbound, node);
            del_map_key(this->node_outbound, node);
            del_map_key(this->node_data, node);
        }

        void update_node_data(const N& node, ND&& data) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            this->node_data[node] = std::forward<ND>(data);
        }

        ND& get_node_data(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->node_data[node];
        }

        void insert_node_between_edge(const N& new_node, ND&& new_node_data, const E& existing_edge, const E& from_edge, ED&& from_edge_data, const E& to_edge, ED&& to_edge_data) {
            if constexpr (error_check) {
                if (has_node(new_node)) {
                    throw std::runtime_error {"Node already exists"};
                }
                if (!has_edge(existing_edge)) {
                    throw std::runtime_error {"Existing edge doesn't exist"};
                }
                if (has_edge(from_edge)) {
                    throw std::runtime_error {"From edge already exists"};
                }
                if (has_edge(to_edge)) {
                    throw std::runtime_error {"To edge already exists"};
                }
            }
            const auto& [node_from, node_to, _] = this->get_edge(existing_edge);
            this->insert_node(new_node, std::forward<ND>(new_node_data));
            this->insert_edge(from_edge, node_from, new_node, std::forward<ED>(from_edge_data));
            this->insert_edge(to_edge, new_node, node_to, std::forward<ED>(to_edge_data));
            this->delete_edge(existing_edge); // must happen at end, otherwise bad memory access
        }

        void insert_edge(const E& edge, const N& from_node, const N& to_node, ED&& data) {
            if constexpr (error_check) {
                if (!has_node(from_node)) {
                    throw std::runtime_error {"From node doesn't exist"};
                }
                if (!has_node(to_node)) {
                    throw std::runtime_error {"To node doesn't exist"};
                }
                if (has_edge(edge)) {
                    throw std::runtime_error {"Edge already exists"};
                }
            }
            this->edges[edge] = std::tuple<N, N, ED> {from_node, to_node, std::forward<ED>(data)};
            this->node_inbound[to_node].insert(edge);
            this->node_outbound[from_node].insert(edge);
        }

        void delete_edge(const E& edge, bool remove_from_if_isolated = false, bool remove_to_if_isolated = false) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            auto& [from_node, to_node, _] = this->edges[edge];
            // if constexpr (error_check) {
            //     if (!has_node(from_node)) {
            //         throw std::runtime_error {"This should never happen"};
            //     }
            //     if (!has_node(to_node)) {
            //         throw std::runtime_error {"This should never happen"};
            //     }
            // }
            del_set(this->node_outbound[from_node], edge);
            del_set(this->node_inbound[to_node], edge);
            // from and to may be the same -- if they are, and you"ve removed the from, make sure you don"t try to remove to
            // because form and to are the same... you can"t remove the same node twice
            bool dealing_with_same_node = (from_node == to_node);
            bool removed_from = false;
            //bool removed_to = false;
            if (remove_from_if_isolated
                    && this->node_inbound[from_node].size() == 0
                    && this->node_outbound[from_node].size() == 0) {
                this->delete_node(from_node);
                removed_from = true;
            }
            if (remove_to_if_isolated
                    && (not dealing_with_same_node or (dealing_with_same_node and not removed_from))
                    && this->node_inbound[to_node].size() == 0
                    && this->node_outbound[to_node].size() == 0) {
                this->delete_node(to_node);
                //removed_to = true;
            }
            del_map_key(this->edges, edge);
        }

        void update_edge_data(const E& edge, ED&& data) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            std::get<2>(this->edges[edge]) = std::forward<ED>(data);
        }

        ED& get_edge_data(const E& edge) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            return std::get<2>(this->edges[edge]);
        }

        const N& get_edge_from(const E& edge) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            return std::get<0>(this->edges[edge]);
        }

        const N& get_edge_to(const E& edge) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            return std::get<1>(this->edges[edge]);
        }

        std::tuple<const N&, const N&, ED&> get_edge(const E& edge) {
            if constexpr (error_check) {
                if (!has_edge(edge)) {
                    throw std::runtime_error {"Edge doesn't exist"};
                }
            }
            auto& ref = edges[edge];
            return std::tuple<const N&, const N&, ED&> {std::get<0>(ref), std::get<1>(ref), std::get<2>(ref)};
        }

        auto get_root_nodes() {
            auto ret { this->get_nodes() | std::views::filter([&](const auto& n) noexcept { return !has_inputs(n); }) };
            return ret;
        }

        const N& get_root_node() {
            auto range = this->get_root_nodes();
            auto it = range.begin();
            const N& ret = *it;
            ++it;
            if (it != range.end()) {
                throw std::runtime_error { "Exactly 1 root node required" };
            }
            return ret;
        }

        auto get_leaf_nodes() {
            auto ret { this->get_nodes() | std::views::filter([&](const auto& n) noexcept { return !has_outputs(n); }) };
            return ret;
        }

        const N& get_leaf_node() {
            auto range = this->get_leaf_nodes();
            auto it = range.begin();
            const N& ret = *it;
            ++it;
            if (it != range.end()) {
                throw std::runtime_error { "Exactly 1 leaf node required" };
            }
            return ret;
        }

        auto get_nodes() {
            return this->node_outbound | std::views::transform([](const auto& p) noexcept -> const N& { return p.first; });
        }

        auto get_edges() {
            return this->edges | std::views::transform([](auto& p) noexcept -> const E& { return p.first; });
        }

        bool has_node(const N& node) {
            return this->node_outbound.contains(node);
        }

        bool has_edge(const E& edge) {
            return this->edges.contains(edge);
        }

        auto get_outputs_full(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->node_outbound[node] | std::views::transform([this](auto& e) noexcept {
                const auto& [from_node, to_node, edge_data] = this->get_edge(e);
                return std::tuple<const E&, const N&, const N&, ED&> {e, from_node, to_node, edge_data};
            });
        }

        std::tuple<const E&, const N&, const N&, ED&> get_output_full(const N& node) {
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
            return this->node_inbound[node] | std::views::transform([this](auto& e) noexcept {
                const auto& [from_node, to_node, edge_data] = this->get_edge(e);
                return std::tuple<const E&, const N&, const N&, ED&> {e, from_node, to_node, edge_data};
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
            return this->node_outbound[node] | std::views::transform([this](auto& e) noexcept -> const E& { return e; });
        }

        const E& get_output(const N& node) {
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
            return this->node_inbound[node] | std::views::transform([this](auto& e) noexcept -> const E& { return e; });
        }

        const E& get_input(const N& node) {
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
            return this->get_outputs(node).size() > 0zu;
        }

        bool has_inputs(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->get_inputs(node).size() > 0zu;
        }

        std::size_t get_out_degree(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->node_outbound[node].size();
        }

        std::size_t get_out_degree_unique(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto data {
                this->get_outputs_full(node)
                | std::views::transform([](const auto& edge_full) { return std::get<2>(edge_full); })
            };
            std::vector<N> data_as_vec(data.begin(), data.end());
            std::sort(data_as_vec.begin(), data_as_vec.end());
            auto new_last { std::unique(data_as_vec.begin(), data_as_vec.end()) };
            return static_cast<std::size_t>(std::distance(data_as_vec.begin(), new_last));
        }

        std::size_t get_in_degree(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            return this->node_inbound[node].size();
        }

        std::size_t get_in_degree_unique(const N& node) {
            if constexpr (error_check) {
                if (!has_node(node)) {
                    throw std::runtime_error {"Node doesn't exist"};
                }
            }
            auto data {
                this->get_inputs_full(node)
                | std::views::transform([](const auto& edge_full) { return std::get<2>(edge_full); })
            };
            std::vector<N> data_as_vec(data.begin(), data.end());
            std::sort(data_as_vec.begin(), data_as_vec.end());
            auto new_last { std::unique(data_as_vec.begin(), data_as_vec.end()) };
            return static_cast<std::size_t>(std::distance(data_as_vec.begin(), new_last));
        }
    };

    /*
    template<typename N, typename ND, typename E, typename ED>
    struct std::formatter<directed_graph<N, ND, E, ED>> : std::formatter<std::string> {
        auto format(const directed_graph<N, ND, E, ED>& g, format_context& ctx) const {
            return format_to(ctx.out(), "{}", g.to_string());  // WONT WORK WITHOUT MAKING to_string() CONST?
        }
    };
    */
};
#endif //OFFBYNULL_ALIGNER_GRAPHS_DIRECTED_GRAPH_H
