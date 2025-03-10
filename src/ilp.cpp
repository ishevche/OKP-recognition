#include <iostream>
#include <vector>
#include <algorithm>
#include <gurobi_c++.h>
#include "ilp_solver.h"

bool ilp_solver::solve() {
    vertex_order.clear();
    vertex_order.insert(vertex_order.end(), boost::vertices(graph).first, boost::vertices(graph).second);
    if (vertex_order.size() <= 3) {
        crossing_number = 0;
        return true;
    }

    GRBModel model = GRBModel(*gurobi_env);

    unsigned long num_vertices = boost::num_vertices(graph);
    // Order variables initialization
    order_variables = std::vector<std::vector<GRBLinExpr>>(num_vertices);
    for (int i = 0; i < num_vertices; ++i) {
        order_variables[i].resize(num_vertices, 0);
    }
    for (int i = 0; i < num_vertices; ++i) {
        for (int j = 0; j < num_vertices; ++j) {
            if (i == j) {
                continue;
            } else if (order_variables[j][i].size() == 0) {
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

    // Transitivity constraints
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

    size_t num_edges = boost::num_edges(graph);
    GRBLinExpr crossing_upper_bound = model.addVar(0, GRB_INFINITY, 0, GRB_INTEGER, "k");
    std::vector<GRBLinExpr> edge_crossing_numbers(num_edges);

    // Edge crossings variables definition
    for (Edge edge1: boost::make_iterator_range(boost::edges(graph))) {
        size_t u = boost::get(vertex_index_map, boost::source(edge1, graph));
        size_t v = boost::get(vertex_index_map, boost::target(edge1, graph));
        for (Edge edge2: boost::make_iterator_range(boost::edges(graph))) {
            if (boost::get(edge_index_map, edge1) >= boost::get(edge_index_map, edge2)) {
                continue;
            }
            size_t s = boost::get(vertex_index_map, boost::source(edge2, graph));
            size_t t = boost::get(vertex_index_map, boost::target(edge2, graph));

            GRBVar do_cross = model.addVar(0, 1, 0, GRB_BINARY, "");

            edge_crossing_numbers[boost::get(edge_index_map, edge1)] += do_cross;
            edge_crossing_numbers[boost::get(edge_index_map, edge2)] += do_cross;

            {
                model.addConstr(do_cross >= order_variables[u][s] + order_variables[s][v] + order_variables[v][t] - 2);
                model.addConstr(do_cross >= order_variables[u][t] + order_variables[t][v] + order_variables[v][s] - 2);
                model.addConstr(do_cross >= order_variables[v][s] + order_variables[s][u] + order_variables[u][t] - 2);
                model.addConstr(do_cross >= order_variables[v][t] + order_variables[t][u] + order_variables[u][s] - 2);
                model.addConstr(do_cross >= order_variables[s][u] + order_variables[u][t] + order_variables[t][v] - 2);
                model.addConstr(do_cross >= order_variables[t][u] + order_variables[u][s] + order_variables[s][v] - 2);
                model.addConstr(do_cross >= order_variables[s][v] + order_variables[v][t] + order_variables[t][u] - 2);
                model.addConstr(do_cross >= order_variables[t][v] + order_variables[v][s] + order_variables[s][u] - 2);
            }
        }
        model.addConstr(crossing_upper_bound >=
                        edge_crossing_numbers[boost::get(edge_index_map, edge1)]);
    }

#ifndef ILP_CROSSING_SUM_OPTIMIZATION
    model.setObjective(crossing_upper_bound, GRB_MINIMIZE);
#else
    GRBLinExpr crossing_sum = 0;
    for (const GRBLinExpr &edge_sum: edge_crossing_numbers) {
        crossing_sum += edge_sum;
    }
    size_t factor = num_edges * num_edges;
    model.setObjective(crossing_upper_bound + crossing_sum / factor, GRB_MINIMIZE);
#endif

    model.optimize();

    std::sort(vertex_order.begin(), vertex_order.end(),
              [this](Vertex u, Vertex v) {
                  size_t u_index = boost::get(vertex_index_map, u);
                  size_t v_index = boost::get(vertex_index_map, v);
                  return order_variables[u_index][v_index].getValue();
              });

    crossing_number = crossing_upper_bound.getValue();
    return true;
}



