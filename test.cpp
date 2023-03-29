// © 2023 Erik Rigtorp <erik@rigtorp.se>
// SPDX-License-Identifier: MIT

#include "TokenBucket.h"
#include <cassert>
#include <iostream>
#include <unistd.h>

int main(int arv, char *argv[]) {

  {
    TokenBucket tb(350, 1050);

    assert(tb.consume(1050));
    assert(!tb.consume(1));
    sleep(1);
    assert(tb.consume(350));
    assert(!tb.consume(1));
  }

  {
    TokenBucket tb(350, 1050);

    assert(!tb.consume(1051));
  }

  return 0;
}