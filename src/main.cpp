#include <boost/property_map/dynamic_property_map.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>
#include "graphIO.h"
#include "sat_solver.h"
#include "graph.h"
#include "argument_parser.h"
#include "timer.h"
#include "bicomponent_solver.h"


std::vector<std::string> split(const std::string& string, const std::string& delimiter) {
    std::vector<std::string> result;
    size_t offset = 0;
    for (size_t already_looked = 0; already_looked != std::string::npos; already_looked = offset) {
        offset = string.find(delimiter, offset + 1);
        result.push_back(string.substr(already_looked, offset - already_looked));
    }
    return result;
}


int main(int ac, char** av) {
    command_line_options_t cmd_arguments(ac, av);

    std::ifstream in(cmd_arguments.input_file);
    std::stringstream input_string;
    input_string << in.rdbuf();
    in.close();

    std::string file_data = input_string.str();
    std::vector<std::string> graph_strings = split(file_data, "graph G {");

    for (const auto& graph_string : graph_strings) {
        Graph graph;

        boost::dynamic_properties graph_props;
        graph_props.property("node_id", get(&VertexStruct::name, graph));
        graph_props.property("pos", get(&VertexStruct::location, graph));
        graph_props.property("color", get(&EdgeStruct::color, graph));
        read_graphviz(graph_string, graph, graph_props);
        // save_dot("data/out/a.dot", graph, {0, 1, 2, 5, 6, 4, 3}, 4, graph_props);

        auto edge_index_map = get(boost::edge_index, graph);

        int edge_id = 0;
        for (auto [ei, ei_end] = edges(graph); ei != ei_end; ++ei) {
            put(edge_index_map, *ei, edge_id++);
        }

        sat_solver solver(graph, 0);
        auto start = get_current_time_fenced();
        solver.solve();
        auto end = get_current_time_fenced();

        std::string name = graph_to_g6(graph);
        std::cout << name << "\nSAT: ";
        std::cout << solver.crossing_number << " " << to_ns(end - start) << std::endl;
#ifndef NDEBUG
        std::cout << "H/M: " << solver.hits << "/" << solver.misses << std::endl;
#endif

        save_dot("data/out/" + name + ".dot", graph, solver.vertex_order, solver.crossing_number, graph_props);
    }
    return 0;
}
