#ifndef OKP_RECOGNITION_SOLVER_H
#define OKP_RECOGNITION_SOLVER_H

#include <vector>
#include "graph.h"

class abstract_solver {
public:
    explicit abstract_solver(const graph_t& graph, int crossing_number = 0)
        : crossing_number(crossing_number), graph(graph) {}

    virtual bool solve() = 0;

    virtual ~abstract_solver() = default;

    std::vector<vertex_t> vertex_order;
    int crossing_number;

protected:
    const graph_t& graph;
};

#endif //OKP_RECOGNITION_SOLVER_H
