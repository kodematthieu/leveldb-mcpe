# Copyright 2017 The LevelDB Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file. See the AUTHORS file for names of contributors.

cmake_minimum_required(VERSION 3.9)
# Keep the version below in sync with the one in db.h
project(leveldb VERSION 1.20.0 LANGUAGES C CXX)

# C standard can be overridden when this is used as a sub-project.
if(NOT CMAKE_C_STANDARD)
  # This project can use C11, but will gracefully decay down to C89.
  set(CMAKE_C_STANDARD 11)
  set(CMAKE_C_STANDARD_REQUIRED OFF)
  set(CMAKE_C_EXTENSIONS OFF)
endif(NOT CMAKE_C_STANDARD)

# C++ standard can be overridden when this is used as a sub-project.
if(NOT CMAKE_CXX_STANDARD)
  # This project requires C++11.
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif(NOT CMAKE_CXX_STANDARD)

if (WIN32)
  set(LEVELDB_PLATFORM_NAME LEVELDB_PLATFORM_WINDOWS)
  # TODO(cmumford): Make UNICODE configurable for Windows.
  add_definitions(-D_UNICODE -DUNICODE)
else (WIN32)
  set(LEVELDB_PLATFORM_NAME LEVELDB_PLATFORM_POSIX)
endif (WIN32)

option(LEVELDB_BUILD_TESTS "Build LevelDB's unit tests" ON)
option(LEVELDB_BUILD_BENCHMARKS "Build LevelDB's benchmarks" ON)
option(LEVELDB_INSTALL "Install LevelDB's header and library" ON)

include(TestBigEndian)
test_big_endian(LEVELDB_IS_BIG_ENDIAN)

include(CheckIncludeFile)
check_include_file("unistd.h" HAVE_UNISTD_H)

include(CheckLibraryExists)
check_library_exists(crc32c crc32c_value "" HAVE_CRC32C)
check_library_exists(snappy snappy_compress "" HAVE_SNAPPY)
check_library_exists(tcmalloc malloc "" HAVE_TCMALLOC)

include(CheckCXXSymbolExists)
# Using check_cxx_symbol_exists() instead of check_c_symbol_exists() because
# we're including the header from C++, and feature detection should use the same
# compiler language that the project will use later. Principles aside, some
# versions of do not expose fdatasync() in <unistd.h> in standard C mode
# (-std=c11), but do expose the function in standard C++ mode (-std=c++11).
check_cxx_symbol_exists(fdatasync "unistd.h" HAVE_FDATASYNC)
check_cxx_symbol_exists(F_FULLFSYNC "fcntl.h" HAVE_FULLFSYNC)
check_cxx_symbol_exists(O_CLOEXEC "fcntl.h" HAVE_O_CLOEXEC)

