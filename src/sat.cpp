#include "solvers.h"
#include "process_graph.h"

extern "C" {
#include <kissat.h>
}

void add_crossing_clauses(kissat *sat_solver, Edge edge1, Edge edge2, const Graph &graph,
                          const std::vector<std::vector<int>> &order_vars,
                          int crossing_var);

void SATSolver(SolverParams &solverParams) {
    Graph &graph = solverParams.graph;
    std::vector<Vertex> &ordering = solverParams.ordering;
    int number_of_crossings = solverParams.number_of_crossings;

    unsigned long num_vertices = boost::num_vertices(graph);
    ordering.clear();

    if (num_vertices <= 3) {
        for (Vertex node: boost::make_iterator_range(boost::vertices(graph))) {
            ordering.push_back(node);
        }
        solverParams.converged = true;
        return;
    }

    if (number_of_crossings >= 7) {
        solverParams.converged = false;
        return;
    }
    kissat *sat_solver = kissat_init();
    kissat_set_option(sat_solver, "quiet", 1);

    // First variable is FALSE
    kissat_add(sat_solver, -1);
    kissat_add(sat_solver, 0);

    int variable_count = 1;

    // Order variables initialization
    std::vector<std::vector<int>> order_variables(num_vertices);
    for (int i = 0; i < num_vertices; ++i) {
        order_variables[i].resize(num_vertices, 0);
    }

    for (int i = 0; i < num_vertices; ++i) {
        for (int j = 0; j < num_vertices; ++j) {
            if (i == j) {
                order_variables[i][j] = 2;
            } else if (order_variables[j][i] != 0) {
                order_variables[i][j] = -order_variables[j][i];
            } else {
                order_variables[i][j] = ++variable_count;
            }
        }
    }

    // Transitivity constraints
    // (sm && me) -> se   <=>   -sm || -me || se
    for (int start = 0; start < num_vertices; ++start) {
        for (int middle = 0; middle < num_vertices; ++middle) {
            for (int end = 0; end < num_vertices; ++end) {
                kissat_add(sat_solver, -order_variables[start][middle]);
                kissat_add(sat_solver, -order_variables[middle][end]);
                kissat_add(sat_solver, order_variables[start][end]);
                kissat_add(sat_solver, 0);
            }
        }
    }

    // Edge crossings variables definition
    size_t num_edges = boost::num_edges(graph);
    fill_vertex_idx(graph);
    fill_edge_idx(graph);
    std::vector<std::vector<int>> crossing_variables(num_edges);
    for (int i = 0; i < num_edges; ++i) {
        crossing_variables[i].resize(num_edges, 0);
    }

    for (Edge edge1: boost::make_iterator_range(boost::edges(graph))) {
        size_t edge1_idx = graph[edge1].idx;
        for (Edge edge2: boost::make_iterator_range(boost::edges(graph))) {
            size_t edge2_idx = graph[edge2].idx;
            if (edge1_idx == edge2_idx) {
                crossing_variables[edge1_idx][edge2_idx] = 2;
            } else if (crossing_variables[edge2_idx][edge1_idx] != 0) {
                crossing_variables[edge1_idx][edge2_idx] =
                        crossing_variables[edge2_idx][edge1_idx];
            } else {
                int crossing_var = ++variable_count;
                crossing_variables[edge1_idx][edge2_idx] = crossing_var;
                add_crossing_clauses(sat_solver, edge1, edge2, graph, order_variables, crossing_var);
            }
        }
    }

    // Restricting number of crossings for every edge
    for (Edge edge: boost::make_iterator_range(boost::edges(graph))) {
        std::string crossings(number_of_crossings + 1, 1);
        size_t edge_idx = graph[edge].idx;
        crossings.resize(num_edges, 0);
        do {
            size_t i = 0;
            for (Edge other: boost::make_iterator_range(boost::edges(graph))) {
                if (crossings[i++]) {
                    size_t other_idx = graph[other].idx;
                    kissat_add(sat_solver,
                               -crossing_variables[edge_idx][other_idx]);
                }
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

    ordering.clear();
    ordering.insert(ordering.end(), boost::vertices(graph).first, boost::vertices(graph).second);
    std::sort(ordering.begin(), ordering.end(),
              [&order_variables, &sat_solver, &graph](Vertex u, Vertex v) {
                  return kissat_value(sat_solver, order_variables[graph[u].idx][graph[v].idx]) > 0;
              });

    kissat_release(sat_solver);

    solverParams.converged = true;
}

#define ADD_CLAUSE4(solver, a, b, c, d) {kissat_add(solver, a);kissat_add(solver, b);kissat_add(solver, c);kissat_add(solver, d);kissat_add(solver, 0);}

void add_crossing_clauses(kissat *sat_solver, Edge edge1, Edge edge2, const Graph &graph,
                          const std::vector<std::vector<int>> &order_vars,
                          int crossing_var) {
    size_t u = graph[boost::source(edge1, graph)].idx;
    size_t v = graph[boost::target(edge1, graph)].idx;
    size_t s = graph[boost::source(edge2, graph)].idx;
    size_t t = graph[boost::target(edge2, graph)].idx;
    ADD_CLAUSE4(sat_solver, -order_vars[u][s], -order_vars[s][v], -order_vars[v][t], crossing_var)
    ADD_CLAUSE4(sat_solver, -order_vars[u][t], -order_vars[t][v], -order_vars[v][s], crossing_var)
    ADD_CLAUSE4(sat_solver, -order_vars[v][s], -order_vars[s][u], -order_vars[u][t], crossing_var)
    ADD_CLAUSE4(sat_solver, -order_vars[v][t], -order_vars[t][u], -order_vars[u][s], crossing_var)
    ADD_CLAUSE4(sat_solver, -order_vars[s][u], -order_vars[u][t], -order_vars[t][v], crossing_var)
    ADD_CLAUSE4(sat_solver, -order_vars[t][u], -order_vars[u][s], -order_vars[s][v], crossing_var)
    ADD_CLAUSE4(sat_solver, -order_vars[s][v], -order_vars[v][t], -order_vars[t][u], crossing_var)
    ADD_CLAUSE4(sat_solver, -order_vars[t][v], -order_vars[v][s], -order_vars[s][u], crossing_var)
}

