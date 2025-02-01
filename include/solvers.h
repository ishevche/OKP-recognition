#ifndef OKP_RECOGNITION_SOLVERS_H
#define OKP_RECOGNITION_SOLVERS_H

#include <ogdf/basic/Graph.h>
#include <vector>

struct SolverParams {
    const ogdf::Graph &graph;
    std::vector<ogdf::node> &ordering;
    int number_of_crossings;
    bool converged;
};

void ILPSolver(SolverParams &params);
void SATSolver(SolverParams &params);

#endif //OKP_RECOGNITION_SOLVERS_H
