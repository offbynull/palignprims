#ifndef OFFBYNULL_HELPERS_BLANKABLE_BIDIRECTIONAL_VIEW_H
#define OFFBYNULL_HELPERS_BLANKABLE_BIDIRECTIONAL_VIEW_H

#include <ranges>
#include <functional>

namespace offbynull::helpers::blankable_bidirectional_view {

    template<std::ranges::bidirectional_range R>
    class blankable_bidirectional_view : public std::ranges::view_interface<blankable_bidirectional_view<R>>{
    private:
        bool passthrough;
        decltype(std::declval<R>() | std::views::common) range;

    public:
        blankable_bidirectional_view(
            bool passthrough,
            R&& range
        )
        : passthrough { passthrough }
        , range { std::forward<R>(range) | std::views::common } { }
        blankable_bidirectional_view(const blankable_bidirectional_view<R> &src) = default;
        blankable_bidirectional_view(blankable_bidirectional_view<R> &&src) = default;
        blankable_bidirectional_view<R>& operator=(const blankable_bidirectional_view<R>& other) = default;
        blankable_bidirectional_view<R>& operator=(blankable_bidirectional_view<R>&& other) = default;

        auto begin() const noexcept {
            return passthrough ? range.begin() : range.end();
        }

        auto end() const noexcept {
            return range.end();
        }
    };

    // Proper placement of the deduction guide
    template<std::ranges::bidirectional_range R>
    blankable_bidirectional_view(bool, R&&) -> blankable_bidirectional_view<R>;

    template<std::ranges::bidirectional_range R, typename Adaptor>
    auto operator|(const blankable_bidirectional_view<R>& v, Adaptor adaptor) {
        return adaptor(std::views::all(v));
    }

    template<std::ranges::bidirectional_range R, typename Adaptor>
    auto operator|(blankable_bidirectional_view<R>&& v, Adaptor adaptor) {
        return adaptor(std::views::all(std::move(v)));
    }
}

#endif //OFFBYNULL_HELPERS_BLANKABLE_BIDIRECTIONAL_VIEW_H