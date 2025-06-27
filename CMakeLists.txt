cmake_minimum_required(VERSION 3.20)

project(ArffFiles
  VERSION 1.2.0
  DESCRIPTION "Library to read Arff Files and return STL vectors with the data read."
  HOMEPAGE_URL "https://github.com/rmontanana/ArffFiles"
  LANGUAGES CXX
)

if (CODE_COVERAGE AND NOT ENABLE_TESTING)
  MESSAGE(FATAL_ERROR "Code coverage requires testing enabled")
endif (CODE_COVERAGE AND NOT ENABLE_TESTING)

if (POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif ()

# Global CMake variables
# ----------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED            ON)
set(CMAKE_CXX_EXTENSIONS                  OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS          ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
# Options
# -------
option(ENABLE_TESTING "Unit testing build"                        OFF)

# CMakes modules
# --------------
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
include(AddGitSubmodule)
include(CodeCoverage)

# Subdirectories
# --------------
add_subdirectory(config)

# Testing
# -------
if (ENABLE_TESTING)
  MESSAGE("Testing enabled")
  Include(FetchContent)
    FetchContent_Declare(Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.3.2
    )
    FetchContent_MakeAvailable(Catch2)
  include(CTest)
  add_subdirectory(tests)
endif (ENABLE_TESTING)

add_library(ArffFiles INTERFACE ArffFiles.hpp)
