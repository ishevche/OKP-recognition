cmake_minimum_required(VERSION 3.31.1)
project(okp-recognition CXX)
set(CMAKE_CXX_STANDARD 20)
set(CXX ON)


set(ILP_CROSSING_SUM_OPTIMIZATION ON)


add_executable(okp-recognition
        src/main.cpp
        src/graphIO.cpp
        src/ilp.cpp
        src/sat.cpp
        src/bcTree.cpp
)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
find_package(OGDF CONFIG PATHS ${OGDF_CONFIG_SEARCH_PATH} REQUIRED)
find_package(GUROBI REQUIRED)
find_library(KISSAT_LIBRARY
        NAMES kissat
        HINTS "${CMAKE_SOURCE_DIR}/../kissat/build"
        REQUIRED
)

target_include_directories(okp-recognition PRIVATE
        ${GUROBI_INCLUDE_DIRS}
        ${OGDF_INCLUDE_DIRS}
        "${CMAKE_SOURCE_DIR}/../kissat/src"
        include)
target_link_libraries(okp-recognition
        ${GUROBI_LIBRARY}
        ${GUROBI_CXX_LIBRARY}
        ${KISSAT_LIBRARY}
        OGDF)

if (ILP_CROSSING_SUM_OPTIMIZATION)
    target_compile_definitions(okp-recognition PRIVATE ILP_CROSSING_SUM_OPTIMIZATION)
endif ()