set(CMAKE_CXX_FLAGS "-DDLLX= ${CMAKE_CXX_FLAGS}")

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # Disable C++ exceptions.
  string(REGEX REPLACE "/EH[a-z]+" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHs-c-")
  add_definitions(-D_HAS_EXCEPTIONS=0)

  # Disable RTTI.
  string(REGEX REPLACE "/GR" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
else(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # Enable strict prototype warnings for C code in clang and gcc.
  if(NOT CMAKE_C_FLAGS MATCHES "-Wstrict-prototypes")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wstrict-prototypes")
  endif(NOT CMAKE_C_FLAGS MATCHES "-Wstrict-prototypes")

  # Disable C++ exceptions.
  string(REGEX REPLACE "-fexceptions" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")

  # Disable RTTI.
  string(REGEX REPLACE "-frtti" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

# Test whether -Wthread-safety is available. See
# https://clang.llvm.org/docs/ThreadSafetyAnalysis.html
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-Wthread-safety HAVE_CLANG_THREAD_SAFETY)

# Used by googletest.
check_cxx_compiler_flag(-Wno-missing-field-initializers
                        LEVELDB_HAVE_NO_MISSING_FIELD_INITIALIZERS)

include(CheckCXXSourceCompiles)

# Test whether C++17 __has_include is available.
check_cxx_source_compiles("
#if defined(__has_include) &&  __has_include(<string>)
#include <string>
#endif
int main() { std::string str; return 0; }
" HAVE_CXX17_HAS_INCLUDE)

set(LEVELDB_PUBLIC_INCLUDE_DIR "include/leveldb")
set(LEVELDB_PORT_CONFIG_DIR "include/port")

include_directories(
  "${PROJECT_BINARY_DIR}/include"
  "."
)

# Must be included before CMAKE_INSTALL_INCLUDEDIR is used.
include(GNUInstallDirs)

add_library(leveldb "")
target_sources(leveldb
  PRIVATE
    "db/builder.cc"
    "db/c.cc"
    "db/dbformat.cc"
    "db/db_impl.cc"
    "db/db_iter.cc"
    "db/dumpfile.cc"
    "db/filename.cc"
    "db/log_reader.cc"
    "db/log_writer.cc"
    "db/memtable.cc"
    "db/repair.cc"
    "db/snappy_compressor.cc"
    "db/table_cache.cc"
    "db/version_edit.cc"
    "db/version_set.cc"
    "db/write_batch.cc"
    "db/zlib_compressor.cc"
    "db/zopfli_compressor.cc"
    "db/zstd_compressor.cc"
    "table/block_builder.cc"
    "table/block.cc"
    "table/filter_block.cc"
    "table/format.cc"
    "table/iterator.cc"
    "table/merger.cc"
    "table/table_builder.cc"
    "table/table.cc"
    "table/two_level_iterator.cc"
    "util/arena.cc"
    "util/bloom.cc"
    "util/cache.cc"
    "util/coding.cc"
    "util/comparator.cc"
    "util/crc32c.cc"
    "util/env_boost.cc"
    "util/env.cc"
    "util/env_posix.cc"
    "util/env_win.cc"
    "util/filter_policy.cc"
    "util/hash.cc"
    "util/histogram.cc"
    "util/logging.cc"
    "util/options.cc"
    "util/status.cc"
    "util/win_logger.cc"
    "port/port_posix.cc"
    "port/port_posix_sse.cc"

  # Only CMake 3.3+ supports PUBLIC sources in targets exported by "install".
  $<$<VERSION_GREATER:CMAKE_VERSION,3.2>:PUBLIC>
    "${LEVELDB_PUBLIC_INCLUDE_DIR}/c.h"
    "${LEVELDB_PUBLIC_INCLUDE_DIR}/cache.h"
    "${LEVELDB_PUBLIC_INCLUDE_DIR}/comparator.h"
    "${LEVELDB_PUBLIC_INCLUDE_DIR}/compressor.h"
    "${LEVELDB_PUBLIC_INCLUDE_DIR}/zlib_compressor.h"
    "${LEVELDB_PUBLIC_INCLUDE_DIR}/snappy_compressor.h"
    "${LEVELDB_PUBLIC_INCLUDE_DIR}/db.h"
    "${LEVELDB_PUBLIC_INCLUDE_DIR}/decompress_allocator.h"
    "${LEVELDB_PUBLIC_INCLUDE_DIR}/dumpfile.h"
    "${LEVELDB_PUBLIC_INCLUDE_DIR}/env.h"
    "${LEVELDB_PUBLIC_INCLUDE_DIR}/filter_policy.h"
    "${LEVELDB_PUBLIC_INCLUDE_DIR}/iterator.h"
    "${LEVELDB_PUBLIC_INCLUDE_DIR}/options.h"
    "${LEVELDB_PUBLIC_INCLUDE_DIR}/slice.h"
    "${LEVELDB_PUBLIC_INCLUDE_DIR}/status.h"
    "${LEVELDB_PUBLIC_INCLUDE_DIR}/table_builder.h"
    "${LEVELDB_PUBLIC_INCLUDE_DIR}/table.h"
    "${LEVELDB_PUBLIC_INCLUDE_DIR}/write_batch.h"
)

if (WIN32)
  target_sources(leveldb
    PRIVATE
      "util/env_windows.cc"
      "util/windows_logger.h"
  )
else (WIN32)
  target_sources(leveldb
    PRIVATE
      "util/env_posix.cc"
      "util/posix_logger.h"
  )
endif (WIN32)

# MemEnv is not part of the interface and could be pulled to a separate library.
target_sources(leveldb
  PRIVATE
    "helpers/memenv/memenv.cc"
    "helpers/memenv/memenv.h"
)

target_include_directories(leveldb
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(leveldb
  PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

target_compile_definitions(leveldb
  PRIVATE
    ${LEVELDB_PLATFORM_NAME}=1
)
if (NOT HAVE_CXX17_HAS_INCLUDE)
  target_compile_definitions(leveldb
    PRIVATE
      LEVELDB_HAS_PORT_CONFIG_H=1
  )
endif(NOT HAVE_CXX17_HAS_INCLUDE)

if(HAVE_CLANG_THREAD_SAFETY)
  target_compile_options(leveldb
    PUBLIC
      -Werror -Wthread-safety)
endif(HAVE_CLANG_THREAD_SAFETY)

if(HAVE_CRC32C)
  target_link_libraries(leveldb crc32c)
endif(HAVE_CRC32C)
if(HAVE_SNAPPY)
  target_link_libraries(leveldb snappy)
endif(HAVE_SNAPPY)
if(HAVE_TCMALLOC)
  target_link_libraries(leveldb tcmalloc)
endif(HAVE_TCMALLOC)

# Needed by port_stdcxx.h
find_package(Threads REQUIRED)
target_link_libraries(leveldb Threads::Threads)

find_package(ZLIB REQUIRED)
target_link_libraries(leveldb ZLIB::ZLIB)

add_executable(leveldbutil
  "db/leveldbutil.cc"
)
target_link_libraries(leveldbutil leveldb)


if(LEVELDB_INSTALL)
  install(TARGETS leveldb
    EXPORT leveldbTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
  install(
    FILES
      "${LEVELDB_PUBLIC_INCLUDE_DIR}/c.h"
      "${LEVELDB_PUBLIC_INCLUDE_DIR}/cache.h"
      "${LEVELDB_PUBLIC_INCLUDE_DIR}/comparator.h"
      "${LEVELDB_PUBLIC_INCLUDE_DIR}/db.h"
      "${LEVELDB_PUBLIC_INCLUDE_DIR}/dumpfile.h"
      "${LEVELDB_PUBLIC_INCLUDE_DIR}/env.h"
      "${LEVELDB_PUBLIC_INCLUDE_DIR}/filter_policy.h"
      "${LEVELDB_PUBLIC_INCLUDE_DIR}/iterator.h"
      "${LEVELDB_PUBLIC_INCLUDE_DIR}/options.h"
      "${LEVELDB_PUBLIC_INCLUDE_DIR}/slice.h"
      "${LEVELDB_PUBLIC_INCLUDE_DIR}/status.h"
      "${LEVELDB_PUBLIC_INCLUDE_DIR}/table_builder.h"
      "${LEVELDB_PUBLIC_INCLUDE_DIR}/table.h"
      "${LEVELDB_PUBLIC_INCLUDE_DIR}/write_batch.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/leveldb"
  )

  include(CMakePackageConfigHelpers)
  configure_package_config_file(
    "cmake/${PROJECT_NAME}Config.cmake.in"
    "${PROJECT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
  )
  write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
  )
  install(
    EXPORT leveldbTargets
    NAMESPACE leveldb::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
  )
  install(
    FILES
      "${PROJECT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake"
      "${PROJECT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
  )
endif(LEVELDB_INSTALL)