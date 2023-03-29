// © 2023 Erik Rigtorp <erik@rigtorp.se>
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>
#include <chrono>

template <typename Clock = std::chrono::steady_clock> class TokenBucket {
public:
  TokenBucket() = default;

  TokenBucket(const uint64_t rate, const uint64_t burstSize)
      : timePerToken_(std::chrono::nanoseconds(std::chrono::seconds(1)) / rate),
        timePerBurst_(burstSize * timePerToken_) {}

  bool consume(const uint64_t tokens) {
    const auto now = Clock::now();
    const auto timeNeeded = tokens * timePerToken_;
    const auto minTime = now - timePerBurst_;
    auto oldTime = time_.load(std::memory_order_relaxed);

    for (;;) {
      auto newTime = oldTime;
      if (minTime > newTime) {
        newTime = minTime;
      }
      newTime += timeNeeded;
      if (newTime > now) {
        return false;
      }
      if (time_.compare_exchange_weak(oldTime, newTime,
                                      std::memory_order_relaxed,
                                      std::memory_order_relaxed)) {
        return true;
      }
    }

    return false;
  }

private:
  std::atomic<typename Clock::time_point> time_ = {Clock::time_point::min()};
  std::chrono::nanoseconds timePerToken_;
  std::chrono::nanoseconds timePerBurst_;
};
