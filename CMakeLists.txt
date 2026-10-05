cmake_minimum_required(VERSION 3.20)
project(TokenBucket VERSION 1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
set(TOKENBUCKET_TOP_LEVEL OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(TOKENBUCKET_TOP_LEVEL ON)
endif()
option(TOKENBUCKET_BUILD_TESTS "Build TokenBucket tests" ${TOKENBUCKET_TOP_LEVEL})
option(TOKENBUCKET_INSTALL "Generate TokenBucket installation rules" ${TOKENBUCKET_TOP_LEVEL})
option(TOKENBUCKET_WARNINGS_AS_ERRORS "Treat test warnings as errors" OFF)

add_library(TokenBucket INTERFACE)
add_library(TokenBucket::TokenBucket ALIAS TokenBucket)
target_compile_features(TokenBucket INTERFACE cxx_std_11)
target_include_directories(TokenBucket INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

if(TOKENBUCKET_BUILD_TESTS)
  enable_testing()
  add_executable(TokenBucketTest tests/test.cpp)
  target_link_libraries(TokenBucketTest PRIVATE TokenBucket)
  set_target_properties(TokenBucketTest PROPERTIES CXX_EXTENSIONS OFF)
  if(MSVC)
    target_compile_options(TokenBucketTest PRIVATE /W4 /permissive-)
    if(TOKENBUCKET_WARNINGS_AS_ERRORS)
      target_compile_options(TokenBucketTest PRIVATE /WX)
    endif()
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(TokenBucketTest PRIVATE -Wall -Wextra -Wpedantic)
    if(TOKENBUCKET_WARNINGS_AS_ERRORS)
      target_compile_options(TokenBucketTest PRIVATE -Werror)
    endif()
  endif()
  add_test(NAME TokenBucketTest COMMAND TokenBucketTest)
  set_tests_properties(TokenBucketTest PROPERTIES TIMEOUT 10)
endif()

configure_package_config_file(cmake/TokenBucketConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/TokenBucketConfig.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/TokenBucket")
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/TokenBucketConfigVersion.cmake"
  VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion)
export(TARGETS TokenBucket NAMESPACE TokenBucket::
  FILE "${CMAKE_CURRENT_BINARY_DIR}/TokenBucketTargets.cmake")

if(TOKENBUCKET_INSTALL)
  install(FILES include/rigtorp/TokenBucket.h
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/rigtorp")
  install(TARGETS TokenBucket EXPORT TokenBucketTargets)
  install(EXPORT TokenBucketTargets NAMESPACE TokenBucket::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/TokenBucket")
  install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/TokenBucketConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/TokenBucketConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/TokenBucket")
endif()
