#include <iostream>
#include <filesystem>
#include "argument_parser.h"

#include <okp_recognition.h>

namespace po = boost::program_options;
namespace fs = std::filesystem;

command_line_options_t::command_line_options_t() {
    opt_conf.add_options()
        ("help,h", "Show help message")
        ("input_graph,i", po::value<std::string>(&input_graph)->required(),
         "Input graph in Graphviz format")
        ("output_file,o", po::value<std::string>(&output_file)->default_value(""),
         "Path to output file used to save graph drawing in Graphviz format")
        ("method,m", po::value<solver_type>(&method)->default_value(ILP_SOLVER),
         "Method to use for calculating the drawing of the input graph. One of the ILP, SAT, DP")
        ("no_bct,b", po::bool_switch(&no_bct_decomposition)->default_value(false),
         "Whether to decompose the graph in biconnected components before passing it to the solver");
    popt_conf.add("input_graph", 1);
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
    if (!output_file.empty()) {
        fs::create_directories(fs::path(output_file).parent_path());
    }
}

void command_line_options_t::assert_exists(const std::string& file) {
    if (!std::filesystem::exists(file)) {
        throw std::invalid_argument("File " + file + " not found!");
    }
}
