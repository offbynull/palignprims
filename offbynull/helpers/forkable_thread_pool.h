#ifndef OFFBYNULL_HELPERS_FORKABLE_THREAD_POOL_H
#define OFFBYNULL_HELPERS_FORKABLE_THREAD_POOL_H

#include <cstddef>
#include <vector>
#include <deque>
#include <thread>
#include <future>
#include <optional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <utility>
#include <concepts>
#include <exception>
#include <stdexcept>
#include <chrono>

namespace offbynull::helpers::forkable_thread_pool {
    template<bool debug_mode, typename TASK_RESULT>
    class worker;

    template<bool debug_mode, typename TASK_RESULT>
    class forkable_thread_pool {
    private:
        const std::size_t concurrency;
        std::mutex mutex;
        std::condition_variable signal;
        std::deque<std::packaged_task<TASK_RESULT(forkable_thread_pool<debug_mode, TASK_RESULT>&)>> queue_;
        std::vector<std::jthread> threads;
        std::atomic_flag closed;

    public:
        forkable_thread_pool()
        : forkable_thread_pool { std::thread::hardware_concurrency() } {}

        forkable_thread_pool(std::size_t concurrency_)
        : concurrency { concurrency_ }
        , mutex {}
        , signal {}
        , queue_ {}
        , threads {}
        , closed { false } {
            if constexpr (debug_mode) {
                if (concurrency == 0zu) {
                    throw std::runtime_error { "Concurrency must be > 0" };
                }
            }
            for (std::size_t i { 0zu }; i < concurrency; ++i) {
                threads.emplace_back(worker<debug_mode, TASK_RESULT> { *this });
            }
        }

        forkable_thread_pool(const forkable_thread_pool&) = delete;
        forkable_thread_pool(forkable_thread_pool&&) = delete;
        forkable_thread_pool& operator=(const forkable_thread_pool&) = delete;
        forkable_thread_pool& operator=(forkable_thread_pool&&) = delete;

        ~forkable_thread_pool() noexcept {
            close();
        }

        template<typename TASK>
        requires requires(TASK t, forkable_thread_pool<debug_mode, TASK_RESULT>& owner) {
            { t(owner) } -> std::convertible_to<TASK_RESULT>;
        }
        std::optional<std::future<TASK_RESULT>> queue(TASK&& task) {
            if (closed.test(std::memory_order_acquire)) {
                return { std::nullopt };
            }

            std::packaged_task<TASK_RESULT(forkable_thread_pool&)> packaged_task { std::forward<TASK>(task) };
            std::future<TASK_RESULT> packaged_task_future { packaged_task.get_future() };

            {
                std::unique_lock lock { mutex };
                queue_.push_back(std::move(packaged_task));
            }
            signal.notify_one();

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
                    std::unique_lock lock { mutex };
                    // If task that was just added hasn't already been consumed by the worker thread, remove it.
                    if (!queue_.empty()) {
                        queue_.pop_back();
                    }
                }
                return { std::nullopt };
            }

            return { std::move(packaged_task_future) };
        }

        std::future<TASK_RESULT>& join(std::future<TASK_RESULT>& future) {
            using namespace std::chrono_literals;
            while (true) {
                std::future_status status { future.wait_for(0s) };
                if (status == std::future_status::ready) {
                    return future;
                } else if (status == std::future_status::timeout) {
                    // do nothing
                } else {
                    throw std::runtime_error { "Unexpected future state" };
                }

                std::packaged_task<TASK_RESULT(forkable_thread_pool<debug_mode, TASK_RESULT>&)> task;
                {
                    std::unique_lock lock { mutex };
                    signal.wait(lock, [&]{ return !queue_.empty() || future.wait_for(0s) == std::future_status::ready || is_closed(); });
                    // Because this a join() call, continue processing even if closed
                    // if (owner.is_closed()) {
                    //    break;
                    // }
                    if (queue_.empty()) {
                        continue;
                    }
                    task = std::move(queue_.front());
                    queue_.pop_front();
                }
                try {
                    task(*this);
                } catch (const std::exception& e) {
                    // Do nothing
                }
                signal.notify_all(); // Something may be waiting on this task -- notify all so the waiting thread can move forward
            }
        }

        bool is_closed() const {
            return closed.test(std::memory_order_acquire);
        }

        void close() {
            closed.test_and_set(std::memory_order_release);
            signal.notify_all();
        }

        friend class worker<debug_mode, TASK_RESULT>;
    };

    template<bool debug_mode, typename TASK_RESULT>
    class worker {
    private:
        forkable_thread_pool<debug_mode, TASK_RESULT>& owner;

    public:
        worker(
            forkable_thread_pool<debug_mode, TASK_RESULT>& owner_
        )
        : owner { owner_ } {}

        void operator()() {
            while (true) {
                std::packaged_task<TASK_RESULT(forkable_thread_pool<debug_mode, TASK_RESULT>&)> task;
                {
                    std::unique_lock lock { owner.mutex };
                    owner.signal.wait(lock, [&]{ return !owner.queue_.empty() || owner.is_closed(); }); // wait until something in queue
                    if (owner.is_closed()) {
                        break;
                    }
                    if (owner.queue_.empty()) {
                        continue;
                    }
                    task = std::move(owner.queue_.front());
                    owner.queue_.pop_front();
                }
                try {
                    task(owner);
                } catch (const std::exception& e) {
                    // Do nothing
                }
                owner.signal.notify_all(); // Something may be waiting on this task -- notify all so the waiting thread can move forward
            }
        }
    };
}

#endif //OFFBYNULL_HELPERS_FORKABLE_THREAD_POOL_H
