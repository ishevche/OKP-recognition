#include "graphIO.h"

void save_dot(const std::string& file_name, Graph& graph,
              const std::vector<Vertex>& ordering,
              int number_of_crossings,
              const boost::dynamic_properties& graph_props) {
    std::ofstream out{file_name};
    for (size_t j = 0; j < ordering.size(); ++j) {
        graph[ordering[j]].order = j;
        double x = 5. * std::cos(2. * j * M_PI / boost::num_vertices(graph));
        double y = 5. * std::sin(2. * j * M_PI / boost::num_vertices(graph));
        graph[ordering[j]].location = std::to_string(x) + ','
                                      + std::to_string(y) + '!';
    }
    for (Edge e : boost::make_iterator_range(boost::edges(graph))) {
        graph[e].color = "";
        int crossing = count_edge_crossing(e, graph);
        if (crossing == number_of_crossings) {
            graph[e].color = "blue";
        } else if (crossing > number_of_crossings) {
            graph[e].color = "red";
        }
    }
    std::ofstream dot_file(file_name);
    boost::write_graphviz_dp(dot_file, graph, graph_props);
    dot_file.close();
}

int count_edge_crossing(Edge edge, const Graph& graph) {
    int result = 0;
    int edge_src = graph[boost::source(edge, graph)].order;
    int edge_trg = graph[boost::target(edge, graph)].order;
    if (edge_src > edge_trg) { std::swap(edge_src, edge_trg); }
    for (Edge other : boost::make_iterator_range(boost::edges(graph))) {
        int other_src = graph[boost::source(other, graph)].order;
        int other_trg = graph[boost::target(other, graph)].order;
        if (other_src > other_trg) { std::swap(other_src, other_trg); }
        bool src_between = edge_src < other_src && other_src < edge_trg;
        bool trg_between = edge_src < other_trg && other_trg < edge_trg;
        result += (src_between && other_trg > edge_trg) ||
            (trg_between && other_src < edge_src);
    }
    return result;
}

char encode_base64(int value) {
    return value + 63;
}

std::string graph_to_g6(const Graph &g) {
    size_t n = boost::num_vertices(g);
    std::ostringstream g6;

    if (n <= 62) {
        g6 << encode_base64(n);
    } else {
        throw std::runtime_error("Graph6 extended format (n > 62) not implemented");
    }

    std::vector<bool> adjacency_bits;
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < i; ++j) {
            if (boost::edge(i, j, g).second) {
                adjacency_bits.push_back(1);
            } else {
                adjacency_bits.push_back(0);
            }
        }
    }

    for (size_t i = 0; i < adjacency_bits.size(); i += 6) {
        int chunk_value = 0;
        for (size_t j = 0; j < 6; ++j) {
            if (i + j < adjacency_bits.size() && adjacency_bits[i + j]) {
                chunk_value |= (1 << (5 - j));
            }
        }
        g6 << encode_base64(chunk_value);
    }

    return g6.str();
}

// 2453160