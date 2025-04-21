cmake_minimum_required(VERSION 3.31.1)
project(okp-recognition CXX)
set(CMAKE_CXX_STANDARD 20)
set(CXX ON)

set(SOURCES
        src/main.cpp
        src/graphIO.cpp
        src/ilp.cpp
        src/sat.cpp
        src/bctree.cpp
        src/argument_parser.cpp
        src/okp_solver.cpp
)

add_executable(okp-recognition-cro ${SOURCES})
add_executable(okp-recognition ${SOURCES})

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
find_package(Boost COMPONENTS graph program_options system REQUIRED)
find_package(GUROBI REQUIRED)
find_library(KISSAT_LIBRARY NAMES kissat REQUIRED PATH_SUFFIXES build)
find_path(KISSAT_INCLUDE_DIR NAMES kissat.h REQUIRED PATH_SUFFIXES src)

set(INCLUDES
        ${GUROBI_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIR}
        ${KISSAT_INCLUDE_DIR}
        include
)
set(LIBS
        ${GUROBI_LIBRARY}
        ${GUROBI_CXX_LIBRARY}
        ${KISSAT_LIBRARY}
        ${Boost_LIBRARIES}
)

target_include_directories(okp-recognition PRIVATE ${INCLUDES})
target_include_directories(okp-recognition-cro PRIVATE ${INCLUDES})
target_link_libraries(okp-recognition ${LIBS})
target_link_libraries(okp-recognition-cro ${LIBS})

target_compile_definitions(okp-recognition-cro PRIVATE ILP_CROSSING_SUM_OPTIMIZATION)

set(PROPERTIES
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
)

set_target_properties(okp-recognition ${PROPERTIES})
set_target_properties(okp-recognition-cro ${PROPERTIES})
