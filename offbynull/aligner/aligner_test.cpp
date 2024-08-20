#include <stdfloat>
#include <string>
#include "offbynull/aligner/aligner.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::aligner::aligner::global_align_heap;
    using offbynull::aligner::aligner::local_align_heap;
    using offbynull::aligner::aligner::fitting_align_heap;
    using offbynull::aligner::aligner::extended_gap_align_heap;

    std::float16_t weight_lookup(
        const std::optional<std::reference_wrapper<const char>>& v_elem,
        const std::optional<std::reference_wrapper<const char>>& w_elem
    ) {
        if (!v_elem.has_value() || !w_elem.has_value()) {
            return -1.0f16;
        }
        return v_elem.value().get() == w_elem.value().get() ? 1.0f16 : -1.0f16;
    }

    std::pair<std::string, std::string> print(auto& elements) {
        std::string v_align {};
        std::string w_align {};
        for (const auto& [v_elem_opt, w_elem_opt] : elements) {
            v_align += (v_elem_opt.has_value() ? *v_elem_opt : '-');
            w_align += (w_elem_opt.has_value() ? *w_elem_opt : '-');
        }
        std::cout << v_align << std::endl;
        std::cout << w_align << std::endl;
        return { v_align, w_align };
    }

    TEST(OAAlignerTest, AlignHeapGlobal) {
        std::string v { "hello" };
        std::string w { "mellow" };
        auto [elements, weight] {
            global_align_heap<std::uint8_t, std::float16_t, false>(
                v,
                w,
                weight_lookup
            )
        };
        const auto& [v_align, w_align] { print(elements) };
    }

    TEST(OAAlignerTest, AlignHeapLocal) {
        std::string v { "abcdefg hello hijklmnop" };
        std::string w { "zzzzzzzz mellow ffffffffff" };
        auto [elements, weight] {
            local_align_heap<std::uint8_t, std::float16_t, false>(
                v,
                w,
                weight_lookup
            )
        };
        const auto& [v_align, w_align] { print(elements) };
    }

    TEST(OAAlignerTest, AlignHeapFitting) {
        std::string v { "abcdefg hello hijklmnop" };
        std::string w { "mellow" };
        auto [elements, weight] {
            fitting_align_heap<std::uint8_t, std::float16_t, false>(
                v,
                w,
                weight_lookup
            )
        };
        const auto& [v_align, w_align] { print(elements) };
    }

    TEST(OAAlignerTest, AlignHeapExtendedGap) {
        std::string v { "abcdefg hello hijklmnop" };
        std::string w { "mellow" };
        auto [elements, weight] {
            extended_gap_align_heap<std::uint8_t, std::float16_t, false>(
                v,
                w,
                weight_lookup
            )
        };
        const auto& [v_align, w_align] { print(elements) };
    }
}