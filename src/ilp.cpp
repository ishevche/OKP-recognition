#include <iostream>
#include <vector>
#include <algorithm>
#include <gurobi_c++.h>
#include <ogdf/fileformats/GraphIO.h>
#include "solvers.h"


void ILPSolver(SolverParams &solverParams) {
    if (solverParams.graph.numberOfNodes() <= 3) {
        for (ogdf::node node: solverParams.graph.nodes) {
            solverParams.ordering.push_back(node);
        }
        return;
    }

    GRBEnv env = GRBEnv(true);
    env.set("LogFile", "test.log");
    env.set("OutputFlag", "0");
    env.start();

    GRBModel model = GRBModel(env);

    const ogdf::Graph &graph = solverParams.graph;

    // Order variables initialization
    ogdf::NodeArray<ogdf::NodeArray<GRBLinExpr>> variables(graph);
    for (const ogdf::node &node: graph.nodes) {
        variables[node].init(graph, 0);
    }
    for (const ogdf::node &start: graph.nodes) {
        for (const ogdf::node &end: graph.nodes) {
            if (ogdf::NodeElement::equal(*start, *end)) {
                variables[start][end] = 0;
            } else if (variables[end][start].size() == 0) {
                variables[start][end] = model.addVar(
                        0, 1, 0, GRB_BINARY,
                        "is_" + std::to_string(start->index()) +
                        "_before_" + std::to_string(end->index())
                );
            } else {
                variables[start][end] = 1 - variables[end][start];
            }
        }
    }

    // Transitivity constraints
    for (const ogdf::node &start: graph.nodes) {
        for (const ogdf::node &end: graph.nodes) {
            for (const ogdf::node &middle: graph.nodes) {
                model.addConstr(
                        variables[start][end] >=
                        variables[start][middle] +
                        variables[middle][end] - 1
                );
            }
        }
    }

    GRBLinExpr crossing_upper_bound = model.addVar(0, GRB_INFINITY, 0, GRB_INTEGER, "k");
    ogdf::EdgeArray<GRBLinExpr> edge_crossing_numbers(graph, 0);

    // Edge crossings variables definition
    for (const ogdf::edge &edge1: graph.edges) {
        ogdf::node u = edge1->source();
        ogdf::node v = edge1->target();
        for (const ogdf::edge &edge2: graph.edges) {
            if (ogdf::EdgeElement::geq(*edge1, *edge2)) continue;
            ogdf::node s = edge2->source();
            ogdf::node t = edge2->target();

            GRBVar do_cross = model.addVar(
                    0, 1, 0, GRB_BINARY,
                    "does_e" + std::to_string(edge1->index()) +
                    "_cross_e" + std::to_string(edge2->index())
            );

            edge_crossing_numbers[edge1] += do_cross;
            edge_crossing_numbers[edge2] += do_cross;

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
                        edge_crossing_numbers[edge1]);
    }

#ifndef ILP_CROSSING_SUM_OPTIMIZATION
    model.setObjective(crossing_upper_bound, GRB_MINIMIZE);
#else
    GRBLinExpr crossing_sum = 0;
    for (const GRBLinExpr &edge_sum: edge_crossing_numbers) {
        crossing_sum += edge_sum;
    }
    int factor = graph.numberOfEdges() * graph.numberOfEdges();
    model.setObjective(crossing_upper_bound + crossing_sum / factor, GRB_MINIMIZE);
#endif

    model.optimize();
    solverParams.converged = true;

    std::vector<ogdf::node> &ordering = solverParams.ordering;
    ordering.clear();
    ordering.reserve(graph.numberOfNodes());
    for (const ogdf::node &node: graph.nodes) {
        ordering.push_back(node);
    }
    std::sort(ordering.begin(), ordering.end(), [&variables](const ogdf::node &u, const ogdf::node &v) {
        return variables[u][v].getValue();
    });

    solverParams.number_of_crossings = crossing_upper_bound.getValue();
}


