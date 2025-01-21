cmake_minimum_required(VERSION 3.31.1)
project(okp-recognition CXX)
set(CMAKE_CXX_STANDARD 17)
set(CXX ON)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

add_executable(gurobi_test
        src/main.cpp
        src/graphIO.cpp
        src/ilp.cpp
        src/sat.cpp
)

find_package(OGDF CONFIG PATHS ${OGDF_CONFIG_SEARCH_PATH} REQUIRED)
find_package(GUROBI REQUIRED)
find_library(KISSAT_LIBRARY kissat "${CMAKE_SOURCE_DIR}/../kissat/build" REQUIRED)

target_include_directories(gurobi_test PRIVATE
        ${GUROBI_INCLUDE_DIRS}
        ${OGDF_INCLUDE_DIRS}
        "${CMAKE_SOURCE_DIR}/../kissat/src"
        include)
target_link_libraries(gurobi_test
        ${GUROBI_LIBRARY}
        ${GUROBI_CXX_LIBRARY}
        ${KISSAT_LIBRARY}
        OGDF)
