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
        src/bctree.cpp
        src/argument_parser.cpp
        src/okp_solver.cpp
)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
find_package(Boost COMPONENTS graph program_options system REQUIRED)
find_package(GUROBI REQUIRED)
find_library(KISSAT_LIBRARY NAMES kissat REQUIRED PATH_SUFFIXES build)
find_path(KISSAT_INCLUDE_DIR NAMES kissat.h REQUIRED PATH_SUFFIXES src)

target_include_directories(okp-recognition PRIVATE
        ${GUROBI_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIR}
        ${KISSAT_INCLUDE_DIR}
        include)
target_link_libraries(okp-recognition
        ${GUROBI_LIBRARY}
        ${GUROBI_CXX_LIBRARY}
        ${KISSAT_LIBRARY}
        ${Boost_LIBRARIES})

if (ILP_CROSSING_SUM_OPTIMIZATION)
    target_compile_definitions(okp-recognition PRIVATE ILP_CROSSING_SUM_OPTIMIZATION)
endif ()

set_target_properties(${PROJECT_NAME}
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
)
