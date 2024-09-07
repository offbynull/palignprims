#ifndef OFFBYNULL_HELPERS_UNORDERED_THREAD_POOL_H
#define OFFBYNULL_HELPERS_UNORDERED_THREAD_POOL_H

#include <cstddef>
#include <unordered_map>
#include <deque>
#include <thread>
#include <future>
#include <optional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <utility>
#include <random>
#include <concepts>
#include <exception>
#include <stdexcept>
#include "offbynull/concepts.h"

namespace offbynull::helpers::unordered_thread_pool {
    template<bool debug_mode, typename TASK_RESULT>
    class unordered_thread_pool;

    template<bool debug_mode, typename TASK_RESULT>
    class worker;

    template<bool debug_mode, typename TASK_RESULT>
    struct concurrency_block {
        std::mutex mutex;
        std::condition_variable signal;
        std::deque<std::packaged_task<TASK_RESULT(unordered_thread_pool<debug_mode, TASK_RESULT>&)>> queue;
        std::jthread thread;

        concurrency_block(unordered_thread_pool<debug_mode, TASK_RESULT>& owner)
        : mutex {}
        , signal {}
        , queue {}
        , thread { worker<debug_mode, TASK_RESULT> { owner, *this } } {}
    };

    template<bool debug_mode, typename TASK_RESULT>
    class unordered_thread_pool {
    private:
        const std::size_t concurrency;
        std::unordered_map<std::size_t, concurrency_block<debug_mode, TASK_RESULT>> blocks;  // can't use vector - block class not
                                                                                             // copyable/movable (due to mutex/cv)
        std::atomic_flag closed;

        static thread_local std::mt19937_64 rand;

    public:
        unordered_thread_pool()
        : unordered_thread_pool { std::thread::hardware_concurrency() } {}

        unordered_thread_pool(std::size_t concurrency_)
        : concurrency { concurrency_ }
        , blocks {}
        , closed { false } {
            if constexpr (debug_mode) {
                if (concurrency == 0zu) {
                    throw std::runtime_error { "Concurrency must be > 0" };
                }
            }
            for (std::size_t i { 0zu }; i < concurrency; ++i) {
                blocks.emplace(i, *this);
            }
        }

        unordered_thread_pool(const unordered_thread_pool&) = delete;
        unordered_thread_pool(unordered_thread_pool&&) = delete;
        unordered_thread_pool& operator=(const unordered_thread_pool&) = delete;
        unordered_thread_pool& operator=(unordered_thread_pool&&) = delete;

        ~unordered_thread_pool() noexcept {
            close();
        }

        template<typename TASK>
        requires requires(TASK t, unordered_thread_pool<debug_mode, TASK_RESULT>& owner) {
            { t(owner) } -> std::convertible_to<TASK_RESULT>;
        }
        std::optional<std::future<TASK_RESULT>> queue(TASK&& task) {
            if (closed.test(std::memory_order_acquire)) {
                return { std::nullopt };
            }

            std::packaged_task<TASK_RESULT(unordered_thread_pool&)> packaged_task { std::forward<TASK>(task) };
            std::future<TASK_RESULT> packaged_task_future { packaged_task.get_future() };

            std::size_t idx { std::uniform_int_distribution<std::size_t> { 0zu, concurrency - 1zu } (rand) };
            concurrency_block<debug_mode, TASK_RESULT>& block { (*blocks.find(idx)).second };
            {
                std::unique_lock lock { block.mutex };
                block.queue.push_back(std::move(packaged_task));
            }
            block.signal.notify_one();

            // Why is this checking for a closed pool again? Imagine that, as this function is in the middle of exeucting, the pool goes on
            // to be closed. The function has already executed past the first closed pool check above, meaning that the task is being added
            // to the queue. But, before the task actually gets added to the queue, the thread responsible for executing tasks in the queue
            // is shut down. As such, the task being added will never get run (and as such calling get() on the future returned for this
            // task will never return).
            //
            // This 2nd check prevents the scenario mentioned above. If the pool gets closed half-way through the execution of this
            // function, the task is removed and no future gets returned.
            if (closed.test(std::memory_order_acquire)) {
                {
                    std::unique_lock lock { block.mutex };
                    // If task that was just added hasn't already been consumed by the worker thread, remove it.
                    if (!block.queue.empty()) {
                        block.queue.pop_back();
                    }
                }
                return { std::nullopt };
            }

            return { std::move(packaged_task_future) };
        }

        bool is_closed() const {
            return closed.test(std::memory_order_acquire);
        }

        void close() {
            closed.test_and_set(std::memory_order_release);
            for (auto& [_, block] : blocks) {
                block.signal.notify_all();
            }
        }
    };

    template<bool debug_mode, typename TASK_RESULT>
    thread_local std::mt19937_64 unordered_thread_pool<debug_mode, TASK_RESULT>::rand {
        std::hash<std::thread::id>{} (std::this_thread::get_id())
    };

    template<bool debug_mode, typename TASK_RESULT>
    class worker {
    private:
        unordered_thread_pool<debug_mode, TASK_RESULT>& owner;
        concurrency_block<debug_mode, TASK_RESULT>& block;

    public:
        worker(
            unordered_thread_pool<debug_mode, TASK_RESULT>& owner_,
            concurrency_block<debug_mode, TASK_RESULT>& block_
        )
        : owner { owner_ }
        , block { block_ } {}

        void operator()() const {
            while (true) {
                std::packaged_task<TASK_RESULT(unordered_thread_pool<debug_mode, TASK_RESULT>&)> task;
                {
                    std::unique_lock lock { block.mutex };
                    block.signal.wait(lock, [&]{ return !block.queue.empty() || owner.is_closed(); }); // wait until something in queue
                    if (owner.is_closed()) {
                        break;
                    }
                    if (block.queue.empty()) {
                        continue;
                    }
                    task = std::move(block.queue.front());
                    block.queue.pop_front();
                }
                try {
                    task(owner);
                } catch (const std::exception& e) {
                    // Do nothing
                }
            }
        }
    };
}

#endif //OFFBYNULL_HELPERS_UNORDERED_THREAD_POOL_H
