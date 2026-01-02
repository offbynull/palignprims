#ifndef OFFBYNULL_HELPERS_FILTER_BIDIRECTIONAL_VIEW_H
#define OFFBYNULL_HELPERS_FILTER_BIDIRECTIONAL_VIEW_H

#include <ranges>
#include <utility>
#include <type_traits>
#include <iterator>
#include <ranges>
#include <concepts>
#include "offbynull/helpers/movable_box.h"

namespace offbynull::helpers::filter_bidirectional_view {
    using offbynull::helpers::movable_box::movable_box;

    // forward decl
    template<std::ranges::bidirectional_range R, typename PREDICATE, bool const_>
    requires requires(
        PREDICATE p,
        std::ranges::range_reference_t<std::conditional_t<const_, const R, R>>& v
    ) {
        { p(v) } -> std::convertible_to<bool>;
    }
    class filter_bidirectional_iterator;

    template<std::ranges::bidirectional_range R, typename PREDICATE, bool const_>
    class filter_bidirectional_sentinel {
    private:
        using R_ = std::conditional_t<const_, const R, R>;
        using S = std::ranges::sentinel_t<R_>;

        S s;

    public:
        filter_bidirectional_sentinel() requires (std::default_initializable<S>) : s {} {}
        filter_bidirectional_sentinel(const filter_bidirectional_sentinel<R, PREDICATE, const_> &src) = default;
        filter_bidirectional_sentinel(filter_bidirectional_sentinel<R, PREDICATE, const_> &&src) noexcept = default;
        filter_bidirectional_sentinel(S s_) : s { s_ } {}

        filter_bidirectional_sentinel& operator=(const filter_bidirectional_sentinel&) = default;
        filter_bidirectional_sentinel& operator=(filter_bidirectional_sentinel&&) noexcept = default;


        friend class filter_bidirectional_iterator<R, PREDICATE, const_>;
    };
    
    template<std::ranges::bidirectional_range R, typename PREDICATE, bool const_>
    requires requires(
        PREDICATE p,
        std::ranges::range_reference_t<std::conditional_t<const_, const R, R>>& v
    ) {
        { p(v) } -> std::convertible_to<bool>;
    }
    class filter_bidirectional_iterator {
    private:
        using R_ = std::conditional_t<const_, const R, R>;
        using I = std::ranges::iterator_t<R_>;
        using S = std::ranges::sentinel_t<R_>;

        I b;
        I it;
        S s;
        movable_box<PREDICATE> p;

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = std::iter_value_t<I>;
        using difference_type = std::iter_difference_t<I>;
        using reference = std::iter_reference_t<I>;

        filter_bidirectional_iterator()
        requires (std::default_initializable<I> && std::default_initializable<S>)
        : b {}, it {}, s {}, p {} {}
        filter_bidirectional_iterator(const filter_bidirectional_iterator<R, PREDICATE, const_> &src) = default;
        filter_bidirectional_iterator(filter_bidirectional_iterator<R, PREDICATE, const_> &&src) noexcept = default;
        filter_bidirectional_iterator(I b_, I it_, S s_, PREDICATE p_) : b { b_ }, it { it_ }, s { s_ }, p { p_ } {
            while (it != s && !(p.get()(*it))) {
                ++it;
            }
        }

        filter_bidirectional_iterator& operator=(const filter_bidirectional_iterator&) = default;
        filter_bidirectional_iterator& operator=(filter_bidirectional_iterator&&) noexcept = default;


        // If either iterator does not return a reference, reference should be a value type due to how iter_reference_t works?
        reference operator*() const {
            return *it;
        }

        filter_bidirectional_iterator<R, PREDICATE, const_>& operator++() {
            ++it;
            while (it != s && !(p.get()(*it))) {
                ++it;
            }
            return *this;
        }
        filter_bidirectional_iterator<R, PREDICATE, const_> operator++(int) {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        filter_bidirectional_iterator<R, PREDICATE, const_>& operator--() {
            --it;
            while (it != b && !(p.get()(*it))) {
                --it;
            }
            return *this;
        }
        filter_bidirectional_iterator<R, PREDICATE, const_> operator--(int) {
            auto tmp = *this;
            --(*this);
            return tmp;
        }

        bool operator==(
            const filter_bidirectional_iterator<R, PREDICATE, const_>& other
        ) const {
            return it == other.it;
        }
        
        bool operator==(
            const filter_bidirectional_sentinel<R, PREDICATE, const_>& other
        ) const {
            return it == other.s;
        }
    };

