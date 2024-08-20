#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_BACKTRACKER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_BACKTRACKER_H

#include <cstddef>
#include <functional>
#include <ranges>
#include <algorithm>
#include "offbynull/aligner/graphs/pairwise_extended_gap_alignment_graph.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/concepts.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/ready_queue.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/slot_container.h"
#include "offbynull/aligner/graph/graph.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::graph_backtracker::backtracker {
    using offbynull::aligner::graph::graph::readable_graph;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::backtrackers::graph_backtracker::concepts::backtrackable_node;
    using offbynull::aligner::backtrackers::graph_backtracker::concepts::backtrackable_edge;
    using offbynull::aligner::backtrackers::graph_backtracker::slot_container::slot;
    using offbynull::aligner::backtrackers::graph_backtracker::slot_container::slot_container;
    using offbynull::aligner::backtrackers::graph_backtracker::slot_container::slot_container_container_creator_pack;
    using offbynull::aligner::backtrackers::graph_backtracker::slot_container::slot_container_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::graph_backtracker::slot_container::slot_container_stack_container_creator_pack;
    using offbynull::aligner::backtrackers::graph_backtracker::ready_queue::ready_queue;
    using offbynull::aligner::backtrackers::graph_backtracker::ready_queue::ready_queue_container_creator_pack;
    using offbynull::aligner::backtrackers::graph_backtracker::ready_queue::ready_queue_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::graph_backtracker::ready_queue::ready_queue_stack_container_creator_pack;
    using offbynull::concepts::range_of_type;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::concepts::random_access_range_of_type;




    template<
        typename T,
        typename N,
        typename E,
        typename WEIGHT
    >
    concept backtracker_container_creator_pack =
        weight<WEIGHT>
        && requires(const T t) {
            { t.create_slot_container_container_creator_pack() } -> slot_container_container_creator_pack<N, E, WEIGHT>;
            { t.create_ready_queue_container_creator_pack() } -> ready_queue_container_creator_pack<>;
            { t.create_path_container() } -> random_access_range_of_type<E>;
        };

    template<
        bool debug_mode,
        typename N,
        typename E,
        weight WEIGHT
    >
    struct backtracker_heap_container_creator_pack {
        slot_container_heap_container_creator_pack<
            debug_mode,
            N,
            E,
            WEIGHT
        > create_slot_container_container_creator_pack() const {
            return {};
        }

        ready_queue_heap_container_creator_pack<
            debug_mode
        > create_ready_queue_container_creator_pack() const {
            return {};
        }

        std::vector<E> create_path_container() const {
            return {};
        }
    };

    template<
        bool debug_mode,
        typename N,
        typename E,
        weight WEIGHT,
        std::size_t slot_container_heap_escape_size = 100zu,
        std::size_t ready_queue_heap_escape_size = 100zu,
        std::size_t path_container_heap_escape_size = 10zu
    >
    struct backtracker_stack_container_creator_pack {
        slot_container_stack_container_creator_pack<
            debug_mode,
            N,
            E,
            WEIGHT,
            slot_container_heap_escape_size
        > create_slot_container_container_creator_pack() const {
            return {};
        }

        ready_queue_stack_container_creator_pack<
            debug_mode,
            ready_queue_heap_escape_size
        > create_ready_queue_container_creator_pack() const {
            return {};
        }

        boost::container::small_vector<
            E,
            path_container_heap_escape_size
        > create_path_container() const {
            return {};
        }
    };





    template<
        bool debug_mode,
        readable_graph G,
        weight WEIGHT,
        backtracker_container_creator_pack<
            typename G::N,
            typename G::E,
            WEIGHT
        > CONTAINER_CREATOR_PACK = backtracker_heap_container_creator_pack<
            debug_mode,
            typename G::N,
            typename G::E,
            WEIGHT
        >
    >
    requires backtrackable_node<typename G::N> &&
        backtrackable_edge<typename G::E>
    class backtracker {
    public:
        using N = typename G::N;
        using E = typename G::E;

        using SLOT_CONTAINER_CONTAINER_CREATOR_PACK =
            decltype(std::declval<CONTAINER_CREATOR_PACK>().create_slot_container_container_creator_pack());
        using READY_QUEUE_CONTAINER_CREATOR_PACK =
            decltype(std::declval<CONTAINER_CREATOR_PACK>().create_ready_queue_container_creator_pack());
        using PATH_CONTAINER = decltype(std::declval<CONTAINER_CREATOR_PACK>().create_path_container());

        using slot_container_t = slot_container<debug_mode, G, WEIGHT, SLOT_CONTAINER_CONTAINER_CREATOR_PACK>;
        using ready_queue_t = ready_queue<debug_mode, G, READY_QUEUE_CONTAINER_CREATOR_PACK>;

        CONTAINER_CREATOR_PACK container_creator_pack;

        backtracker(
            CONTAINER_CREATOR_PACK container_creator_pack_ = {}
        )
        : container_creator_pack { container_creator_pack_ } {}

        slot_container_t populate_weights_and_backtrack_pointers(
            const G& g,
            std::function<WEIGHT(const E&)> edge_weight_accessor
        ) {
            // Create "slots" list
            // -------------------
            // The "slots" list tracks number of unprocessed parents for each node within the graph. The list is sorted by the
            // ordering of the node being track (node type must be orderable using std::less or < or whatever stdlib deems) such
            // that the index for a node object can be quickly found.
            auto slots_lazy {
                std::views::common(
                    g.get_nodes()
                    | std::views::transform([&](const auto& n) -> slot<N, E, WEIGHT> {
                        std::size_t in_degree { g.get_in_degree(n) };
                        // COUNT in_degree_narrowed { static_cast<COUNT>(in_degree) };
                        // if constexpr (debug_mode) {
                        //     if (in_degree_narrowed != in_degree) {
                        //         throw std::runtime_error("Narrowed but led to information loss");
                        //     }
                        // }
                        return { n, in_degree };
                    })
                )
            };
            slot_container_t slots {
                g,
                slots_lazy.begin(),
                slots_lazy.end(),
                container_creator_pack.create_slot_container_container_creator_pack()
            };
            // Create "ready_idxes" queue
            // --------------------------
            // The "ready_idxes" queue contains indicies within "slots" that are ready-to-process (node in that slot has had all
            // parents processed, and so it can be processed). Since root nodes have no parents, they are ready-to-process from
            // the get-go. As such, the "ready_idxes" queue is primed with the "slots" indices for root nodes (of which there
            // should be only one).
            ready_queue_t ready_idxes {
                container_creator_pack.create_ready_queue_container_creator_pack()
            };
            const N& root_node { g.get_root_node() };
            const auto& [root_slot_idx, root_slot] { slots.find(root_node) };
            ready_idxes.push(root_slot_idx);
            // static_assert(std::numeric_limits<float>::is_iec559, "IEEE 754 required"); // Require for inf and nan?
            root_slot.backtracking_weight = 0.0;
            // Find max path within graph
            // --------------------------
            // Using the backtracking algorithm, find the path within graph that has the maximum weight. If more than one such
            // path exists, any one of the paths will be returned.
            //
            // The backtracking algorithm is a "dynamic programming" algorithm that walks over each node. A node is walked once
            // all of its parent nodes have already been walked, where the process of "walking a node" is inspecting how much
            // weight has been accumulated within each incoming edge (weight at parent + the edge from parent to the node), and
            // selecting highest one as the edge to backtrack to.
            top:
            while (!ready_idxes.empty()) {
                std::size_t idx { ready_idxes.pop() };
                auto& current_slot { slots.at_idx(idx) };
                for (const auto& edge : g.get_inputs(current_slot.node)) {
                    const auto& src_node { g.get_edge_from(edge) };
                    if (slots.find_ref(src_node).unwalked_parent_cnt != 0) {
                        goto top;
                    }
                }
                auto incoming_accumulated {
                    std::views::common(
                        g.get_inputs(current_slot.node)
                        | std::views::transform(
                            [&](const auto& edge) noexcept -> std::pair<E, WEIGHT> {
                                const auto& src_node { g.get_edge_from(edge) };
                                const slot<N, E, WEIGHT>& src_node_slot { slots.find_ref(src_node) };
                                const auto& edge_weight { edge_weight_accessor(edge) };
                                return { edge, src_node_slot.backtracking_weight + edge_weight };
                            }
                        )
                    )
                };
                auto found {
                    // Ensure range is a common_view (begin() and end() are of same type)
                    std::ranges::max_element(
                        incoming_accumulated.begin(),
                        incoming_accumulated.end(),
                        [](const std::pair<E, WEIGHT>& a, const std::pair<E, WEIGHT>& b) noexcept {
                            return a.second < b.second;
                        }
                    )
                };
                if (found != incoming_accumulated.end()) {  // if no incoming nodes found, it's a root node
                    current_slot.backtracking_edge = (*found).first;
                    current_slot.backtracking_weight = (*found).second;
                }
                // For outgoing nodes this node points to, decrement its number of unprocessed parents (this node was one of its
                // parents, and it was processed in this iteration of the loop) then add it to "ready_idxes" if it has no more
                // unprocessed parents.
                for (const auto& edge : g.get_outputs(current_slot.node)) {
                    const auto& dst_node { g.get_edge_to(edge) };
                    const auto& [dst_slot_idx, dst_slot] { slots.find(dst_node) };
                    if constexpr (debug_mode) {
                        if (dst_slot.unwalked_parent_cnt == 0u) {
                            throw std::runtime_error("Invalid number of unprocessed parents");
                        }
                    }
                    dst_slot.unwalked_parent_cnt = static_cast<std::size_t>(dst_slot.unwalked_parent_cnt - 1u);
                    if (dst_slot.unwalked_parent_cnt == 0u) {
                        ready_idxes.push(dst_slot_idx);
                    }
                }
            }
            // Return slots container, which can go on to be used for backtracking
            return slots;
        }

        range_of_type<E> auto backtrack(
                G& g,
                slot_container_t& slots,
                const N& end_node
        ) {
            auto next_node { end_node };
            PATH_CONTAINER path {
                container_creator_pack.create_path_container()
            };
            while (true) {
                auto node { next_node };
                if (!g.has_inputs(node)) {
                    break;
                }
                const auto& node_slot { slots.find_ref(node) };
                path.push_back(node_slot.backtracking_edge);
                next_node = g.get_edge_from(node_slot.backtracking_edge);
            }
            // At this point, path is in reverse order (from last to first). Reverse it to get it into the forward order
            // (from first to last).
            std::reverse(path.begin(), path.end());
            return path;
        }

        auto find_max_path(
                G& graph,
                std::function<WEIGHT(const E&)> edge_weight_accessor
        ) {
            auto slots {
                populate_weights_and_backtrack_pointers(
                    graph,
                    edge_weight_accessor
                )
            };
            const auto& end_node { graph.get_leaf_node() };
            const auto& path { backtrack(graph, slots, end_node) };
            const auto& weight { slots.find_ref(end_node).backtracking_weight };
            return std::make_pair(path, weight);
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_BACKTRACKER_H
