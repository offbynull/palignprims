#ifndef OFFBYNULL_ALIGNER_ALIGNERS_UTILS_H
#define OFFBYNULL_ALIGNER_ALIGNERS_UTILS_H

#include "offbynull/concepts.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/aligners/concepts.h"
#include "offbynull/aligner/sequence/sequence.h"
#include <algorithm>
#include <format>
#include <ranges>
#include <vector>
#include <string>
#include <stdexcept>

namespace offbynull::aligner::aligners::utils {
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::concepts::unqualified_object_type;
    using offbynull::aligner::sequence::sequence::sequence;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::aligners::concepts::bidirectional_range_of_alignment_index_pairs;

    /**
     * Convert pairwise alignment to string, stacking the two aligned sequences on top of each other. For example, "hello" vs "mellow" may
     * output as ...
     *
     * ```
     * h-ello--
     * -mello-w
     * ```
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @param down Down sequence.
     * @param right Right sequence.
     * @param alignment_indices Pairwise alignment indexes between `down` and `right`.
     * @param separator String to inject between sequence elements.
     * @param gap String to inject when sequence element is a gap.
     * @param freeride String to inject when sequence elements are freeride.
     * @param padding String to pad elements with (such that in the output, each element in `down` is aligned with its corresponding element
     *     in `right` - this may be needed if elements end up as variable length strings).
     */
    template<bool debug_mode>
    std::string alignment_to_stacked_string(
        const sequence auto& down,
        const sequence auto& right,
        const bidirectional_range_of_alignment_index_pairs auto& alignment_indices,
        const std::string separator = std::string {},
        const std::string gap = std::string { "-" },
        const std::string freeride = std::string { "-" },
        const char padding = ' '
    ) {
        std::vector<std::string> d_elem_strs {};
        std::vector<std::string> r_elem_strs {};
        for (const auto& idxes : alignment_indices) {
            if (idxes.has_value()) {
                const auto& [d_idx, r_idx] { *idxes };
                if (d_idx.has_value() && r_idx.has_value()) {
                    const auto& d_elem { down[*d_idx ] };
                    const auto& r_elem { right[*r_idx ] };
                    d_elem_strs.push_back(std::format("{}", d_elem));
                    r_elem_strs.push_back(std::format("{}", r_elem));
                } else if (d_idx.has_value() && !r_idx.has_value()) {
                    const auto& d_elem { down[*d_idx ] };
                    d_elem_strs.push_back(std::format("{}", d_elem));
                    r_elem_strs.push_back(gap);
                } else if (!d_idx.has_value() && r_idx.has_value()) {
                    const auto& r_elem { right[*r_idx ] };
                    d_elem_strs.push_back(gap);
                    r_elem_strs.push_back(std::format("{}", r_elem));
                } else {
                    if constexpr (debug_mode) {
                        throw std::runtime_error { "This should never happen" };
                    }
                }
            } else {
                d_elem_strs.push_back(freeride);
                r_elem_strs.push_back(freeride);
            }
        }

        auto d_elem_str_sizes { d_elem_strs | std::views::transform([](const auto& s){ return s.size(); }) };
        auto r_elem_str_sizes { r_elem_strs | std::views::transform([](const auto& s){ return s.size(); }) };
        const auto max_size {
            std::max(
                *std::ranges::max_element(d_elem_str_sizes),
                *std::ranges::max_element(r_elem_str_sizes)
            )
        };
        std::string d_final_str {};
        for (const auto& s: d_elem_strs) {
            d_final_str += s + std::string(s.size() - max_size, padding) + separator;
        }
        d_final_str.erase(d_final_str.size() - separator.size());  // remove trailing separator
        std::string r_final_str {};
        for (const auto& s: r_elem_strs) {
            r_final_str += s + std::string(s.size() - max_size, padding) + separator;
        }
        r_final_str.erase(r_final_str.size() - separator.size());  // remove trailing separator

        return d_final_str + '\n' + r_final_str;
    }
}

#endif //OFFBYNULL_ALIGNER_ALIGNERS_UTILS_H