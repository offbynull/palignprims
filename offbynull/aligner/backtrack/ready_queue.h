#ifndef OFFBYNULL_ALIGNER_BACKTRACK_READY_QUEUE_H
#define OFFBYNULL_ALIGNER_BACKTRACK_READY_QUEUE_H

#include <cstddef>
#include <stdexcept>
#include "offbynull/aligner/backtrack/container_creator.h"
#include "offbynull/aligner/backtrack/container_creators.h"

namespace offbynull::aligner::backtrack::ready_queue {
    using offbynull::aligner::backtrack::container_creator::container_creator;
    using offbynull::aligner::backtrack::container_creators::vector_container_creator;

    template<
        container_creator ALLOCATOR=vector_container_creator<std::size_t>,
        bool error_check=true
    >
    class ready_queue {
    private:
        decltype(std::declval<ALLOCATOR>().create_empty(std::nullopt)) queue;

    public:
        ready_queue(
            ALLOCATOR container_creator = {}
        ) : queue{container_creator.create_empty(std::nullopt)} {
            if constexpr (error_check) {
                if (!queue.empty()) {
                    throw std::runtime_error("Queue must be sized 0 on creation");  // Happens on std::array sized > 0
                }
            }
        }

        bool empty() {
            return queue.empty();
        }

        void push(std::size_t idx) {
            queue.push_back(idx);
        }

        std::size_t pop() {
            auto ret { queue.back() };
            queue.pop_back();
            return ret;
        }
    };
}
#endif //OFFBYNULL_ALIGNER_BACKTRACK_READY_QUEUE_H
