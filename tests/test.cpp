// © 2023 Erik Rigtorp <erik@rigtorp.se>
// SPDX-License-Identifier: MIT

#include <rigtorp/TokenBucket.h>
#include <cstdlib>
#include <iostream>

struct TestClock {
  using rep = std::chrono::nanoseconds::rep;
  using period = std::chrono::nanoseconds::period;
  using duration = std::chrono::nanoseconds;
  using time_point = std::chrono::time_point<TestClock>;
  static constexpr bool is_steady = true;
  static time_point current;
  static time_point now() noexcept { return current; }
};

TestClock::time_point TestClock::current{};

static void check(bool success, const char *description) {
  if (!success) {
    std::cerr << "Failed: " << description << '\n';
    std::exit(EXIT_FAILURE);
  }
}

int main() {
  using Bucket = rigtorp::TokenBucket<TestClock>;
  using Nanoseconds = std::chrono::nanoseconds;
  const Nanoseconds tokenTime = Nanoseconds(std::chrono::seconds(1)) / 350;

  // Startup before one full burst interval has elapsed must still be full.
  TestClock::current = TestClock::time_point{};
  Bucket bucket(350, 1050);
  check(bucket.consume(1050), "initial full burst at clock epoch");
  check(!bucket.consume(1), "empty bucket");
  check(bucket.consume(0), "zero-token request");
  TestClock::current += tokenTime - Nanoseconds(1);
  check(!bucket.consume(1), "no token before refill boundary");
  TestClock::current += Nanoseconds(1);
  check(bucket.consume(1), "one token at refill boundary");
  check(!bucket.consume(1), "refill token consumed once");
  TestClock::current += tokenTime * 350;
  check(bucket.consume(350), "350 tokens after exact refill interval");
  check(!bucket.consume(1), "exact refill exhausted");

  TestClock::current += std::chrono::seconds(10);
  check(!bucket.consume(1051), "refill capped at burst size");
  check(bucket.consume(1050), "rejected request leaves full burst intact");
  check(!bucket.consume(1), "full burst exhausted");

  TestClock::current = TestClock::time_point(std::chrono::seconds(100));
  Bucket later(350, 1050);
  check(!later.consume(1051), "oversized initial request");
  check(later.consume(1050), "initial rejection leaves state intact");
  check(!later.consume(1), "later initial burst exhausted");
  return EXIT_SUCCESS;
}
