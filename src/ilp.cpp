#include <iostream>
#include <vector>
#include <algorithm>
#include <gurobi_c++.h>
#include "solvers.h"
#include "process_graph.h"

void ILPSolver(SolverParams &solverParams) {
    Graph &graph = solverParams.graph;
    std::vector<Vertex> &ordering = solverParams.ordering;

    unsigned long num_vertices = boost::num_vertices(graph);
    ordering.clear();

    if (num_vertices <= 3) {
        for (Vertex node: boost::make_iterator_range(boost::vertices(graph))) {
            ordering.push_back(node);
        }
        solverParams.number_of_crossings = 0;
        solverParams.converged = true;
        return;
    }

    GRBEnv env = GRBEnv(true);
    env.set("LogFile", "test.log");
    env.set("OutputFlag", "0");
    env.start();

    GRBModel model = GRBModel(env);

    // Order variables initialization
    std::vector<std::vector<GRBLinExpr>> variables(num_vertices);
    for (int i = 0; i < num_vertices; ++i) {
        variables[i].resize(num_vertices, 0);
    }
    for (int i = 0; i < num_vertices; ++i) {
        for (int j = 0; j < num_vertices; ++j) {
            if (i == j) {
                continue;
            } else if (variables[j][i].size() == 0) {
                variables[i][j] = model.addVar(
                        0, 1, 0, GRB_BINARY,
                        "is_" + std::to_string(i) +
                        "_before_" + std::to_string(j)
                );
            } else {
                variables[i][j] = 1 - variables[j][i];
            }
        }
    }

    // Transitivity constraints
    for (int start = 0; start < num_vertices; ++start) {
        for (int middle = 0; middle < num_vertices; ++middle) {
            for (int end = 0; end < num_vertices; ++end) {
                model.addConstr(
                        variables[start][end] >=
                        variables[start][middle] +
                        variables[middle][end] - 1
                );
            }
        }
    }

    size_t num_edges = boost::num_edges(graph);
    GRBLinExpr crossing_upper_bound = model.addVar(0, GRB_INFINITY, 0, GRB_INTEGER, "k");
    fill_vertex_idx(graph);
    fill_edge_idx(graph);
    std::vector<GRBLinExpr> edge_crossing_numbers(num_edges);

    // Edge crossings variables definition
    for (Edge edge1: boost::make_iterator_range(boost::edges(graph))) {
        size_t u = graph[boost::source(edge1, graph)].idx;
        size_t v = graph[boost::target(edge1, graph)].idx;
        for (Edge edge2: boost::make_iterator_range(boost::edges(graph))) {
            if (graph[edge1].idx >= graph[edge2].idx) continue;
            size_t s = graph[boost::source(edge2, graph)].idx;
            size_t t = graph[boost::target(edge2, graph)].idx;

            GRBVar do_cross = model.addVar(0, 1, 0, GRB_BINARY, "");

            edge_crossing_numbers[graph[edge1].idx] += do_cross;
            edge_crossing_numbers[graph[edge2].idx] += do_cross;

            {
                model.addConstr(do_cross >= variables[u][s] + variables[s][v] + variables[v][t] - 2);
                model.addConstr(do_cross >= variables[u][t] + variables[t][v] + variables[v][s] - 2);
                model.addConstr(do_cross >= variables[v][s] + variables[s][u] + variables[u][t] - 2);
                model.addConstr(do_cross >= variables[v][t] + variables[t][u] + variables[u][s] - 2);
                model.addConstr(do_cross >= variables[s][u] + variables[u][t] + variables[t][v] - 2);
                model.addConstr(do_cross >= variables[t][u] + variables[u][s] + variables[s][v] - 2);
                model.addConstr(do_cross >= variables[s][v] + variables[v][t] + variables[t][u] - 2);
                model.addConstr(do_cross >= variables[t][v] + variables[v][s] + variables[s][u] - 2);
            }
        }
        model.addConstr(crossing_upper_bound >=
                        edge_crossing_numbers[graph[edge1].idx]);
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
    solverParams.converged = true;
    ordering.insert(ordering.end(), boost::vertices(graph).first, boost::vertices(graph).second);

    std::sort(ordering.begin(), ordering.end(),
              [&variables, &graph](Vertex u, Vertex v) {
                  return variables[graph[u].idx][graph[v].idx].getValue();
              });

    solverParams.number_of_crossings = crossing_upper_bound.getValue();
}



