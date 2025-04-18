#ifndef OKP_RECOGNITION_SAT_SOLVER_H
#define OKP_RECOGNITION_SAT_SOLVER_H

#include <vector>
#include "solver.h"

extern "C" {
#include <kissat.h>
}

class sat_solver : public solver {
    typedef boost::property_map<Graph, boost::edge_index_t>::const_type edge_index_map_t;
    typedef boost::property_map<Graph, boost::vertex_index_t>::const_type vertex_index_map_t;

    const vertex_index_map_t vertex_index_map;
    const edge_index_map_t edge_index_map;
    kissat* kissat_solver;

public:
    explicit sat_solver(const Graph& graph, int crossing_number = 0)
        : solver(graph, crossing_number),
          vertex_index_map(get(boost::vertex_index, graph)),
          edge_index_map(get(boost::edge_index, graph)),
          kissat_solver(nullptr) {}

    bool solve() override;

    bool is_drawable();

    ~sat_solver() override { if (kissat_solver != nullptr) { kissat_release(kissat_solver); } }

private:
    void initialise_solver();

    void add_crossing_clauses(const Edge& edge1, const Edge& edge2, int crossing_var) const;

    std::vector<std::vector<int>> order_variables;
    std::vector<std::vector<int>> crossing_variables;
};

#endif //OKP_RECOGNITION_SAT_SOLVER_H
