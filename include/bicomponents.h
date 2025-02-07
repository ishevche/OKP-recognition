#ifndef OKP_RECOGNITION_PREPROCESS_H
#define OKP_RECOGNITION_PREPROCESS_H

#include <functional>
#include <ogdf/basic/Graph.h>
#include <ogdf/decomposition/BCTree.h>
#include "solvers.h"

int solve_for_each_component(SolverParams &solverParams,
                             const std::function<void(SolverParams &)> &solver);

SolverParams solve_biconnected_component(ogdf::BCTree &bc_tree, ogdf::node b_component,
                                         const SolverParams &solverParams,
                                         const std::function<void(SolverParams &)> &solve);


#endif //OKP_RECOGNITION_PREPROCESS_H
