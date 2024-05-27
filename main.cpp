#include <iostream>
#include <stdfloat>
#include <cstddef>
#include "offbynull/aligner/aligner.h"

int main() {
    using offbynull::aligner::aligner::align_heap;
    using offbynull::aligner::aligner::align_stack;
    using offbynull::aligner::aligner::alignment_mode;

    auto weight_lookup {
        [](const auto& v_elem, const auto& w_elem) -> std::float16_t {
            if (!v_elem.has_value() || !w_elem.has_value()) {
                return 0.0;
            }
            return v_elem.value().get() == w_elem.value().get() ? 1.0 : 0.0;
        }
    };
    auto print {
        [](auto& elements) {
            std::string v_align {};
            std::string w_align {};
            for (const auto& [v_elem_opt, w_elem_opt] : elements) {
                v_align += (v_elem_opt.has_value() ? *v_elem_opt : '-');
                w_align += (w_elem_opt.has_value() ? *w_elem_opt : '-');
            }
            std::cout << v_align << std::endl;
            std::cout << w_align << std::endl;
        }
    };

    std::array<char, 5> v { 'h', 'e', 'l', 'l', 'o' };
    std::array<char, 6> w { 'm', 'e', 'l', 'l', 'o', 'w' };

    // dynamic
    {
        auto [elements, weight] {
            align_heap<alignment_mode::GLOBAL, std::uint8_t, std::float16_t, false>(
                v,
                w,
                weight_lookup
            )
        };
        print(elements);
    }
    // static
    {
        auto [elements, weight] {
            align_stack<alignment_mode::GLOBAL, std::float16_t, v.size(), w.size(), false>(
                v,
                w,
                weight_lookup
            )
        };
        print(elements);
    }

    return 0;
}