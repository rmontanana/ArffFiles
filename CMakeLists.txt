cmake_minimum_required(VERSION 3.20)

project(ArffFiles
  VERSION 1.2.1
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
  find_package(Catch2 REQUIRED)
  include(CTest)
  add_subdirectory(tests)
endif (ENABLE_TESTING)

add_library(ArffFiles INTERFACE ArffFiles.hpp)

target_include_directories(ArffFiles INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/configured_files/include>
    $<INSTALL_INTERFACE:include>
)
# Install
# -------
install(TARGETS ArffFiles EXPORT ArffFilesTargets
  INCLUDES DESTINATION include
)
install(EXPORT ArffFilesTargets
  FILE ArffFilesTargets.cmake
  NAMESPACE ArffFiles::
  DESTINATION lib/cmake/ArffFiles
)

# Install the main header file
install(FILES ArffFiles.hpp
  DESTINATION include
)

# Install the generated configuration header
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/configured_files/include/arffFiles_config.h"
  DESTINATION include
)

# Install documentation files
install(FILES LICENSE README.md
  DESTINATION share/doc/ArffFiles
)

# Create and install package configuration files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/ArffFilesConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ArffFilesConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/ArffFilesConfig.cmake"
  INSTALL_DESTINATION lib/cmake/ArffFiles
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/ArffFilesConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/ArffFilesConfigVersion.cmake"
  DESTINATION lib/cmake/ArffFiles
)
