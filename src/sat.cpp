#include "sat_solver.h"

bool sat_solver::solve() {
    vertex_order.clear();
    vertex_order.insert(vertex_order.end(), boost::vertices(graph).first, boost::vertices(graph).second);
    if (vertex_order.size() <= 3) {
        return true;
    }

    while (crossing_number < 7 && !is_drawable()) {
        crossing_number++;
    }

    if (crossing_number >= 7) {
        return false;
    }

    std::sort(vertex_order.begin(), vertex_order.end(),
              [this](Vertex u, Vertex v) {
                  size_t u_idx = boost::get(vertex_index_map, u);
                  size_t v_idx = boost::get(vertex_index_map, v);
                  return kissat_value(kissat_solver, order_variables[u_idx][v_idx]) > 0;
              });

    return true;
}

bool sat_solver::is_drawable() {
    initialise_kissat();
    int result = kissat_solve(kissat_solver);

    switch (result) {
        case 10:
            return true;
        case 20:
            return false;
        default:
            std::cout << "SAT solver undefined result" << std::endl;
            return false;
    }
}

void sat_solver::initialise_kissat() {
    if (kissat_solver != nullptr) { kissat_release(kissat_solver); }
    unsigned long num_vertices = boost::num_vertices(graph);

    kissat_solver = kissat_init();
    kissat_set_option(kissat_solver, "quiet", 1);

    // First variable is FALSE
    kissat_add(kissat_solver, -1);
    kissat_add(kissat_solver, 0);

    int variable_count = 1;

    // Order variables initialization
    if (order_variables.empty()) {
        order_variables = std::vector<std::vector<int>>(num_vertices);
        for (int i = 0; i < num_vertices; ++i) {
            order_variables[i].resize(num_vertices, 0);
        }

        for (int i = 0; i < num_vertices; ++i) {
            for (int j = 0; j < num_vertices; ++j) {
                if (i == j) {
                    order_variables[i][j] = 1;
                } else if (order_variables[j][i] != 0) {
                    order_variables[i][j] = -order_variables[j][i];
                } else {
                    order_variables[i][j] = ++variable_count;
                }
            }
        }
    }

    // Transitivity constraints
    // (sm && me) -> se   <=>   -sm || -me || se
    for (int start = 0; start < num_vertices; ++start) {
        for (int middle = 0; middle < num_vertices; ++middle) {
            for (int end = 0; end < num_vertices; ++end) {
                kissat_add(kissat_solver, -order_variables[start][middle]);
                kissat_add(kissat_solver, -order_variables[middle][end]);
                kissat_add(kissat_solver, order_variables[start][end]);
                kissat_add(kissat_solver, 0);
            }
        }
    }

    // Edge crossings variables definition
    size_t num_edges = boost::num_edges(graph);
    crossing_variables = std::vector<std::vector<int>>(num_edges);
    for (int i = 0; i < num_edges; ++i) {
        crossing_variables[i].resize(num_edges, 0);
    }

    for (Edge edge1: boost::make_iterator_range(boost::edges(graph))) {
        size_t edge1_idx = boost::get(edge_index_map, edge1);
        for (Edge edge2: boost::make_iterator_range(boost::edges(graph))) {
            size_t edge2_idx = boost::get(edge_index_map, edge2);
            if (edge1_idx == edge2_idx) {
                crossing_variables[edge1_idx][edge2_idx] = 1;
            } else if (crossing_variables[edge2_idx][edge1_idx] != 0) {
                crossing_variables[edge1_idx][edge2_idx] =
                        crossing_variables[edge2_idx][edge1_idx];
            } else {
                crossing_variables[edge1_idx][edge2_idx] = ++variable_count;
            }
        }
    }

    for (auto [e1i, ei_end] = boost::edges(graph); e1i != ei_end; ++e1i) {
        size_t edge1_idx = boost::get(edge_index_map, *e1i);
        for (auto e2i = std::next(e1i); e2i != ei_end; ++e2i) {
            size_t edge2_idx = boost::get(edge_index_map, *e2i);
            add_crossing_clauses(*e1i, *e2i, crossing_variables[edge1_idx][edge2_idx]);
        }
    }

    // Restricting number of crossings for every edge
    for (Edge edge: boost::make_iterator_range(boost::edges(graph))) {
        std::string crossings(crossing_number + 1, 1);
        size_t edge_idx = boost::get(edge_index_map, edge);
        crossings.resize(num_edges, 0);
        do {
            size_t i = 0;
            for (Edge other: boost::make_iterator_range(boost::edges(graph))) {
                if (crossings[i++]) {
                    size_t other_idx = boost::get(edge_index_map, other);
                    kissat_add(kissat_solver, -crossing_variables[edge_idx][other_idx]);
                }
            }
            kissat_add(kissat_solver, 0);
        } while (std::prev_permutation(crossings.begin(), crossings.end()));
    }
}

#define ADD_CLAUSE4(solver, a, b, c, d) {kissat_add(solver, a);kissat_add(solver, b);kissat_add(solver, c);kissat_add(solver, d);kissat_add(solver, 0);}

void sat_solver::add_crossing_clauses(Edge edge1, Edge edge2, int crossing_var) {
    size_t u = boost::get(vertex_index_map, boost::source(edge1, graph));
    size_t v = boost::get(vertex_index_map, boost::target(edge1, graph));
    size_t s = boost::get(vertex_index_map, boost::source(edge2, graph));
    size_t t = boost::get(vertex_index_map, boost::target(edge2, graph));
    ADD_CLAUSE4(kissat_solver, -order_variables[u][s], -order_variables[s][v], -order_variables[v][t], crossing_var)
    ADD_CLAUSE4(kissat_solver, -order_variables[u][t], -order_variables[t][v], -order_variables[v][s], crossing_var)
    ADD_CLAUSE4(kissat_solver, -order_variables[v][s], -order_variables[s][u], -order_variables[u][t], crossing_var)
    ADD_CLAUSE4(kissat_solver, -order_variables[v][t], -order_variables[t][u], -order_variables[u][s], crossing_var)
    ADD_CLAUSE4(kissat_solver, -order_variables[s][u], -order_variables[u][t], -order_variables[t][v], crossing_var)
    ADD_CLAUSE4(kissat_solver, -order_variables[t][u], -order_variables[u][s], -order_variables[s][v], crossing_var)
    ADD_CLAUSE4(kissat_solver, -order_variables[s][v], -order_variables[v][t], -order_variables[t][u], crossing_var)
    ADD_CLAUSE4(kissat_solver, -order_variables[t][v], -order_variables[v][s], -order_variables[s][u], crossing_var)
}

