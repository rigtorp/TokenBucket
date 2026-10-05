# TokenBucket.h

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/rigtorp/TokenBucket/master/LICENSE)

Lock-free implementation of the
[token bucket](https://en.wikipedia.org/wiki/Token_bucket) algorithm
in C++11.

## Usage

```cpp
#include <rigtorp/TokenBucket.h>

rigtorp::TokenBucket<> bucket(350, 1050); // 350 tokens/s, burst of 1050
if (bucket.consume(1)) {
  // Perform rate-limited work.
}
```

The header now lives in `include/rigtorp/TokenBucket.h` and the class is in
namespace `rigtorp`. Update previous `#include "TokenBucket.h"` directives and
global `TokenBucket<>` uses to the forms above. For a manual build, add `include/`
to the compiler's header search path. C++11 code must include the template
arguments (`<>` selects `std::chrono::steady_clock`).

## CMake

CMake 3.20 or newer is required. For an embedded checkout:

```cmake
add_subdirectory(path/to/TokenBucket)
target_link_libraries(your_target PRIVATE TokenBucket::TokenBucket)
```

To build, test, and install:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
cmake --install build --config Release --prefix /path/to/install
```

An installed package, or the configured build directory, can be consumed with:

```cmake
find_package(TokenBucket 1 REQUIRED CONFIG)
target_link_libraries(your_target PRIVATE TokenBucket::TokenBucket)
```

Set `CMAKE_PREFIX_PATH` to the installation prefix, or `TokenBucket_DIR` to the
configured build directory. Installed packages support relocation and custom
`CMAKE_INSTALL_INCLUDEDIR` and `CMAKE_INSTALL_LIBDIR` values.

`TOKENBUCKET_BUILD_TESTS` and `TOKENBUCKET_INSTALL` default to `ON` for standalone
builds and `OFF` when embedded; each can be overridden. Test checks run in both
Debug and Release with a controllable clock and no sleeps. Enable
`TOKENBUCKET_WARNINGS_AS_ERRORS` to treat test compiler warnings as errors.
Run `python tests/check_cmake.py` to check package consumers and relocation.

## About

This project was created by [Erik Rigtorp](http://rigtorp.se)
<[erik@rigtorp.se](mailto:erik@rigtorp.se)>.
