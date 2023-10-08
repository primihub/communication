/*
 * Copyright (c) 2023 by PrimiHub
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      https://www.apache.org/licenses/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef THREADSAFE_QUEUE_H_
#define THREADSAFE_QUEUE_H_

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

namespace primihub::link {
template <typename T> class ThreadSafeQueue {
public:
  void push(const T &item) { emplace(item); }

  void push(T &&item) { emplace(std::move(item)); }

  template <typename... Args> void emplace(Args &&...args) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_queue.emplace(std::forward<Args>(args)...);
    lock.unlock();
    m_cv.notify_one();
  }

  bool empty() const {
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_queue.empty();
  }

  bool try_pop(T &popped_value) {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_queue.empty()) {
      return false;
    }

    popped_value = std::move(m_queue.front());
    m_queue.pop();
    return true;
  }

  void wait_and_pop(T &popped_value) {
    std::unique_lock<std::mutex> lock(m_mutex);
    // while (m_queue.empty()) {
    //   m_cv.wait(lock);
    // }
    m_cv.wait(lock, [&]() { return stop_.load() || !m_queue.empty(); });
    if (stop_.load()) {
      return;
    }
    popped_value = std::move(m_queue.front());
    m_queue.pop();
  }

  // Provides only basic exception safety guarantee when RVO is not applied.
  T pop() {
    std::unique_lock<std::mutex> lock(m_mutex);
    // while (m_queue.empty()) {
    //   m_cv.wait(lock);
    // }
    m_cv.wait(lock, [&]() { return stop_.load() || !m_queue.empty(); });
    if (stop_.load()) {
      return T();
    }
    auto item = std::move(m_queue.front());
    m_queue.pop();
    return item;
  }

  void shutdown() {
    stop_.store(true);
    m_cv.notify_one();
  }

private:
  std::queue<T> m_queue;
  mutable std::mutex m_mutex;
  std::condition_variable m_cv;
  std::atomic<bool> stop_{false};
};
} // namespace primihub::link

#endif
