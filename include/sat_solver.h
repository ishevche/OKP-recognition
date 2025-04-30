#ifndef OKP_RECOGNITION_SAT_SOLVER_H
#define OKP_RECOGNITION_SAT_SOLVER_H

#include <vector>
#include "abstract_solver.h"

extern "C" {
#include <kissat.h>
}

class sat_solver : public abstract_solver {
    typedef boost::property_map<graph_t, boost::edge_index_t>::const_type edge_index_map_t;
    typedef boost::property_map<graph_t, boost::vertex_index_t>::const_type vertex_index_map_t;

    const vertex_index_map_t vertex_index_map;
    const edge_index_map_t edge_index_map;
    kissat* kissat_solver;

public:
    explicit sat_solver(const graph_t& graph, int crossing_number = 0)
        : abstract_solver(graph, crossing_number),
          vertex_index_map(get(boost::vertex_index, graph)),
          edge_index_map(get(boost::edge_index, graph)),
          kissat_solver(nullptr) {}

    bool solve() override;

    bool is_drawable();

    ~sat_solver() override { if (kissat_solver != nullptr) { kissat_release(kissat_solver); } }

private:
    void initialise_solver();

    void add_crossing_clauses(const edge_t& edge1, const edge_t& edge2, int crossing_var) const;

    std::vector<std::vector<int>> order_variables;
    std::vector<std::vector<int>> crossing_variables;
};

#endif //OKP_RECOGNITION_SAT_SOLVER_H
