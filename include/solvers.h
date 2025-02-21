#ifndef OKP_RECOGNITION_SOLVERS_H
#define OKP_RECOGNITION_SOLVERS_H

#include <vector>
#include "graph.h"

struct SolverParams {
    Graph &graph;
    std::vector<Vertex> &ordering;
    int number_of_crossings;
    bool converged;
};

void ILPSolver(SolverParams &params);
void SATSolver(SolverParams &params);

#endif //OKP_RECOGNITION_SOLVERS_H
