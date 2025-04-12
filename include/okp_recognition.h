#ifndef OKP_RECOGNITION_H
#define OKP_RECOGNITION_H

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <string>
#include "bicomponent_solver.h"
#include "ilp_solver.h"
#include "sat_solver.h"
#include "okp_solver.h"

enum solver_type {
    ILP_SOLVER,
    SAT_SOLVER,
    OKP_SOLVER,
};

inline std::istream& operator>>(std::istream& in, solver_type& solverType) {
    std::string token;
    in >> token;
    boost::algorithm::to_lower(token);
    if (token == "ilp") {
        solverType = ILP_SOLVER;
    } else if (token == "sat") {
        solverType = SAT_SOLVER;
    } else if (token == "okp") {
        solverType = OKP_SOLVER;
    } else {
        throw boost::program_options::validation_error(
            boost::program_options::validation_error::invalid_option,
            "method", token);
    }
    return in;
}


#endif //OKP_RECOGNITION_H
