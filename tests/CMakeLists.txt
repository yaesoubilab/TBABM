add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(SimulationLib REQUIRED)
find_package(StatisticalDistributionsLib REQUIRED)

add_executable (TBABMtest
                tests-main.cpp)

target_link_libraries(TBABMtest Catch SimulationLib StatisticalDistributionsLib)

add_test(NAME MyTest COMMAND Test WORKING_DIRECTORY <${CMAKE_CURRENT_SOURCE_DIR}>)
