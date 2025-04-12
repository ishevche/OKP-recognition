#ifndef OKP_RECOGNITION_SOLVER_H
#define OKP_RECOGNITION_SOLVER_H

#include <vector>
#include "graph.h"

class solver {
public:
    explicit solver(const Graph& graph, int crossing_number = 0)
        : crossing_number(crossing_number), graph(graph) {}

    virtual bool solve() = 0;

    virtual ~solver() = default;

    std::vector<Vertex> vertex_order;
    int crossing_number;

protected:
    const Graph& graph;
};

#endif //OKP_RECOGNITION_SOLVER_H
