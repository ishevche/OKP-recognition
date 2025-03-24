#include <iostream>
#include <iomanip>
#include <fstream>
#include <okp_solver.h>
#include <boost/property_map/dynamic_property_map.hpp>
#include "graph.h"
#include "argument_parser.h"
#include "timer.h"
#include "bicomponent_solver.h"


std::vector<std::string> split(const std::string &string, const std::string &delimiter) {
    std::vector<std::string> result;
    size_t offset = 0;
    for (size_t already_looked = 0; already_looked != std::string::npos; already_looked = offset) {
        offset = string.find(delimiter, offset + 1);
        result.push_back(string.substr(already_looked, offset - already_looked));
    }
    return result;
}


int main(int ac, char **av) {
    command_line_options_t cmd_arguments(ac, av);

    std::ifstream in(cmd_arguments.input_file);
    std::stringstream input_string;
    input_string << in.rdbuf();
    in.close();

    std::string file_data = input_string.str();
    std::vector<std::string> graph_strings = split(file_data, "graph G {");

    for (const auto &graph_string: graph_strings) {
        Graph graph;

        boost::dynamic_properties graph_props;
        graph_props.property("node_id", boost::get(&VertexStruct::name, graph));
        graph_props.property("pos", boost::get(&VertexStruct::location, graph));
        graph_props.property("color", boost::get(&EdgeStruct::color, graph));
        boost::read_graphviz(graph_string, graph, graph_props);

        auto edge_index_map = boost::get(boost::edge_index, graph);

        int edge_id = 0;
        for (auto [ei, ei_end] = boost::edges(graph); ei != ei_end; ++ei) {
            boost::put(edge_index_map, *ei, edge_id++);
        }
        std::cout << boost::num_vertices(graph) << std::endl;

        okp_solver solver(graph, 11);

        solver.solve();

        // std::cout << solver.crossing_number << std::endl;

        // save_dot("data/out/tmp.dot", graph, solver.vertex_order, solver.crossing_number, graph_props);
    }
    return 0;
}
