#ifndef OFFBYNULL_HELPERS_SIMPLE_VALUE_BIDIRECTIONAL_VIEW_H
#define OFFBYNULL_HELPERS_SIMPLE_VALUE_BIDIRECTIONAL_VIEW_H

#include <cstdint>
#include <ranges>
#include <iterator>
#include <concepts>
#include "offbynull/concepts.h"

namespace offbynull::helpers::simple_value_bidirectional_view {
    using offbynull::concepts::unqualified_object_type;

    /**
     * Concept that's satisfied if `T` has the traits of a simplified bidirectional iterator. Note that, while normal C++ iterators can
     * return values or references, this iterator only ever returns values (copies).
     *
     * @tparam T Type to check.
     */
    template<typename T>
    concept state =
        unqualified_object_type<T>
        && std::semiregular<T>
        && requires(T self, const T const_self) {
            // Move internal pointer back by one element. Moving back before the first element is undefined.
            { self.to_prev() } -> std::same_as<void>;
            // Move internal pointer forward by one element. Moving forward 1 past the last element is undefined (1 past last = end marker).
            { self.to_next() } -> std::same_as<void>;
            // Get copy of value at internal pointer.
            { const_self.value() } -> unqualified_object_type;
        };

    /**
     * Bidirectional iterator wrapper around @ref offbynull::helpers::simple_value_bidirectional_view::state. Note that this iterator
     * returns value types (copies), not references.
     *
     * @tparam STATE Type to wrap as a bidirectional iterator.
     */
    template<state STATE>
    class iterator {
    private:
        STATE state;
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = decltype(state.value());
        using difference_type = std::ptrdiff_t;

        /**
         * Construct an @ref offbynull::helpers::simple_value_bidirectional_view::iterator instance.
         *
         * @param state_ State of this bidirectional iterator.
         */
        iterator(
            STATE state_
        )
        : state { state_ } {}

        /**
         * Construct an @ref offbynull::helpers::simple_value_bidirectional_view::iterator instance that has no elements.
         */
        iterator()
        : state {} {}

        iterator(const iterator<STATE> &src) = default;
        iterator(iterator<STATE> &&src) noexcept = default;
        iterator<STATE>& operator=(const iterator<STATE>& rhs) = default;

        value_type operator*() const {
            return state.value();
        }

        iterator<STATE>& operator++() {
            state.to_next();
            return *this;
        }

        iterator<STATE> operator++(int) {
            iterator<STATE> tmp = *this;
            ++(*this);
            return tmp;
        }

        iterator<STATE>& operator--() {
            state.to_prev();
            return *this;
        }

        iterator<STATE> operator--(int) {
            iterator<STATE> tmp = *this;
            --(*this);
            return tmp;
        }

        bool operator==(const auto& other) const {
            return state == other.state;
        }

        bool operator!=(const auto& other) const {
            return !(*this == other);
        }
    };

    /**
     * Bidirectional view wrapper around @ref offbynull::helpers::simple_value_bidirectional_view::state.
     *
     * @tparam STATE Type to wrap as a bidirectional view.
     */
    template<state STATE>
    class simple_value_bidirectional_view : public std::ranges::view_interface<simple_value_bidirectional_view<STATE>> {
    private:
        STATE begin_state;
        STATE end_state;

    public:
        /**
         * Construct an @ref offbynull::helpers::simple_value_bidirectional_view::simple_value_bidirectional_view instance.
         *
         * @param begin_state_ State at the first position of this bidirectional view.
         * @param end_state_ State at the last position of this bidirectional view (one position past the last element).
         */
        simple_value_bidirectional_view(
            STATE begin_state_,
            STATE end_state_
        )
        : begin_state { begin_state_ }
        , end_state { end_state_ } {}

        simple_value_bidirectional_view(const simple_value_bidirectional_view<STATE> &src) = default;
        simple_value_bidirectional_view(simple_value_bidirectional_view<STATE> &&src) = default;
        simple_value_bidirectional_view<STATE>& operator=(const simple_value_bidirectional_view<STATE>& other) = default;
        simple_value_bidirectional_view<STATE>& operator=(simple_value_bidirectional_view<STATE>&& other) = default;

        iterator<STATE> begin() const {
            return { begin_state };
        }

        iterator<STATE> end() const {
            return { end_state };
        }
    };

    /**
     * @ref offbynull::helpers::simple_value_bidirectional_view::simple_value_bidirectional_view template deduction guide.
     *
     * @tparam STATE Type backing @ref offbynull::helpers::simple_value_bidirectional_view::simple_value_bidirectional_view.
     * @return No return (this is a template deduction guide).
     */
    template<state STATE>
    simple_value_bidirectional_view(STATE&&, STATE&&) -> simple_value_bidirectional_view<STATE>;
}

#endif //OFFBYNULL_HELPERS_SIMPLE_VALUE_BIDIRECTIONAL_VIEW_H