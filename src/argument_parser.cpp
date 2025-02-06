#include <iostream>
#include <filesystem>
#include "argument_parser.h"

namespace po = boost::program_options;

command_line_options_t::command_line_options_t() {
    opt_conf.add_options()
            ("help,h", "Show help message")
            ("input,i", po::value<std::string>(&input_file)->required(),
             "Path to input file with graphs in G6 format separated by newline character")
            ("output,o", po::value<std::string>(&output_file),
             "Path to output directory with drawing produced by all methods for each graph");
    popt_conf.add("input", 1);
    popt_conf.add("output", 1);
}

command_line_options_t::command_line_options_t(int ac, char **av) :
        command_line_options_t() {
    parse(ac, av);
}

void command_line_options_t::parse(int ac, char **av) {
    auto parsed = po::command_line_parser(ac, av)
            .options(opt_conf).positional(popt_conf).run();
    po::store(parsed, var_map);
    if (var_map.count("help")) {
        std::cout << opt_conf << "\n";
        exit(EXIT_SUCCESS);
    }
    po::notify(var_map);
    assert_exists(input_file);
    if (!output_file.empty()) {
        std::filesystem::create_directories(output_file);
    }
}

void command_line_options_t::assert_exists(const std::string &file) {
    if (!std::filesystem::exists(file)) {
        throw std::invalid_argument("File " + file + " not found!");
    }
}
