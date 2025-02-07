#include <iostream>
#include <iomanip>
#include "process_graph.h"
#include "argument_parser.h"


int main(int ac, char **av) {
    command_line_options_t cmd_arguments(ac, av);

    std::ifstream in(cmd_arguments.input_file);
    std::string input_string;
    std::vector<std::string> graph_strings;
    while (in >> input_string) {
        graph_strings.push_back(input_string);
    }
    in.close();

    for (int i = 0; i < graph_strings.size(); ++i) {
        const std::string &graph_string = graph_strings[i];
        std::cout << std::setw(4) << i + 1 << ": " << graph_string;
        process_graph(graph_string);
    }
    return 0;
}
