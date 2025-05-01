#include <iostream>
#include <filesystem>
#include "argument_parser.h"

#include <okp_recognition.h>

namespace po = boost::program_options;
namespace fs = std::filesystem;

command_line_options_t::command_line_options_t() {
    opt_conf.add_options()
        ("help,h", "Show help message")
        ("input-graph,i", po::value<std::string>(&input_graph)->required(),
         "Input graph in Graphviz format. Required.")
        ("output-file,o", po::value<std::string>(&output_file)->default_value(""),
         "Path to output file used to save graph drawing in Graphviz format. Default: \"\" (output is ignored)")
        ("method,m", po::value<solver_type>(&method)->default_value(ILP_SOLVER),
         "Method to use for calculating the drawing of the input graph. One of the ILP, SAT, DP. Default: ILP")
        ("no-bct,b", po::bool_switch(&no_bct_decomposition)->default_value(false),
         "Include this to disable biconnected decomposition before passing it to the solver");
    popt_conf.add("input-graph", 1);
}

command_line_options_t::command_line_options_t(int ac, char** av) :
    command_line_options_t() {
    parse(ac, av);
}

void command_line_options_t::parse(int ac, char** av) {
    auto parsed = po::command_line_parser(ac, av)
                  .options(opt_conf).positional(popt_conf).run();
    store(parsed, var_map);
    if (var_map.contains("help")) {
        std::cout << opt_conf << "\n";
        exit(EXIT_SUCCESS);
    }
    notify(var_map);
}
