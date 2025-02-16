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

/**
 * Unordered thread pool.
 */
namespace offbynull::helpers::unordered_thread_pool {
    template<bool debug_mode, typename TASK_RESULT>
    class unordered_thread_pool;

    template<bool debug_mode, typename TASK_RESULT>
    class worker;

    /**
     * Variables for a single thread within @ref offbynull::helpers::unordered_thread_pool::unordered_thread_pool instance.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam TASK_RESULT Return type of tasks processed by this thread.
     */
    template<bool debug_mode, typename TASK_RESULT>
    struct concurrency_block {
        /** Mutex used to synchronize access to concurrently accessed objects. */
        std::mutex mutex;
        /** Signal used to wake `thread`. */
        std::condition_variable signal;
        /** Queue of tasks to be run by `thread`. */
        std::deque<std::packaged_task<TASK_RESULT(unordered_thread_pool<debug_mode, TASK_RESULT>&)>> queue;
        /** Thread. */
        std::jthread thread;

        /**
         * Construct an @ref offbynull::helpers::unordered_thread_pool::concurrency_block instance.
         *
         * @param owner Thread pool that spawned / owns this object.
         */
        concurrency_block(unordered_thread_pool<debug_mode, TASK_RESULT>& owner)
        : mutex {}
        , signal {}
        , queue {}
        , thread { worker<debug_mode, TASK_RESULT> { owner, *this } } {}
    };

    /**
     * Thread pool where tasks potentially start out-of-order (when compared to the order that those tasks were submitted).
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam TASK_RESULT Return type of tasks processed by this thread pool.
     */
    template<bool debug_mode, typename TASK_RESULT>
    class unordered_thread_pool {
    private:
        const std::size_t concurrency;
        std::unordered_map<std::size_t, concurrency_block<debug_mode, TASK_RESULT>> blocks;  // Can't use vector - block class not
                                                                                             // copyable/movable (due to mutex/cv)
        std::atomic_flag closed;

        static thread_local std::mt19937_64 rand;

    public:
        /**
         * Equivalent to invoking `unordered_thread_pool { std::thread::hardware_concurrency() }`.
         */
        unordered_thread_pool()
        : unordered_thread_pool { std::thread::hardware_concurrency() } {}

        /**
         * Construct an @ref offbynull::helpers::unordered_thread_pool::unordered_thread_pool instance.
         *
         * @param concurrency_ Number of threads to launch.
         * @throws std::runtime_error If `debug_mode == true && concurrency_ == 0zu`.
         */
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

        // Deletions required because concurrency_block isn't copyable/moveable (due to mutex/cv)?
        unordered_thread_pool(const unordered_thread_pool&) = delete;
        unordered_thread_pool(unordered_thread_pool&&) = delete;
        unordered_thread_pool& operator=(const unordered_thread_pool&) = delete;
        unordered_thread_pool& operator=(unordered_thread_pool&&) = delete;

        ~unordered_thread_pool() noexcept {
            close();
        }

        /**
         * Queue task to be executed by this thread pool.
         *
         * @tparam TASK Type of task to be executed.
         * @param task Task to be executed.
         * @return An optional holding an `std::future<TASK_RESULT>` which will contain the `task`'s result once it executes, or
         * `std::nullopt` if this thread pool has closed.
         */
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

        /**
         * Test if this thread pool has closed.
         *
         * @return `true` if this thread pool is closed, `false` otherwise.
         */
        bool is_closed() const {
            return closed.test(std::memory_order_acquire);
        }

        /**
         * Close this thread pool. Each thread within this pool is signalled such that it shuts down once the task it's currently running,
         * if any, is finished. Any remaining queued tasks are silently discarded.
         *
         * This function doesn't wait for threads to shut down before returning.
         */
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

    /**
     * Thread pool worker.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam TASK_RESULT Return type of tasks processed by this thread pool / worker.
     */
    template<bool debug_mode, typename TASK_RESULT>
    class worker {
    private:
        unordered_thread_pool<debug_mode, TASK_RESULT>& owner;
        concurrency_block<debug_mode, TASK_RESULT>& block;

    public:
        /**
         * Construct an @ref offbynull::helpers::unordered_thread_pool::worker instance.
         *
         * @param owner_ Owning thread pool.
         * @param block_ `owner_`'s concurrency block for this worker.
         */
        worker(
            unordered_thread_pool<debug_mode, TASK_RESULT>& owner_,
            concurrency_block<debug_mode, TASK_RESULT>& block_
        )
        : owner { owner_ }
        , block { block_ } {}

        /**
         * Worker loop. Loop picks out tasks from the concurrency block's queue and executes it until the owning thread pool is closed.
         */
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
