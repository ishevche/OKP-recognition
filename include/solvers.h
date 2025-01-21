#ifndef OKP_RECOGNITION_SOLVERS_H
#define OKP_RECOGNITION_SOLVERS_H

#include <ogdf/basic/Graph.h>

int ILPSolver(const ogdf::Graph &graph, ogdf::NodeArray<int> &ordering);
bool SATSolver(const ogdf::Graph &graph, int number_of_crossings, ogdf::NodeArray<int> &ordering);

#endif //OKP_RECOGNITION_SOLVERS_H
