#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_PATH_CONTAINER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_PATH_CONTAINER_H

#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/forward_walker.h"
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <ranges>
#include <vector>
#include <array>
#include <utility>
#include <stdexcept>

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container {
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_pairwise_alignment_graph;
    using offbynull::concepts::random_access_range_of_type;
    using offbynull::utils::static_vector_typer;

    template<typename E>
    struct element {
        element<E>* prev {};
        element<E>* next {};
        E backtracking_edge {};

        element() = default;

        bool operator==(const element &) const = default;
    };








    template<readable_sliceable_pairwise_alignment_graph G>
    class backward_walker_iterator {
    private:
        using N = typename G::N;
        using E = typename G::E;

        element<E>* current;

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = E;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        backward_walker_iterator()
        : current {} {}

        backward_walker_iterator(element<E>* tail)
        : current { tail } {}

        reference operator*() const {
            return current->backtracking_edge;
        }

        backward_walker_iterator& operator++() {
            current = current->prev;
            return *this;
        }

        backward_walker_iterator operator++(int) {
            backward_walker_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const backward_walker_iterator& other) const {
            return current == other.current;
        }
    };

    template<readable_sliceable_pairwise_alignment_graph G>
    struct backward_walker_range {
    private:
        using E = typename G::E;

        element<E>* head;
        element<E>* tail;

    public:
        backward_walker_range(
            element<E>* head_,
            element<E>* tail_
        )
        : head { head_ }
        , tail { tail_ } {}

        backward_walker_iterator<G> begin() const {
            return backward_walker_iterator<G> { tail };
        }

        backward_walker_iterator<G> end() const {
            return backward_walker_iterator<G> { nullptr };
        }
    };









    template<
        typename T,
        typename E
    >
    concept path_container_container_creator_pack =
        requires(const T t, std::size_t path_edge_capacity) {
            { t.create_element_container(path_edge_capacity) } -> random_access_range_of_type<element<E>>;
        };

    template<
        bool debug_mode,
        typename E
    >
    struct path_container_heap_container_creator_pack {
        std::vector<element<E>> create_element_container(std::size_t path_edge_capacity) const {
            return std::vector<element<E>>(path_edge_capacity);
        }
    };

    template<
        bool debug_mode,
        typename E,
        std::size_t path_edge_capacity
    >
    struct path_container_stack_container_creator_pack {
        std::array<
            element<E>,
            path_edge_capacity
        > create_element_container(std::size_t path_edge_capacity_) const {
            if constexpr (debug_mode) {
                if (path_edge_capacity != path_edge_capacity_) {
                    throw std::runtime_error("Bad element count");
                }
            }
            return {};
        }
    };








    template<
        bool debug_mode,
        readable_sliceable_pairwise_alignment_graph G,
        path_container_container_creator_pack<
            typename G::E
        > CONTAINER_CREATOR_PACK = path_container_heap_container_creator_pack<
            debug_mode,
            typename G::E
        >
    >
    class path_container {
    private:
        using E = typename G::E;

        using ELEMENT_CONTAINER = decltype(std::declval<CONTAINER_CREATOR_PACK>().create_element_container(0zu));

        ELEMENT_CONTAINER element_container;
        element<E>* head;
        element<E>* tail;
        std::size_t next_idx;

    public:
        path_container(
            const G& g,
            CONTAINER_CREATOR_PACK container_creator_pack = {}
        )
        : element_container {
            container_creator_pack.create_element_container(g.path_edge_capacity)
        }
        , head { nullptr }
        , tail { nullptr }
        , next_idx { 0zu } {}

        // Custom copy/move/copy assignment/move assigned because this class has raw pointer types as members. The default copy/assignment
        // will do a SHALLOW copy of these pointers, meaning they won't be pointing into the copy'd element_container (they'll instead be
        // pointing into the original element_container).
        path_container(const path_container& other)
        : element_container { other.element_container }
        , head { other.head == nullptr ? nullptr : &element_container[other.head - &(other.element_container[0zu])] }
        , tail { other.tail == nullptr ? nullptr : &element_container[other.tail - &(other.element_container[0zu])] }
        , next_idx { other.next_idx } {
            for (std::size_t i { 0zu }; i < other.next_idx; ++i) {
                element<E>& other_element { element_container[i] };
                element<E>& this_element { element_container[i] };
                this_element.backtracking_edge = other_element.backtracking_edge;
                this_element.prev = &element_container[other_element.prev - &(other.element_container[0zu])];
                this_element.next = &element_container[other_element.next - &(other.element_container[0zu])];
            }
        }

        path_container(path_container&& other)
        : element_container { other.element_container }
        , head { other.head == nullptr ? nullptr : &element_container[other.head - &(other.element_container[0zu])] }
        , tail { other.tail == nullptr ? nullptr : &element_container[other.tail - &(other.element_container[0zu])] }
        , next_idx { other.next_idx } {
            for (std::size_t i { 0zu }; i < other.next_idx; ++i) {
                element<E>& other_element { element_container[i] };
                element<E>& this_element { element_container[i] };
                this_element.backtracking_edge = other_element.backtracking_edge;
                this_element.prev = &element_container[other_element.prev - &(other.element_container[0zu])];
                this_element.next = &element_container[other_element.next - &(other.element_container[0zu])];
            }
            other.head = nullptr;
            other.tail = nullptr;
            other.next_idx = 0zu;
        }

        path_container& operator=(const path_container& other) {
            if (this != &other) { // guard against self-assignment
                element_container = other.element_container;
                head = other.head == nullptr ? nullptr : &element_container[other.head - &(other.element_container[0zu])];
                tail = other.tail == nullptr ? nullptr : &element_container[other.tail - &(other.element_container[0zu])];
                next_idx = other.next_idx;
                for (std::size_t i { 0zu }; i < other.next_idx; ++i) {
                    element<E>& other_element { element_container[i] };
                    element<E>& this_element { element_container[i] };
                    this_element.backtracking_edge = other_element.backtracking_edge;
                    this_element.prev = &element_container[other_element.prev - &(other.element_container[0zu])];
                    this_element.next = &element_container[other_element.next - &(other.element_container[0zu])];
                }
            }
            return *this;
        }

        path_container& operator=(path_container&& other) {
            if (this != &other) { // guard against self-assignment
                element_container = other.element_container;
                head = other.head == nullptr ? nullptr : &element_container[other.head - &(other.element_container[0zu])];
                tail = other.tail == nullptr ? nullptr : &element_container[other.tail - &(other.element_container[0zu])];
                next_idx = other.next_idx;
                for (std::size_t i { 0zu }; i < other.next_idx; ++i) {
                    element<E>& other_element { element_container[i] };
                    element<E>& this_element { element_container[i] };
                    this_element.backtracking_edge = other_element.backtracking_edge;
                    this_element.prev = &element_container[other_element.prev - &(other.element_container[0zu])];
                    this_element.next = &element_container[other_element.next - &(other.element_container[0zu])];
                    other.head = nullptr;
                    other.tail = nullptr;
                    other.next_idx = 0zu;
                }
            }
            return *this;
        }

        element<E>* initialize(const E& backtracking_edge) {
            if constexpr (debug_mode) {
                if (next_idx != 0zu) {
                    throw std::runtime_error("Already initialized");
                }
                if (next_idx >= element_container.size()) {
                    // If this happens, G::path_edge_capacity is probably giving back a number that's too low
                    throw std::runtime_error("Container too small");
                }
            }
            element<E>* entry { &element_container[0zu] };
            entry->backtracking_edge = backtracking_edge;
            ++next_idx;
            head = entry;
            tail = entry;
            return entry;
        }

        element<E>* push_prefix(element<E>* entry, const E& backtracking_edge) {
            if constexpr (debug_mode) {
                if (next_idx == 0zu) {
                    throw std::runtime_error("Not initialized");
                }
                if (next_idx >= element_container.size()) {
                    // If this happens, G::path_edge_capacity is probably giving back a number that's too low
                    throw std::runtime_error("Container too small");
                }
            }
            element<E>* prefix_entry { &element_container[next_idx] };
            prefix_entry->backtracking_edge = backtracking_edge;
            ++next_idx;
            element<E>* orig_prev { entry->prev };
            if (orig_prev != nullptr) {
                orig_prev->next = prefix_entry;
            }
            prefix_entry->prev = orig_prev;
            entry->prev = prefix_entry;
            prefix_entry->next = entry;
            if (entry == head) {
                head = prefix_entry;
            }
            return prefix_entry;
        }

        element<E>* push_suffix(element<E>* entry, const E& backtracking_edge) {
            if constexpr (debug_mode) {
                if (next_idx == 0zu) {
                    throw std::runtime_error("Not initialized");
                }
                if (next_idx >= element_container.size()) {
                    // If this happens, G::path_edge_capacity is probably giving back a number that's too low
                    throw std::runtime_error("Container too small");
                }
            }
            element<E>* suffix_entry { &element_container[next_idx] };
            suffix_entry->backtracking_edge = backtracking_edge;
            ++next_idx;
            element<E>* orig_next { entry->next };
            if (orig_next != nullptr) {
                orig_next->prev = suffix_entry;
            }
            suffix_entry->next = orig_next;
            entry->next = suffix_entry;
            suffix_entry->prev = entry;
            if (entry == tail) {
                tail = suffix_entry;
            }
            return suffix_entry;
        }

        std::ranges::forward_range auto walk_path_backward() {
            return backward_walker_range<G> { head, tail };
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_PATH_CONTAINER_H
