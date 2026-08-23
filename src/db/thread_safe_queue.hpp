
#pragma once

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <optional>
#include <queue>
#include <type_traits>
#include <utility>

namespace Helpers {

/**
 * \brief потокобезопасная очередь с поддержкой корректного завершения (stop).
 *
 * обеспечивает безопасную передачу данных между потоками по паттерну
 * Producer-Consumer. поддерживает как блокирующее извлечение (wait_and_pop),
 * так и неблокирующее (try_pop).
 *
 * \tparam T тип элементов очереди.
 * \note все публичные методы являются потокобезопасными и не выбрасывают
 * исключений.
 */
template <typename T>
class ThreadSafeQueue {
 private:
  std::queue<T> queue_;
  mutable std::mutex mutex_;
  std::condition_variable cv_;
  bool is_working_ = true;

 public:
  ThreadSafeQueue() = default;
  ~ThreadSafeQueue() { stop(); }

  ThreadSafeQueue(const ThreadSafeQueue&) = delete;
  ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;
  ThreadSafeQueue(ThreadSafeQueue&&) = delete;
  ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;
  /**
   * \brief переводит очередь в состояние остановки и пробуждает ожидающие
   * потоки.
   *
   * \post после вызова stop() новые элементы не принимаются, а ожидающие
   * вызовы wait_and_pop() завершаются.
   */
  void stop() noexcept {
    {
      std::scoped_lock<std::mutex> lock{mutex_};
      is_working_ = false;
    }
    cv_.notify_all();
  }

  /**
   * \brief добавляет элемент в конец очереди.
   *
   * \tparam U универсальный тип добавляемого элемента.
   * \return true при успешном добавлении, false если очередь остановлена.
   */
  template <typename U>
  bool push(U&& value) noexcept {
    static_assert(std::is_constructible_v<T, U>,
                  "U must be constructible to T");
    static_assert(std::is_move_constructible_v<T>,
                  "ThreadSafeQueue elements must be move constructible");
    try {
      {
        std::scoped_lock<std::mutex> lock{mutex_};
        if (!is_working_) {
          return false;
        }
        queue_.emplace(std::forward<U>(value));
      }
      cv_.notify_one();
      return true;
    } catch (...) {
      return false;
    }
  }
  /**
   * \brief блокирующее извлечение элемента из головы очереди.
   *
   * приостанавливает вызывающий поток до появления хотя бы одного элемента
   * либо до вызова метода stop().
   *
   * \return std::optional<T> с элементом или std::nullopt.
   */
  std::optional<T> wait_and_pop() noexcept {
    try {
      std::unique_lock<std::mutex> lock{mutex_};
      cv_.wait(lock, [this]() { return !queue_.empty() || !is_working_; });

      if (!is_working_ && queue_.empty()) {
        return std::nullopt;
      }
      auto value = std::move(queue_.front());
      queue_.pop();
      return value;
    } catch (...) {
      return std::nullopt;
    }
  }
  /**
   * \brief неблокирующая попытка извлечения элемента из очереди.
   *
   * \return std::optional с элементом типа T, либо std::nullopt, если очередь
   * пуста.
   */
  std::optional<T> try_pop() noexcept {
    try {
      std::scoped_lock<std::mutex> lock{mutex_};
      if (queue_.empty()) {
        return std::nullopt;
      }
      auto value = std::move(queue_.front());
      queue_.pop();
      return value;
    } catch (...) {
      return std::nullopt;
    }
  }

  bool empty() const noexcept {
    std::scoped_lock<std::mutex> lock{mutex_};
    return queue_.empty();
  }

  std::size_t size() const noexcept {
    std::scoped_lock<std::mutex> lock{mutex_};
    return queue_.size();
  }

  void clear() noexcept {
    std::scoped_lock<std::mutex> lock{mutex_};
    queue_ = {};
  }
};

}  // namespace Helpers
