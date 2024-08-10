#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_UTILS_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_UTILS_H

#include <concepts>
#include <limits>
#include <cstdint>
#include <boost/safe_numerics/safe_integer.hpp>

// USE ME SOMEWHERE?
namespace offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::utils {
    using namespace boost::safe_numerics;

    consteval std::unsigned_integral auto narrowest_possible_slot_index_type(
        std::unsigned_integral auto grid_down_type,
        std::unsigned_integral auto grid_right_type,
        std::unsigned_integral auto grid_depth_type
    ) {
        constexpr safe<std::uintmax_t> grid_down_max { std::numeric_limits<decltype(grid_down_type)>::max() };
        constexpr safe<std::uintmax_t> grid_right_max { std::numeric_limits<decltype(grid_right_type)>::max() };
        constexpr safe<std::uintmax_t> grid_depth_max { std::numeric_limits<decltype(grid_depth_type)>::max() };
        // try {
        //     constexpr safe<std::uintmax_t> max_slots { grid_down_max * grid_right_max * grid_depth_max };
        // } catch (const boost::safe_numerics::safe_numerics_error& e) {
        //     return std::uintmax_t {};
        // }
        constexpr safe<std::uintmax_t> max_slots { grid_down_max * grid_right_max * grid_depth_max };
        if constexpr (max_slots <= std::numeric_limits<uint8_t>::max()) {
            return std::uint8_t {};
        } else if constexpr (max_slots <= std::numeric_limits<uint16_t>::max()) {
            return std::uint16_t {};
        } else if constexpr (max_slots <= std::numeric_limits<uint32_t>::max()) {
            return std::uint32_t {};
        } else if constexpr (max_slots <= std::numeric_limits<uint64_t>::max()) {
            return std::uint64_t {};
        } else {
            return std::uintmax_t {};
        }
    }
}
#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_UTILS_H
