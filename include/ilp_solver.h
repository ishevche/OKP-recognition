#ifndef OKP_RECOGNITION_ILP_SOLVER_H
#define OKP_RECOGNITION_ILP_SOLVER_H

#include <vector>
#include <gurobi_c++.h>
#include "abstract_solver.h"


class ilp_solver : public abstract_solver {
    typedef boost::property_map<graph_t, boost::edge_index_t>::const_type edge_index_map_t;
    typedef boost::property_map<graph_t, boost::vertex_index_t>::const_type vertex_index_map_t;

    const vertex_index_map_t vertex_index_map;
    const edge_index_map_t edge_index_map;

public:
    explicit ilp_solver(const graph_t& graph, int crossing_number = 0)
        : abstract_solver(graph, crossing_number),
          vertex_index_map(get(boost::vertex_index, graph)),
          edge_index_map(get(boost::edge_index, graph)) {
        if (!gurobi_env) {
            gurobi_env = std::make_unique<GRBEnv>(true);
            gurobi_env->set(GRB_IntParam_OutputFlag, 0);
            gurobi_env->set(GRB_IntParam_Threads, 1);
            gurobi_env->start();
        }
    }

    bool solve() override;

private:
    void initialise_model(GRBModel& model);
    void setup_order_variables(GRBModel model);
    void transitivity_constraints(GRBModel model);
    void setup_crossing_variables(GRBModel& model);

    std::vector<GRBLinExpr> edge_crossing_numbers;
    std::vector<std::vector<GRBLinExpr>> order_variables;
    GRBLinExpr crossing_upper_bound;
    static inline std::unique_ptr<GRBEnv> gurobi_env = nullptr;
};

#endif //OKP_RECOGNITION_ILP_SOLVER_H
