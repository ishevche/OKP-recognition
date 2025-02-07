#include "solvers.h"

extern "C" {
#include <kissat.h>
}

void add_crossing_clauses(kissat *sat_solver, ogdf::edge edge1, ogdf::edge edge2,
                          const ogdf::NodeArray<ogdf::NodeArray<int>> &order_vars,
                          int crossing_var);

void SATSolver(SolverParams &solverParams) {
    if (solverParams.graph.numberOfNodes() <= 3) {
        for (ogdf::node node: solverParams.graph.nodes) {
            solverParams.ordering.push_back(node);
        }
        return;
    }

    const ogdf::Graph &graph = solverParams.graph;
    int number_of_crossings = solverParams.number_of_crossings;

    if (number_of_crossings >= 7) {
        solverParams.converged = false;
        return;
    }
    kissat *sat_solver = kissat_init();
    kissat_set_option(sat_solver, "quiet", 1);

    // First variable is TRUE
    kissat_add(sat_solver, 1);
    kissat_add(sat_solver, 0);
    // Second variable is FALSE
    kissat_add(sat_solver, -2);
    kissat_add(sat_solver, 0);

    int variable_count = 2;

    // Order variables initialization
    ogdf::NodeArray<ogdf::NodeArray<int>> order_variables(graph);
    for (const ogdf::node &node: graph.nodes) {
        order_variables[node].init(graph, 0);
    }

    for (const ogdf::node &start: graph.nodes) {
        for (const ogdf::node &end: graph.nodes) {
            if (ogdf::NodeElement::equal(*start, *end)) {
                order_variables[start][end] = 2;
            } else if (order_variables[end][start] != 0) {
                order_variables[start][end] = -order_variables[end][start];
            } else {
                order_variables[start][end] = ++variable_count;
            }
        }
    }

    // Transitivity constraints
    // (sm && me) -> se   <=>   -sm || -me || se
    for (const ogdf::node &start: graph.nodes) {
        for (const ogdf::node &end: graph.nodes) {
            for (const ogdf::node &middle: graph.nodes) {
                kissat_add(sat_solver, -order_variables[start][middle]);
                kissat_add(sat_solver, -order_variables[middle][end]);
                kissat_add(sat_solver, order_variables[start][end]);
                kissat_add(sat_solver, 0);
            }
        }
    }

    // Edge crossings variables definition
    ogdf::EdgeArray<ogdf::EdgeArray<int>> crossing_variables(graph);
    for (const ogdf::edge &edge: graph.edges) {
        crossing_variables[edge].init(graph, 0);
    }

    for (const ogdf::edge &edge1: graph.edges) {
        for (const ogdf::edge &edge2: graph.edges) {
            if (ogdf::EdgeElement::equal(*edge1, *edge2)) {
                crossing_variables[edge1][edge2] = 2;
            } else if (crossing_variables[edge2][edge1] != 0) {
                crossing_variables[edge1][edge2] = crossing_variables[edge2][edge1];
            } else {
                int crossing_var = ++variable_count;
                crossing_variables[edge1][edge2] = crossing_var;
                add_crossing_clauses(sat_solver, edge1, edge2, order_variables, crossing_var);
            }
        }
    }

    std::vector<ogdf::edge> edges_vector;
    edges_vector.reserve(graph.numberOfEdges());
    for (ogdf::edge edge: graph.edges) {
        edges_vector.push_back(edge);
    }

    // Restricting number of crossings for every edge
    for (ogdf::edge edge: graph.edges) {
        std::string crossings(number_of_crossings + 1, 1);
        crossings.resize(graph.numberOfEdges(), 0);
        do {
            for (int i = 0; i < graph.numberOfEdges(); ++i) {
                if (crossings[i])
                    kissat_add(sat_solver, -crossing_variables[edge][edges_vector[i]]);
            }
            kissat_add(sat_solver, 0);
        } while (std::prev_permutation(crossings.begin(), crossings.end()));
    }

    int result = kissat_solve(sat_solver);

    if (result == 20) {
        kissat_release(sat_solver);
        solverParams.converged = false;
        return;
    } else if (result != 10) {
        std::cout << "SAT solver undefined result" << std::endl;
        kissat_release(sat_solver);
        solverParams.converged = false;
        return;
    }

    std::vector<ogdf::node> &ordering = solverParams.ordering;
    ordering.clear();
    ordering.reserve(graph.numberOfNodes());
    for (const ogdf::node &node: graph.nodes) {
        ordering.push_back(node);
    }
    std::sort(ordering.begin(), ordering.end(), [&order_variables, &sat_solver](const ogdf::node &u, const ogdf::node &v) {
        return kissat_value(sat_solver, order_variables[u][v]) > 0;
    });

    kissat_release(sat_solver);

    solverParams.converged = true;
}

#define ADD_CLAUSE4(solver, a, b, c, d) {kissat_add(solver, a);kissat_add(solver, b);kissat_add(solver, c);kissat_add(solver, d);kissat_add(solver, 0);}

void add_crossing_clauses(kissat *sat_solver, ogdf::edge edge1, ogdf::edge edge2,
                          const ogdf::NodeArray<ogdf::NodeArray<int>> &order_vars,
                          int crossing_var) {
    ogdf::node u = edge1->source();
    ogdf::node v = edge1->target();
    ogdf::node s = edge2->source();
    ogdf::node t = edge2->target();
    ADD_CLAUSE4(sat_solver, -order_vars[u][s], -order_vars[s][v], -order_vars[v][t], crossing_var)
    ADD_CLAUSE4(sat_solver, -order_vars[u][t], -order_vars[t][v], -order_vars[v][s], crossing_var)
    ADD_CLAUSE4(sat_solver, -order_vars[v][s], -order_vars[s][u], -order_vars[u][t], crossing_var)
    ADD_CLAUSE4(sat_solver, -order_vars[v][t], -order_vars[t][u], -order_vars[u][s], crossing_var)
    ADD_CLAUSE4(sat_solver, -order_vars[s][u], -order_vars[u][t], -order_vars[t][v], crossing_var)
    ADD_CLAUSE4(sat_solver, -order_vars[t][u], -order_vars[u][s], -order_vars[s][v], crossing_var)
    ADD_CLAUSE4(sat_solver, -order_vars[s][v], -order_vars[v][t], -order_vars[t][u], crossing_var)
    ADD_CLAUSE4(sat_solver, -order_vars[t][v], -order_vars[v][s], -order_vars[s][u], crossing_var)
}

