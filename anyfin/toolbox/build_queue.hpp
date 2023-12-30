
#pragma once

#include "anyfin/core/allocator.hpp"
#include "anyfin/core/arena.hpp"
#include "anyfin/core/atomics.hpp"
#include "anyfin/core/closure.hpp"
#include "anyfin/core/slice.hpp"

#include "anyfin/platform/threads.hpp"
#include "anyfin/platform/concurrent.hpp"

namespace Fin::Toolbox {

template <typename T, Core::Basic_Allocator Allocator>
class Task_Queue {
  using Task_Handler = Core::Closure<void (T &)>;

  struct Node {
    static_assert(sizeof(T) <= 64 - sizeof(as32));

    T    task;
    as32 sequence_number;

    char cache_line_pad[64 - sizeof(T) - sizeof(as32)];
  };

  static_assert(sizeof(Node) == 64);

  Allocator &allocator;

  Core::Slice<Platform::Thread> builders;

  Core::Slice<Node>   tasks_queue;
  Platform::Semaphore tasks_available;

  cas64 write_index = 0;
  cas64 read_index  = 0;

  cau32 tasks_submitted = 0;
  cau32 tasks_completed = 0;

  abool terminating = false;

  Task_Queue (Allocator &_allocator, Core::Slice<Platform::Thread> _builders,
              Core::Slice<Node> nodes, Platform::Semaphore semaphore,
              const Core::Closure<void (T &)> &handler)
    : allocator { _allocator }, builders { _builders }, tasks_queue { nodes }, tasks_available { semaphore }
  {
    for (auto &builder: builders) {
      builder = *spawn_thread([this, &handler] () {
        while (true) {
          wait_for_semaphore_signal(this->tasks_available);
          if (atomic_load<Core::Memory_Order::Acquire>(this->terminating)) break;
          pull_task_for_execution(this)
            .handle_value([&] (auto task) {
              handler(task);
              atomic_fetch_add(this->tasks_completed, 1);
            });
        }
      });
    }
  }

public:

  ~Task_Queue () {
    
  }

  bool submit_task (T &&task);

  bool has_unfinished_tasks () {
    auto submitted = atomic_load(this->tasks_submitted);
    auto completed = atomic_load(this->tasks_completed);

    return (submitted != completed);
  }

};
  
/*
  Create a queue's instance.
  This function may complete with an error result if any of the underlying system calls fail.
*/
template <typename T, Core::Basic_Allocator Allocator>
static Core::Result<Platform::System_Error, Task_Queue<T, Allocator>> create_task_queue (
  Allocator &allocator,

  /*
    Size of the queue.
    This value must be a power of 2, if not, it will be aligned forward to the next closes value.
  */
  const usize queue_size,

  /*
    Number of builders that will be waiting on tasks and executing them.
    It's allowed for this value to be 0, meaning that it's the main thread that should handle tasks from the queue.
  */
  const usize builders_count,

  /*
    Processing function that each builder will run on each queued task.
  */
  const Core::Closure<void (T &)> &func)
{
  using Node = typename Task_Queue<T, Allocator>::Node;
    
  auto aligned_size = align_forward_to_pow_2(queue_size);

  Platform::Semaphore semaphore;
  {
    auto result = create_semaphore();
    if (!result) return result.as();
    semaphore = result.take();
  }

  auto nodes = reserve_array<Node>(allocator, aligned_size);
  for (u32 idx = 0; auto &node: nodes) node.sequence_number = idx;

  auto builders = Slice(reserve_array<Platform::Thread>(allocator, builders_count), builders_count);

  return Core::Ok(Task_Queue(allocator, builders, nodes, func));
}

}

