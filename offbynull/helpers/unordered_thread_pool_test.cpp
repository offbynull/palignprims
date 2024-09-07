#include <thread>
#include <iostream>
#include <ostream>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <future>
#include <vector>
#include "offbynull/helpers/unordered_thread_pool.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::helpers::unordered_thread_pool::unordered_thread_pool;
    using offbynull::utils::is_debug_mode;

    TEST(OHUnorderedThreadPoolTest, SanityTest) {
        std::atomic_int64_t cntr {};
        auto task {
            [&cntr](unordered_thread_pool<true, std::int64_t>& /*owner*/) -> std::int64_t {
                std::cout << std::this_thread::get_id() << "_t" << std::endl;
                return ++cntr;
            }
        };

        unordered_thread_pool<true, std::int64_t> pool { 4zu };
        std::vector<std::future<std::int64_t>> futures;
        for (std::size_t i { 0zu }; i < 100zu; ++i) {
            futures.push_back(
                *pool.queue(task)
            );
        }
        for (auto&& future : futures) {
            std::cout << future.get() << std::endl;
        }
        pool.close();
    }
}