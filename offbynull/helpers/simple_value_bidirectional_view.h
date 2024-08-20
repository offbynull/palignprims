#ifndef OFFBYNULL_HELPERS_SIMPLE_VALUE_BIDIRECTIONAL_VIEW_H
#define OFFBYNULL_HELPERS_SIMPLE_VALUE_BIDIRECTIONAL_VIEW_H

#include <ranges>
#include <functional>

namespace offbynull::helpers::simple_value_bidirectional_view {
    template<typename T>
    concept decayable_type = !std::is_void_v<T> && std::is_convertible_v<T, std::decay_t<T>>;

    template<typename T>
    concept state =
        std::semiregular<T>
        && requires(T self, const T const_self) {
            { self.to_prev() } -> std::same_as<void>;
            { self.to_next() } -> std::same_as<void>;
            { const_self.value() } -> decayable_type;  // Convertible to non-void type that decays (only needs copy constructor?)
        };

    template<state STATE>
    class iterator {
    private:
        STATE state;
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = decltype(state.value());
        using difference_type = std::ptrdiff_t;

        iterator(const iterator<STATE> &src) = default;

        iterator(iterator<STATE> &&src) = default;

        iterator(
            STATE state_
        )
        : state { state_ } {}

        iterator()
        : state {} {}

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

    template<state STATE>
    class simple_value_bidirectional_view : public std::ranges::view_interface<simple_value_bidirectional_view<STATE>> {
    private:
        STATE begin_state;
        STATE end_state;

    public:
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

        iterator<STATE> begin() const noexcept {
            return { begin_state };
        }

        iterator<STATE> end() const noexcept {
            return { end_state };
        }
    };

    // Proper placement of the deduction guide
    template<std::regular VALUE, std::semiregular STATE>
    simple_value_bidirectional_view(STATE&&, STATE&&) -> simple_value_bidirectional_view<STATE>;

    template<std::regular VALUE, std::semiregular STATE, typename Adaptor>
    auto operator|(const simple_value_bidirectional_view<STATE>& v, Adaptor adaptor) {
        return adaptor(std::views::all(v));
    }

    template<std::regular VALUE, std::semiregular STATE, typename Adaptor>
    auto operator|(simple_value_bidirectional_view<STATE>&& v, Adaptor adaptor) {
        return adaptor(std::views::all(std::move(v)));
    }
}

#endif //OFFBYNULL_HELPERS_SIMPLE_VALUE_BIDIRECTIONAL_VIEW_H