#include <iostream>
#include <vector>
#include <algorithm>
#include <gurobi_c++.h>
#include "ilp_solver.h"

bool ilp_solver::solve() {
    vertex_order.clear();
    vertex_order.insert(vertex_order.end(), vertices(graph).first, vertices(graph).second);
    if (vertex_order.size() <= 3) {
        crossing_number = 0;
        return true;
    }

    GRBModel model = GRBModel(*gurobi_env);
    initialise_model(model);
    model.optimize();

    std::ranges::sort(vertex_order,
                      [this](vertex_t u, vertex_t v) {
                          int u_index = static_cast<int>(get(vertex_index_map, u));
                          int v_index = static_cast<int>(get(vertex_index_map, v));
                          return order_variables[u_index][v_index].getValue();
                      });

    crossing_number = static_cast<int>(crossing_upper_bound.getValue());
    return true;
}

void ilp_solver::initialise_model(GRBModel& model) {
    setup_order_variables(model);
    transitivity_constraints(model);

    crossing_upper_bound = model.addVar(0, GRB_INFINITY, 0, GRB_INTEGER, "k");
    setup_crossing_variables(model);

#ifndef ILP_CROSSING_SUM_OPTIMIZATION
    model.setObjective(crossing_upper_bound, GRB_MINIMIZE);
#else
    GRBLinExpr crossing_sum = 0;
    for (const GRBLinExpr& edge_sum : edge_crossing_numbers) {
        crossing_sum += edge_sum;
    }
    int factor = boost::num_edges(graph) * boost::num_edges(graph);
    model.setObjective(crossing_upper_bound + crossing_sum / factor, GRB_MINIMIZE);
#endif
}

void ilp_solver::setup_order_variables(GRBModel model) {
    int num_vertices = static_cast<int>(boost::num_vertices(graph));
    order_variables = std::vector<std::vector<GRBLinExpr>>(num_vertices);
    for (int i = 0; i < num_vertices; ++i) {
        order_variables[i].resize(num_vertices, 0);
    }
    for (int i = 0; i < num_vertices; ++i) {
        for (int j = 0; j < num_vertices; ++j) {
            if (i == j) { continue; }
            if (order_variables[j][i].size() == 0) {
                order_variables[i][j] = model.addVar(
                    0, 1, 0, GRB_BINARY,
                    "is_" + std::to_string(i) +
                    "_before_" + std::to_string(j)
                );
            } else {
                order_variables[i][j] = 1 - order_variables[j][i];
            }
        }
    }
}

void ilp_solver::transitivity_constraints(GRBModel model) {
    int num_vertices = static_cast<int>(boost::num_vertices(graph));
    for (int start = 0; start < num_vertices; ++start) {
        for (int middle = 0; middle < num_vertices; ++middle) {
            for (int end = 0; end < num_vertices; ++end) {
                model.addConstr(
                    order_variables[start][end] >=
                    order_variables[start][middle] +
                    order_variables[middle][end] - 1
                );
            }
        }
    }
}

void ilp_solver::setup_crossing_variables(GRBModel& model) {
    edge_crossing_numbers = std::vector<GRBLinExpr>(boost::num_edges(graph));
    for (edge_t edge1 : make_iterator_range(edges(graph))) {
        int u = static_cast<int>(get(vertex_index_map, source(edge1, graph)));
        int v = static_cast<int>(get(vertex_index_map, target(edge1, graph)));
        for (edge_t edge2 : make_iterator_range(edges(graph))) {
            if (get(edge_index_map, edge1) >= get(edge_index_map, edge2)) {
                continue;
            }
            int s = static_cast<int>(get(vertex_index_map, source(edge2, graph)));
            int t = static_cast<int>(get(vertex_index_map, target(edge2, graph)));

            GRBVar do_cross = model.addVar(0, 1, 0, GRB_BINARY, "");

            edge_crossing_numbers[get(edge_index_map, edge1)] += do_cross;
            edge_crossing_numbers[get(edge_index_map, edge2)] += do_cross;

            {
                model.addConstr(do_cross >= order_variables[u][s] + order_variables[s][v] + order_variables[v][t] - 2);
                model.addConstr(do_cross >= order_variables[u][t] + order_variables[t][v] + order_variables[v][s] - 2);
                model.addConstr(do_cross >= order_variables[v][s] + order_variables[s][u] + order_variables[u][t] - 2);
                model.addConstr(do_cross >= order_variables[v][t] + order_variables[t][u] + order_variables[u][s] - 2);
                model.addConstr(do_cross >= order_variables[s][u] + order_variables[u][t] + order_variables[t][v] - 2);
                model.addConstr(do_cross >= order_variables[t][u] + order_variables[u][s] + order_variables[s][v] - 2);
                model.addConstr(do_cross >= order_variables[s][v] + order_variables[v][t] + order_variables[t][u] - 2);
                model.addConstr(do_cross >= order_variables[t][v] + order_variables[v][s] + order_variables[s][u] - 2);
#ifdef ILP_EXACT_CROSS
                model.addConstr(do_cross <= 3 - order_variables[u][v] - order_variables[v][s] - order_variables[s][t]);
                model.addConstr(do_cross <= 3 - order_variables[u][s] - order_variables[s][t] - order_variables[t][v]);
                model.addConstr(do_cross <= 3 - order_variables[s][t] - order_variables[t][u] - order_variables[u][v]);
                model.addConstr(do_cross <= 3 - order_variables[s][u] - order_variables[u][v] - order_variables[v][t]);
                model.addConstr(do_cross <= 3 - order_variables[v][u] - order_variables[u][s] - order_variables[s][t]);
                model.addConstr(do_cross <= 3 - order_variables[v][s] - order_variables[s][t] - order_variables[t][u]);
                model.addConstr(do_cross <= 3 - order_variables[s][t] - order_variables[t][v] - order_variables[v][u]);
                model.addConstr(do_cross <= 3 - order_variables[s][v] - order_variables[v][u] - order_variables[u][t]);
                model.addConstr(do_cross <= 3 - order_variables[u][v] - order_variables[v][t] - order_variables[t][s]);
                model.addConstr(do_cross <= 3 - order_variables[u][t] - order_variables[t][s] - order_variables[s][v]);
                model.addConstr(do_cross <= 3 - order_variables[t][s] - order_variables[s][u] - order_variables[u][v]);
                model.addConstr(do_cross <= 3 - order_variables[t][u] - order_variables[u][v] - order_variables[v][s]);
                model.addConstr(do_cross <= 3 - order_variables[v][u] - order_variables[u][t] - order_variables[t][s]);
                model.addConstr(do_cross <= 3 - order_variables[v][t] - order_variables[t][s] - order_variables[s][u]);
                model.addConstr(do_cross <= 3 - order_variables[t][s] - order_variables[s][v] - order_variables[v][u]);
                model.addConstr(do_cross <= 3 - order_variables[t][v] - order_variables[v][u] - order_variables[u][s]);
#endif
            }
        }
        model.addConstr(crossing_upper_bound >= edge_crossing_numbers[get(edge_index_map, edge1)]);
    }
}
