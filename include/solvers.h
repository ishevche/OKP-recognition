#ifndef OKP_RECOGNITION_SOLVERS_H
#define OKP_RECOGNITION_SOLVERS_H

#include <vector>
#include <functional>
#include "graph.h"

struct SolverParams {
    const Graph &graph;
    std::vector<Vertex> &ordering;
    int number_of_crossings;
    bool converged;
};

void ILPSolver(SolverParams &params);
void SATSolver(SolverParams &params);
void bicomponent_solver(SolverParams &params, std::function<void(SolverParams &)> solver);

#endif //OKP_RECOGNITION_SOLVERS_H
