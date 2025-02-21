#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <boost/property_map/dynamic_property_map.hpp>
#include "graph.h"
#include "process_graph.h"
#include "argument_parser.h"
#include "graphIO.h"
#include "solvers.h"
#include "timer.h"


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

    for (int i = 0; i < graph_strings.size(); ++i) {
        Graph graph;

        boost::dynamic_properties graph_props;
        graph_props.property("node_id", boost::get(&VertexStruct::name, graph));
        graph_props.property("pos", boost::get(&VertexStruct::location, graph));
        graph_props.property("color", boost::get(&EdgeStruct::color, graph));
        boost::read_graphviz(graph_strings[i], graph, graph_props);

        std::vector<Vertex> ILP_order;
        SolverParams params{graph, ILP_order};

        auto start = get_current_time_fenced();
        ILPSolver(params);
        auto end = get_current_time_fenced();

        std::cout << i << "/776:\nILP: " << params.number_of_crossings <<
                  " " << to_ns(end - start) << std::endl;

        save_dot("data/out/" + std::to_string(i) + ".dot", graph,
                 ILP_order, params.number_of_crossings, graph_props);
    }
    return 0;
}
