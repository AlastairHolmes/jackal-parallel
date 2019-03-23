cmake_minimum_required(VERSION 3.9)
project("jackal-parallel")

include(${CMAKE_CURRENT_LIST_DIR}/ext/common-cmake/common.cmake)

# Set the default install location.
SetDefaultInstallPrefix("${CMAKE_SOURCE_DIR}/bin")

# Load the project version from a file 'VERSION'
SetProjectVersionFromFile(JKPARALLEL ${CMAKE_CURRENT_LIST_DIR}/meta/version/VERSION)

# Enable target groups for the Visual Studio IDE
EnableGroups()

# Enable Testing
EnableTests()

# Generate Documentation
add_subdirectory("${CMAKE_SOURCE_DIR}/meta/doxy")

# Main Library
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/src/jkparallel")

# Tests
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/test/jkparallel_tests")

# Installs the project's targets
SetupExport("JKParallel")
InstallExport("JKParallel"
	NAMESPACE "JKParallel")