    /**
     * View that filters some inner range without caching (unlike std::views::filter). Useful for supporting const ranges, which
     * std::views::filter doesn't allow because it caches on access.
     *
     * @tparam R Wrapped bidirectional range type.
     */
    template<std::ranges::bidirectional_range R, typename PREDICATE>
    class filter_bidirectional_view : public std::ranges::view_interface<filter_bidirectional_view<R, PREDICATE>> {
    private:
        R r;
        movable_box<PREDICATE> p;
    public:
        /** Iterator type. */
        using I = filter_bidirectional_iterator<R, PREDICATE, false>;
        /** Iterator sentinel type. */
        using S = filter_bidirectional_sentinel<R, PREDICATE, false>;
        /** Constant iterator type. */
        using IC = filter_bidirectional_iterator<R, PREDICATE, true>;
        /** Constant iterator sentinel type. */
        using SC = filter_bidirectional_sentinel<R, PREDICATE, true>;

        /**
         * Construct an @ref offbynull::helpers::filter_bidirectional_view::filter_bidirectional_view instance.
         *
         * @param r_ Range to wrap.
         * @param p_ Filter predicate.
         */
        filter_bidirectional_view(auto&& r_, auto&& p_)
        : r(std::forward<decltype(r_)>(r_))
        , p { std::forward<decltype(p_)>(p_) } {}

        filter_bidirectional_view(const filter_bidirectional_view<R, PREDICATE> &src) = default;
        filter_bidirectional_view(filter_bidirectional_view<R, PREDICATE> &&src) noexcept = default;
        filter_bidirectional_view<R, PREDICATE>& operator=(const filter_bidirectional_view<R, PREDICATE>& other) = default;
        filter_bidirectional_view<R, PREDICATE>& operator=(filter_bidirectional_view<R, PREDICATE>&& other) noexcept = default;


        /**
         * Get begin iterator.
         *
         * @return begin iterator.
         */
        I begin() {
            auto b = std::ranges::begin(r);
            auto e = std::ranges::end(r);
            return { b, b, e, p.get() };
        }
        /**
         * Get end iterator.
         *
         * @return end iterator.
         */
        S end() {
            return { std::ranges::end(r) };
        }
        /**
         * Get end iterator.
         *
         * @return end iterator.
         */
        I end() requires std::ranges::common_range<R> {
            auto b = std::ranges::begin(r);
            auto e = std::ranges::end(r);
            return { b, e, e, p.get() };
        }

        /**
         * Get begin iterator.
         *
         * @return begin iterator (const).
         */
        IC begin() const {
            auto b = std::ranges::begin(r);
            auto e = std::ranges::end(r);
            return { b, b, e, p.get() };
        }
        /**
         * Get end iterator.
         *
         * @return end iterator (const).
         */
        SC end() const {
            return { std::ranges::end(r) };
        }
        /**
         * Get end iterator.
         *
         * @return end iterator (const).
         */
        IC end() const requires std::ranges::common_range<R> {
            auto b = std::ranges::begin(r);
            auto e = std::ranges::end(r);
            return { b, e, e, p.get() };
        }
    };

    template<typename R, typename P>
    filter_bidirectional_view(R&&, P&&) -> filter_bidirectional_view<std::remove_cvref_t<R>, std::remove_cvref_t<P>>;

    template<typename P>
    struct filter_bidirectional_closure {
        P p;

        template<typename R>
        auto operator()(R&& r) const {
            using V = std::views::all_t<R>;
            return filter_bidirectional_view<V, P> {
                std::views::all(std::forward<R>(r)),
                p
            };
        }

        template<typename R>
        friend auto operator|(R&& r, const filter_bidirectional_closure& c) {
            return c(std::forward<R>(r));
        }
    };

    template<typename P>
    auto filter_bidirectional(P&& p) {
        return filter_bidirectional_closure<std::remove_cvref_t<P>> {
            { std::forward<P>(p) }
        };
    }

    template<typename R, typename P>
    auto filter_bidirectional(R&& r, P&& p) {
        return filter_bidirectional(
            { std::forward<P>(p) }
        )(std::forward<R>(r));
    }
}

#endif //OFFBYNULL_HELPERS_FILTER_BIDIRECTIONAL_VIEW_H