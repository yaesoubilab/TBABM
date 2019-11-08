cmake_minimum_required(VERSION 3.9)
project(TBABM)

# Define library version
# set(TBABM_VERSION_MAJOR 0 CACHE STRING "major version" FORCE)
# set(TBABM_VERSION_MINOR 1 CACHE STRING "minor version" FORCE)
# set(TBABM_VERSION ${TBABM_VERSION_MAJOR}.${TBABM_VERSION_MINOR} CACHE STRING "version" FORCE)

# Define options here
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add subdirectories
add_subdirectory(src)

enable_testing()
add_subdirectory(tests)

# install(FILES TBABMConfig.cmake DESTINATION ${main_lib_dest})
# install(EXPORT TBABM DESTINATION ${lib_dest})
