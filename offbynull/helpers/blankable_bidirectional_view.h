#ifndef OFFBYNULL_HELPERS_BLANKABLE_BIDIRECTIONAL_VIEW_H
#define OFFBYNULL_HELPERS_BLANKABLE_BIDIRECTIONAL_VIEW_H

#include <ranges>
#include <utility>

/**
 * View that wraps a bidirectional range, optionally making it appear as if its blank.
 *
 * @author Kasra Faghihi
 */
namespace offbynull::helpers::blankable_bidirectional_view {

    /**
     * View that wraps a bidirectional range, optionally making it appear as if its blank.
     *
     * @tparam R Wrapped bidirectional range type.
     */
    template<std::ranges::bidirectional_range R>
    class blankable_bidirectional_view : public std::ranges::view_interface<blankable_bidirectional_view<R>> {
    private:
        bool passthrough;
        decltype(std::declval<R>() | std::views::common) range;

    public:
        /**
         * Construct an @ref offbynull::helpers::blankable_bidirectional_view::blankable_bidirectional_view instance.
         *
         * @param passthrough_ Pass through flag. If `false`, this view will appear as if it's empty.
         * @param range_ Range to wrap.
         */
        blankable_bidirectional_view(
            bool passthrough_,
            R&& range_
        )
        : passthrough { passthrough_ }
        , range { std::forward<R>(range_) | std::views::common } {}
        blankable_bidirectional_view(const blankable_bidirectional_view<R> &src) = default;
        blankable_bidirectional_view(blankable_bidirectional_view<R> &&src) = default;
        blankable_bidirectional_view<R>& operator=(const blankable_bidirectional_view<R>& other) = default;
        blankable_bidirectional_view<R>& operator=(blankable_bidirectional_view<R>&& other) = default;

        auto begin() const {
            return passthrough ? range.begin() : range.end();
        }

        auto end() const {
            return range.end();
        }
    };

    /**
     * @ref offbynull::helpers::blankable_bidirectional_view::blankable_bidirectional_view template deduction guide.
     *
     * @tparam R Wrapped bidirectional range type.
     * @return No return (this is a template deduction guide).
     */
    template<std::ranges::bidirectional_range R>
    blankable_bidirectional_view(bool, R&&) -> blankable_bidirectional_view<R>;

    /**
     * `const` @ref offbynull::helpers::blankable_bidirectional_view::blankable_bidirectional_view range pipe operator, where the `v` is
     * fed into `adaptor`.
     *
     * @tparam R Wrapped bidirectional range type.
     * @tparam Adaptor Type being piped to.
     * @param v Range to pipe from.
     * @param adaptor Range adaptor to pipe into.
     * @return `v` piped into `adaptor`.
     */
    template<std::ranges::bidirectional_range R, typename Adaptor>
    auto operator|(const blankable_bidirectional_view<R>& v, Adaptor adaptor) {
        return adaptor(std::views::all(v));
    }

    /**
     * @ref offbynull::helpers::blankable_bidirectional_view::blankable_bidirectional_view range pipe operator, where the `v` is fed into
     * `adaptor` (via move).
     *
     * @tparam R Wrapped bidirectional range type.
     * @tparam Adaptor Type being piped to.
     * @param v Range to pipe from.
     * @param adaptor Range adaptor to pipe into.
     * @return `v` piped into `adaptor`.
     */
    template<std::ranges::bidirectional_range R, typename Adaptor>
    auto operator|(blankable_bidirectional_view<R>&& v, Adaptor adaptor) {
        return adaptor(std::views::all(std::move(v)));
    }
}

#endif //OFFBYNULL_HELPERS_BLANKABLE_BIDIRECTIONAL_VIEW_H