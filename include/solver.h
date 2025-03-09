#ifndef OKP_RECOGNITION_SOLVER_H
#define OKP_RECOGNITION_SOLVER_H

#include <vector>
#include <functional>
#include "graph.h"

class solver {
public:
    explicit solver(const Graph &graph, size_t crossing_number = 0)
            : graph(graph), crossing_number(crossing_number) {}

    virtual bool solve() = 0;

    std::vector<Vertex> vertex_order;
    size_t crossing_number;
protected:
    const Graph &graph;
};

#endif //OKP_RECOGNITION_SOLVER_H
