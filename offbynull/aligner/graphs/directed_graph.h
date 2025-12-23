#ifndef OFFBYNULL_ALIGNER_GRAPHS_DIRECTED_GRAPH_H
#define OFFBYNULL_ALIGNER_GRAPHS_DIRECTED_GRAPH_H

#include <cstddef>
#include <ranges>
#include <map>
#include <set>
#include <tuple>
#include <stdexcept>
#include <utility>
#include <concepts>
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::graphs::directed_graph {
    using offbynull::aligner::graph::graph::node;
    using offbynull::aligner::graph::graph::edge;
    using offbynull::concepts::unqualified_object_type;
    using offbynull::concepts::bidirectional_range_of_non_cvref;
    using offbynull::aligner::graph::graph::full_input_output_range;

    /**
     * @ref offbynull::aligner::graph::graph::graph implementation that is *not immutable* (can be modified).
     * 
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam N_ Node identifier type, used to lookup nodes.
     * @tparam ND_ Node data type, used to associated data with nodes.
     * @tparam E_ Edge identifier type, used to lookup edges.
     * @tparam ED_ Edge data type, used to associated data with edges.
     */
    template<
        bool debug_mode,
        node N_,
        unqualified_object_type ND_,
        edge E_,
        unqualified_object_type ED_
    >
    requires requires(N_ n, E_ e) {
        { n < n } -> std::convertible_to<bool>;  // Required because N_ used as std::map key.
        { e < e } -> std::convertible_to<bool>;  // Required because E_ used as std::map key.
    }
    class directed_graph {
    public:
        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::N */
        using N = N_;
        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::ND */
        using ND = ND_;
        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::E */
        using E = E_;
        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::ED */
        using ED = ED_;

    private:
        std::map<N, std::set<E>> node_outbound {};
        std::map<N, std::set<E>> node_inbound {};
        std::map<N, ND> node_data {};
        std::map<E, std::tuple<N, N, ED>> edges {};

        template<typename K, typename V>
        void del_map_key(std::map<K, V> &map, const K& key) {
            auto iter { map.find(key) };
            if (iter != map.end()) {
                map.erase(iter);
            }
        }

        template<typename V>
        void del_set(std::set<V> &set, const V& val) {
            auto iter { set.find(val) };
            if (iter != set.end()) {
                set.erase(iter);
            }
        }

    public:
        /**
         * Insert node.
         *
         * If `n` already exists within this graph, the behavior of this function is undefined.
         *
         * @param n Node ID.
         * @param data Node data.
         */
        void insert_node(const N& n, ND&& data) {
            if constexpr (debug_mode) {
                if (has_node(n)) {
                    throw std::runtime_error { "Node already exists" };
                }
            }
            this->node_outbound.insert({ n, {} });
            this->node_inbound.insert({ n, {} });
            this->node_data.insert({ n, std::forward<ND>(data) });
        }

        /**
         * Delete node.
         *
         * If `n` doesn't exist within this graph, the behavior of this function is undefined.
         *
         * @param n Node ID.
         */
        void delete_node(const N& n) {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            std::set<E> outbound_copy(this->node_outbound.at(n)); // done to prevent concurrent modification problems
            for (const auto& e : outbound_copy) {
                auto& [from_n, to_n, _] { this->edges.at(e) };
                del_set(this->node_outbound.at(from_n), e);
                del_set(this->node_inbound.at(to_n), e);
                del_map_key(this->edges, e);
            }
            std::set<E> inbound_copy(this->node_inbound.at(n)); // done to prevent concurrent modification problems
            for (const auto& e : inbound_copy) {
                auto& [from_n, to_n, _] { this->edges.at(e) };
                del_set(this->node_outbound.at(from_n), e);
                del_set(this->node_inbound.at(to_n), e);
                del_map_key(this->edges, e);
            }
            del_map_key(this->node_inbound, n);
            del_map_key(this->node_outbound, n);
            del_map_key(this->node_data, n);
        }

        /**
         * Update data associated with node.
         *
         * If `n` doesn't exist within this graph, the behavior of this function is undefined.
         *
         * @param n Node ID.
         * @param data Updated node data.
         */
        void update_node_data(const N& n, ND&& data) {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            this->node_data.at(n) = std::forward<ND>(data);
        }

        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::get_node_data */
        const ND& get_node_data(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return this->node_data.at(n);
        }

        /**
         * Insert a node between an existing edge, breaking that edge into 2.
         *
         * The existing edge `existing_e` is replaced with two new edges: `from_e` and `to_e`:
         *
         *  * `from_e` spans from `existing_e`'s source node to `insert_n`.
         *  * `to_e` spans from `insert_n` to `existing_e`'s destination node.
         *
         * The behavior of this function is undefined if any of the following conditions are met:
         *
         *  * `insert_n` already exists with this graph.
         *  * `existing_e` doesn't exist within this graph.
         *  * `from_e` already exists with this graph.
         *  * `to_e` already exists with this graph.
         *
         * @param insert_n Insert node ID.
         * @param insert_n_data Insert node data.
         * @param existing_e Existing edge ID.
         * @param from_e Former portion's replacement edge ID (from source node to `insert_n`).
         * @param from_e_data Former portion's replacement edge data.
         * @param to_e Latter portion's replacement edge ID (from `insert_n` to destination node).
         * @param to_e_data Latter portion's replacement edge data.
         */
        void insert_node_between_edge(
            const N& insert_n,
            ND&& insert_n_data,
            const E& existing_e,
            const E& from_e,
            ED&& from_e_data,
            const E& to_e,
            ED&& to_e_data
        ) {
            if constexpr (debug_mode) {
                if (has_node(insert_n)) {
                    throw std::runtime_error { "Node already exists" };
                }
                if (!has_edge(existing_e)) {
                    throw std::runtime_error { "Existing edge doesn't exist" };
                }
                if (has_edge(from_e)) {
                    throw std::runtime_error { "From edge already exists" };
                }
                if (has_edge(to_e)) {
                    throw std::runtime_error { "To edge already exists" };
                }
            }
            const auto& [node_from, node_to, _] { this->get_edge(existing_e) };
            this->insert_node(insert_n, std::forward<ND>(insert_n_data));
            this->insert_edge(from_e, node_from, insert_n, std::forward<ED>(from_e_data));
            this->insert_edge(to_e, insert_n, node_to, std::forward<ED>(to_e_data));
            this->delete_edge(existing_e); // must happen at end, otherwise bad memory access
        }

        /**
         * Insert edge.
         *
         * The behavior of this function is undefined if any of the following conditions are met:
         *
         *  * `e` already exists with this graph.
         *  * `from_n` doesn't exist within this graph.
         *  * `to_n` doesn't exist within this graph.
         *
         * @param e Edge ID.
         * @param from_n Edge's source node.
         * @param to_n Edge's destination node.
         * @param data Edge data.
         */
        void insert_edge(const E& e, const N& from_n, const N& to_n, ED&& data) {
            if constexpr (debug_mode) {
                if (!has_node(from_n)) {
                    throw std::runtime_error { "From node doesn't exist" };
                }
                if (!has_node(to_n)) {
                    throw std::runtime_error { "To node doesn't exist" };
                }
                if (has_edge(e)) {
                    throw std::runtime_error { "Edge already exists" };
                }
            }
            this->edges.insert({ e, std::tuple<N, N, ED> { from_n, to_n, std::forward<ED>(data) } });
            this->node_inbound.at(to_n).insert(e);
            this->node_outbound.at(from_n).insert(e);
        }

        /**
         * Delete edge.
         *
         * If `e` doesn't exist within this graph, the behavior of this function is undefined.
         *
         * @param e Edge ID
         * @param remove_from_if_isolated If `true`, `e`'s source node will also be removed provided that no other edges point to it.
         * @param remove_to_if_isolated If `true`, `e`'s destination node will also be removed provided that no other edges point to it.
         */
        void delete_edge(const E& e, bool remove_from_if_isolated = false, bool remove_to_if_isolated = false) {
            if constexpr (debug_mode) {
                if (!has_edge(e)) {
                    throw std::runtime_error { "Edge doesn't exist" };
                }
            }
            auto& [from_node, to_node, _] { this->edges.at(e) };
            // if constexpr (debug_mode) {
            //     if (!has_node(from_node)) {
            //         throw std::runtime_error { "This should never happen" };
            //     }
            //     if (!has_node(to_node)) {
            //         throw std::runtime_error { "This should never happen" };
            //     }
            // }
            del_set(this->node_outbound.at(from_node), e);
            del_set(this->node_inbound.at(to_node), e);
            // from and to may be the same -- if they are, and you"ve removed the from, make sure you don"t try to remove to
            // because form and to are the same... you can"t remove the same node twice
            bool dealing_with_same_node { from_node == to_node };
            bool removed_from { false };
            //bool removed_to { false };
            if (remove_from_if_isolated
                    && this->node_inbound.at(from_node).size() == 0zu
                    && this->node_outbound.at(from_node).size() == 0zu) {
                this->delete_node(from_node);
                removed_from = true;
            }
            if (remove_to_if_isolated
                    && (not dealing_with_same_node or (dealing_with_same_node and not removed_from))
                    && this->node_inbound.at(to_node).size() == 0zu
                    && this->node_outbound.at(to_node).size() == 0zu) {
                this->delete_node(to_node);
                //removed_to = true;
            }
            del_map_key(this->edges, e);
        }

        /**
         * Update data associated with edge.
         *
         * If `e` doesn't exist within this graph, the behavior of this function is undefined.
         *
         * @param e Edge ID.
         * @param data Updated edge data.
         */
        void update_edge_data(const E& e, ED&& data) {
            if constexpr (debug_mode) {
                if (!has_edge(e)) {
                    throw std::runtime_error { "Edge doesn't exist" };
                }
            }
            std::get<2zu>(this->edges.at(e)) = std::forward<ED>(data);
        }

        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::get_edge_data */
        const ED& get_edge_data(const E& e) const {
            if constexpr (debug_mode) {
                if (!has_edge(e)) {
                    throw std::runtime_error { "Edge doesn't exist" };
                }
            }
            return std::get<2zu>(this->edges.at(e));
        }

        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::get_edge_from */
        const N& get_edge_from(const E& e) const {
            if constexpr (debug_mode) {
                if (!has_edge(e)) {
                    throw std::runtime_error { "Edge doesn't exist" };
                }
            }
            return std::get<0zu>(this->edges.at(e));
        }

        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::get_edge_to */
        const N& get_edge_to(const E& e) const {
            if constexpr (debug_mode) {
                if (!has_edge(e)) {
                    throw std::runtime_error { "Edge doesn't exist" };
                }
            }
            return std::get<1zu>(this->edges.at(e));
        }

        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::get_edge */
        std::tuple<const N&, const N&, const ED&> get_edge(const E& e) const {
            if constexpr (debug_mode) {
                if (!has_edge(e)) {
                    throw std::runtime_error { "Edge doesn't exist" };
                }
            }
            auto& ref { edges.at(e) };
            return std::tuple<const N&, const N&, const ED&> { std::get<0zu>(ref), std::get<1zu>(ref), std::get<2zu>(ref) };
        }

        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::get_root_nodes */
        bidirectional_range_of_non_cvref<N> auto get_root_nodes() const {
            auto ret { this->get_nodes() | std::views::filter([&](const auto& n) { return !has_inputs(n); }) };
            return ret;
        }

        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::get_root_node */
        const N& get_root_node() const {
            auto range { this->get_root_nodes() };
            auto it { range.begin() };
            const N& ret { *it };
            ++it;
            if (it != range.end()) {
                throw std::runtime_error { "Exactly 1 root node required" };
            }
            return ret;
        }

        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::get_leaf_nodes */
        bidirectional_range_of_non_cvref<N> auto get_leaf_nodes() const {
            auto ret { this->get_nodes() | std::views::filter([&](const auto& n) { return !has_outputs(n); }) };
            return ret;
        }

        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::get_leaf_node */
        const N& get_leaf_node() const {
            auto range { this->get_leaf_nodes() };
            auto it { range.begin() };
            const N& ret { *it };
            ++it;
            if (it != range.end()) {
                throw std::runtime_error { "Exactly 1 leaf node required" };
            }
            return ret;
        }

        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::get_nodes */
        bidirectional_range_of_non_cvref<N> auto get_nodes() const {
            return this->node_outbound | std::views::transform([](const auto& p) -> const N& { return p.first; });
        }

        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::get_edges */
        bidirectional_range_of_non_cvref<E> auto get_edges() const {
            return this->edges | std::views::transform([](auto& p) -> const E& { return p.first; });
        }

        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::has_node */
        bool has_node(const N& n) const {
            return this->node_outbound.contains(n);
        }

        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::has_edge */
        bool has_edge(const E& e) const {
            return this->edges.contains(e);
        }

        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::get_outputs_full */
        full_input_output_range<N, E, ED> auto get_outputs_full(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return this->node_outbound.at(n) | std::views::transform([this](auto& e) {
                const auto& [from_node, to_node, edge_data] { this->get_edge(e) };
                return std::tuple<const E&, const N&, const N&, const ED&> { e, from_node, to_node, edge_data };
            });
        }

        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::get_inputs_full */
        full_input_output_range<N, E, ED> auto get_inputs_full(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return this->node_inbound.at(n) | std::views::transform([this](auto& e) {
                const auto& [from_node, to_node, edge_data] { this->get_edge(e) };
                return std::tuple<const E&, const N&, const N&, const ED&> { e, from_node, to_node, edge_data };
            });
        }

        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::get_outputs */
        bidirectional_range_of_non_cvref<E> auto get_outputs(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return this->node_outbound.at(n)
                | std::views::transform([this](auto& e) -> const E& { return e; });
        }

        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::get_inputs */
        bidirectional_range_of_non_cvref<E> auto get_inputs(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return this->node_inbound.at(n)
                | std::views::transform([this](auto& e) -> const E& { return e; });
        }

        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::has_outputs */
        bool has_outputs(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return this->get_outputs(n).size() > 0zu;
        }

        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::has_inputs */
        bool has_inputs(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return this->get_inputs(n).size() > 0zu;
        }

        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::get_out_degree */
        std::size_t get_out_degree(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return this->node_outbound.at(n).size();
        }

        /** @copydoc offbynull::aligner::graph::graph::unimplemented_graph::get_in_degree */
        std::size_t get_in_degree(const N& n) const {
            if constexpr (debug_mode) {
                if (!has_node(n)) {
                    throw std::runtime_error { "Node doesn't exist" };
                }
            }
            return this->node_inbound.at(n).size();
        }
    };
};
#endif //OFFBYNULL_ALIGNER_GRAPHS_DIRECTED_GRAPH_H
