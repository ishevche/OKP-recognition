#ifndef OKP_RECOGNITION_ARGUMENT_PARSER_H
#define OKP_RECOGNITION_ARGUMENT_PARSER_H

#include <boost/program_options.hpp>
#include <string>

class command_line_options_t {
public:
    command_line_options_t();
    command_line_options_t(int ac, char** av);

    command_line_options_t(const command_line_options_t&) = default;
    command_line_options_t& operator=(const command_line_options_t&) = delete;
    command_line_options_t(command_line_options_t&&) = default;
    command_line_options_t& operator=(command_line_options_t&&) = delete;

    ~command_line_options_t() = default;

    void parse(int ac, char** av);

    std::string input_file;
    std::string output_file;

private:
    boost::program_options::variables_map var_map{};
    boost::program_options::options_description opt_conf{
        "Usage:\n\tokp-recognition [options]\n"
        "Allowed options:"
    };
    boost::program_options::positional_options_description popt_conf{};

    static void assert_exists(const std::string& file);
};

#endif //OKP_RECOGNITION_ARGUMENT_PARSER_H
