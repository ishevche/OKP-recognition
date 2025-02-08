#include <iostream>
#include <iomanip>
#include <fstream>
#include <boost/property_map/dynamic_property_map.hpp>
#include "graph.h"
#include "process_graph.h"
#include "argument_parser.h"


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
        Graph g;

        boost::read_graphviz(
                graph_strings[i], g,
                boost::dynamic_properties()
                        .property("name", boost::get(&Vertex::name, g))
        );

        std::cout << std::setw(4) << i + 1 << ": " << graph_strings[i];
//        process_graph(g);
    }
    return 0;
}
