
#pragma once

#include "anyfin/core/atomics.hpp"
#include "anyfin/platform/platform.hpp"

namespace Fin::Platform {

struct Spin_Lock {
  enum struct Status: u64 { Available = 0, Locked = 1 };

  Core::Atomic<Status> _lock { Status::Available };

  Spin_Lock () = default;

  void lock () {
    using enum Status;
    using enum Core::Memory_Order;

    while (atomic_compare_and_set<Acquire_Release, Acquire>(this->_lock, Available, Locked)) {}
  }

  void unlock () {
    atomic_store<Core::Memory_Order::Release>(this->_lock, Status::Available);
  }
};

struct Semaphore {
  struct Handle;
  Handle *handle;
};

Result<Semaphore> create_semaphore (u32 count = static_cast<u32>(-1));
Result<void> destroy_semaphore (Semaphore &semaphore);

Result<u32> increment_semaphore (Semaphore &semaphore, u32 increment_value = 1);

Result<void> wait_for_semaphore_signal (const Semaphore &sempahore);

}
