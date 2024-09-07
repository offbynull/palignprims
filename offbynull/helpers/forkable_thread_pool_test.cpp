#include <iostream>
#include <ostream>
#include <cstdint>
#include "offbynull/helpers/forkable_thread_pool.h"
#include "offbynull/utils.h"
#include "gtest/gtest.h"

namespace {
    using offbynull::helpers::forkable_thread_pool::forkable_thread_pool;
    using offbynull::utils::is_debug_mode;

    TEST(OHForkableThreadPoolTest, ForkJoinTest) {
        struct reducer {
            const std::int64_t num {};
            std::int64_t operator()(forkable_thread_pool<is_debug_mode(), std::int64_t>& owner) const {
                // std::cout << num << std::endl;
                if (num == 1) {
                    return 1;
                }
                auto fut1 { *owner.queue(reducer { num - 1 }) };
                auto fut2 { *owner.queue(reducer { num - 1 }) };
                return owner.join(fut1).get() + owner.join(fut2).get();
            }
        };

        forkable_thread_pool<is_debug_mode(), std::int64_t> pool { 4zu };
        std::int64_t final_num { (*pool.queue(reducer {10})).get() };
        std::cout << final_num << std::endl;
        pool.close();
        EXPECT_EQ(512, final_num);
    }
}