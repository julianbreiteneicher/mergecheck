cmake_minimum_required(VERSION 2.8)

project(mergecheck)

enable_language(CXX)

set(WARNING "-Wall -Wextra")
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # using Clang
  set(WARNING "${WARNING} -Wpedantic")
endif()

option(CMAKE_CXX_FLAGS_SAN_ASAN "Enable address sanitizer" OFF)
option(CMAKE_CXX_FLAGS_SAN_MEM "Enable memory sanitizer" OFF)
option(CMAKE_CXX_FLAGS_SAN_UBSAN "Enable undevined behaviour sanitizer" OFF)
option(CMAKE_CXX_FLAGS_COLOROUT "Enable clang color output" ON)

if (CMAKE_CXX_FLAGS_SAN_ASAN OR CMAKE_CXX_FLAGS_SAN_MEM OR
    CMAKE_CXX_FLAGS_SAN_UBSAN)
  set(SANITIZERS "${SANITIZERS}-fsanitize=")
endif()

if (CMAKE_CXX_FLAGS_SAN_ASAN)
  set(SANITIZERS "${SANITIZERS},address")
endif()

if (CMAKE_CXX_FLAGS_SAN_MEM)
  set(SANITIZERS "${SANITIZERS},memory")
endif()

if (CMAKE_CXX_FLAGS_SAN_UBSAN)
  set(SANITIZERS "${SANITIZERS},undefined")
endif()

if (CMAKE_CXX_FLAGS_COLOROUT)
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")
  endif()
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZERS}")

set(FLAGS "-O3 -std=c++14")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING} ${FLAGS}")

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include_directories(
  ${PROJECT_SOURCE_DIR}/include
)

#  Libraries  #
include(ExternalProject)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Libgit2 libs
find_package(Libgit2 REQUIRED)
include_directories(${LIBGIT2_INCLUDE_DIR})
list(APPEND COMMON_LIBS ${LIBGIT2_LIBRARIES})

# Boost libs
set(Boost_USE_STATIC_LIBS   ON)
find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
add_definitions(-DUSE_BOOST_)
list(APPEND COMMON_LIBS ${Boost_LIBRARIES})

add_subdirectory(src)
