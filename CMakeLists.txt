cmake_minimum_required(VERSION 2.8.9)
project(mergecheck)

##############################
#  Projects default options  #
##############################

option( C++14               "C++14"               ON )
option( Warnings            "Warnings"            ON )
option( Debug               "Debug"               OFF )

################
##  Variables  #
################

set(SOURCES_SUBDIR src)
set(HEADERS_SUBDIR include)
set(BINARY_SUBDIR bin)
set(SOURCES_DIRECTORY ${CMAKE_SOURCE_DIR}/${SOURCES_SUBDIR})
set(HEADERS_DIRECTORY ${CMAKE_SOURCE_DIR}/${HEADERS_SUBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/${BINARY_SUBDIR})
set(BIN_NAME "mergecheck")

# export compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Add custom debug build compiler flags here...
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -03")

###########
##  Misc  #
###########

# force color output if ninja is used
if(CMAKE_MAKE_PROGRAM MATCHES "ninja")
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xclang -fcolor-diagnostics")
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
  endif()
endif()

######################
#  Sources / Headers #
######################

# Include CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
 
include_directories(${HEADERS_DIRECTORY})
 
#set(SOURCES src/main.cpp ...)
file(GLOB_RECURSE SOURCES "${SOURCES_DIRECTORY}/*.cpp")

###################
#  Build options  #
###################

# Debugging 
if(Debug)
   set(CMAKE_BUILD_TYPE "Debug")
elseif(CMAKE_BUILD_TYPE MATCHES "Debug")
   option(Debug "Debug" ON)
endif()
# Set _DEBUG #define for project
if(Debug)
   add_definitions(-D_DEBUG)
endif()

# Enable Warnings 
if(Warnings)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

if(CMAKE_BUILD_TYPE)
   message("Build type: ${CMAKE_BUILD_TYPE}")
else()
   message(STATUS "Using default build type...")
endif()

if(C++14)
  message(STATUS "Enabling C++14 support...")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  add_definitions(-DUSE_CPP14_)
  add_definitions(-DGTEST_LANG_CXX14)
endif()

###############
#  Libraries  #
###############

include(ExternalProject)

find_package(Libgit2 REQUIRED)
include_directories(${LIBGIT2_INCLUDE_DIR})
list(APPEND COMMON_LIBS ${LIBGIT2_LIBRARIES})

# Add Boost libs
set(Boost_USE_STATIC_LIBS   ON)
find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
add_definitions(-DUSE_BOOST_)
list(APPEND COMMON_LIBS ${Boost_LIBRARIES})

#######################
#  Build the project  #
#######################

# The main executable
add_executable(${BIN_NAME} ${SOURCES})
target_link_libraries(${BIN_NAME} ${COMMON_LIBS})

