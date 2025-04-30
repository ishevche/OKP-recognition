#ifndef OKP_RECOGNITION_H
#define OKP_RECOGNITION_H

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <string>
#include "bicomponent_solver.hpp"
#include "ilp_solver.h"
#include "sat_solver.h"
#include "dp_solver.h"

enum solver_type {
    ILP_SOLVER,
    SAT_SOLVER,
    DP_SOLVER,
};

inline std::istream& operator>>(std::istream& in, solver_type& solver_type) {
    std::string token;
    in >> token;
    boost::algorithm::to_lower(token);
    if (token == "ilp") {
        solver_type = ILP_SOLVER;
    } else if (token == "sat") {
        solver_type = SAT_SOLVER;
    } else if (token == "dp") {
        solver_type = DP_SOLVER;
    } else {
        throw boost::program_options::validation_error(
            boost::program_options::validation_error::invalid_option,
            "method", token);
    }
    return in;
}


#endif //OKP_RECOGNITION_H
