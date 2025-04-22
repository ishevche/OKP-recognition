#include <boost/property_map/dynamic_property_map.hpp>
#include <iostream>
#include "okp_recognition.h"
#include "graphIO.h"
#include "argument_parser.h"
#include "timer.h"

std::unique_ptr<solver> get_solver(solver_type type, bool use_bct, const Graph& graph) {
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
    case OKP_SOLVER:
        if (!use_bct) {
            return std::make_unique<okp_solver>(graph, 0);
        }
        return std::make_unique<bicomponent_solver<okp_solver>>(graph, 0);
    default:
        return std::make_unique<bicomponent_solver<ilp_solver>>(graph, 0);
    }
}

int main(int ac, char** av) {
    command_line_options_t cmd_arguments(ac, av);
    Graph graph;

    boost::dynamic_properties graph_props;
    graph_props.property("node_id", get(&VertexStruct::name, graph));
    graph_props.property("pos", get(&VertexStruct::location, graph));
    graph_props.property("color", get(&EdgeStruct::color, graph));
    read_graphviz(cmd_arguments.input_graph, graph, graph_props);

    auto edge_index_map = get(boost::edge_index, graph);

    int edge_id = 0;
    for (auto [ei, ei_end] = edges(graph); ei != ei_end; ++ei) {
        put(edge_index_map, *ei, edge_id++);
    }

    std::unique_ptr<solver> solver = get_solver(cmd_arguments.method, !cmd_arguments.no_bct_decomposition, graph);
    auto start = get_current_time_fenced();
    bool solved = solver->solve();
    auto end = get_current_time_fenced();
    std::cout << solved << " " << solver->crossing_number << " " << to_ns(end - start) << std::endl;

    if (solved && !cmd_arguments.output_file.empty()) {
        save_dot(cmd_arguments.output_file, graph, solver->vertex_order, solver->crossing_number, graph_props);
    }
    return 0;
}
