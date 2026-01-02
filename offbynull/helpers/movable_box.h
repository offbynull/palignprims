#ifndef OFFBYNULL_HELPERS_MOVABLE_BOX_H
#define OFFBYNULL_HELPERS_MOVABLE_BOX_H

#include <optional>
#include <utility>
#include <ranges>

namespace offbynull::helpers::movable_box {
    // What's the point of this class? It nests a type and exposes move assignment even when the nested type doesn't support it (nested type
    // most support move constructor). Why is it important? Imagine making your own range view and accepting a lambda. If that lambda has a
    // capture, it will no longer be move assignable. That means expressions like these will fail:
    //
    // std::move(r) | std::views::transform([x] (auto y) { return x+y; } | std::views::reverse;
    //
    // See https://chatgpt.com/share/69592099-e10c-8008-9b9a-1cefc2c019b3

    template<typename T>
    struct movable_box {
        std::optional<T> v;

        movable_box() : v {} {}
        template<typename U>
        movable_box(U&& u) : v { std::in_place, std::forward<U>(u) } {}
        movable_box(const movable_box&) = default;
        movable_box(movable_box&&) noexcept = default;

        movable_box& operator=(const movable_box& o) {
            v.reset();
            if (o.v) {
                v.emplace(*o.v);
            }
            return *this;
        }
        movable_box& operator=(movable_box&& o) noexcept {
            v.reset();
            if (o.v) {
                v.emplace(std::move(*o.v));
            }
            return *this;
        }
        movable_box& operator=(const T& o) {
            v.emplace(o);
            return *this;
        }
        movable_box& operator=(T&& o) noexcept {
            v.reset();
            v.emplace(std::move(o));
            return *this;
        }

        T& get() { return *v; }
        const T& get() const { return *v; }
    };
}


#endif //OFFBYNULL_HELPERS_MOVABLE_BOX_H