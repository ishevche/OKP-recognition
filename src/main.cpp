#include <boost/property_map/dynamic_property_map.hpp>
#include <boost/graph/connected_components.hpp>
#include <iostream>

#include "okp_recognition.h"
#include "graphIO.h"
#include "argument_parser.h"
#include "timer.h"

std::unique_ptr<abstract_solver> get_solver(solver_type type, bool use_bct, const graph_t& graph) {
    switch (type) {
    case ILP_SOLVER:
        if (!use_bct) {
            return std::make_unique<ilp_solver>(graph, 0);
        }
        return std::make_unique<bicomponent_solver<ilp_solver>>(graph, 0);
    case SAT_SOLVER:
        if (!use_bct) {
            return std::make_unique<sat_solver>(graph, 0);
        }
        return std::make_unique<bicomponent_solver<sat_solver>>(graph, 0);
    case DP_SOLVER:
        if (!use_bct) {
            return std::make_unique<dp_solver>(graph, 0);
        }
        return std::make_unique<bicomponent_solver<dp_solver>>(graph, 0);
    default:
        return std::make_unique<bicomponent_solver<ilp_solver>>(graph, 0);
    }
}

int main(int ac, char** av) {
    command_line_options_t cmd_arguments(ac, av);
    graph_t graph;

    boost::dynamic_properties graph_props;
    graph_props.property("node_id", get(&vertex_struct::name, graph));
    graph_props.property("pos", get(&vertex_struct::location, graph));
    graph_props.property("color", get(&edge_struct::color, graph));
    read_graphviz(cmd_arguments.input_graph, graph, graph_props);

    if (connected_components(graph, boost::dummy_property_map()) != 1) {
#ifndef PERF_TEST
        std::cout << "ERROR: The input graph is not connected!" << std::endl;
#else
        std::cout << "0 0 0" << std::endl;
#endif
        return 0;
    }

    auto edge_index_map = get(boost::edge_index, graph);

    int edge_id = 0;
    for (auto [ei, ei_end] = edges(graph); ei != ei_end; ++ei) {
        put(edge_index_map, *ei, edge_id++);
    }

    std::unique_ptr<abstract_solver> solver = get_solver(cmd_arguments.method, !cmd_arguments.no_bct_decomposition, graph);
    auto start = get_current_time_fenced();
    bool solved = solver->solve();
    auto end = get_current_time_fenced();
#ifndef PERF_TEST
    if (solved) {
        std::cout << "Crossing number: " << solver->crossing_number << std::endl;
        std::cout << "Time elapsed: " << to_ns(end - start) / 1.0e9 << " s" << std::endl;
    } else {
        std::cout << "The solver wasn't able to find a solution!" << std::endl;
    }
#else
    std::cout << solved << " " << solver->crossing_number << " " << to_ns(end - start) << std::endl;
#endif

    if (solved && !cmd_arguments.output_file.empty()) {
        save_dot(cmd_arguments.output_file, graph, solver->vertex_order, solver->crossing_number, graph_props);
    }
    return 0;
}
