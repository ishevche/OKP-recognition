#include "graphIO.h"

void save_dot(const std::string& file_name, Graph& graph,
              const std::vector<Vertex>& ordering,
              size_t number_of_crossings,
              const boost::dynamic_properties& graph_props) {
    std::ofstream out{file_name};
    for (int j = 0; j < ordering.size(); ++j) {
        graph[ordering[j]].order = j;
        double x = 5. * std::cos(2. * j * M_PI / static_cast<int>(num_vertices(graph)));
        double y = 5. * std::sin(2. * j * M_PI / static_cast<int>(num_vertices(graph)));
        graph[ordering[j]].location = std::to_string(x) + ','
                                      + std::to_string(y) + '!';
    }
    for (Edge e : make_iterator_range(edges(graph))) {
        graph[e].color = "";
        int crossing = count_edge_crossing(e, graph);
        if (crossing == number_of_crossings) {
            graph[e].color = "blue";
        } else if (crossing > number_of_crossings) {
            graph[e].color = "red";
        }
    }
    std::ofstream dot_file(file_name);
    write_graphviz_dp(dot_file, graph, graph_props);
    dot_file.close();
}

int count_edge_crossing(const Edge& edge, const Graph& graph) {
    int result = 0;
    size_t edge_src = graph[source(edge, graph)].order;
    size_t edge_trg = graph[target(edge, graph)].order;
    if (edge_src > edge_trg) { std::swap(edge_src, edge_trg); }
    for (Edge other : make_iterator_range(edges(graph))) {
        size_t other_src = graph[source(other, graph)].order;
        size_t other_trg = graph[target(other, graph)].order;
        if (other_src > other_trg) { std::swap(other_src, other_trg); }
        bool src_between = edge_src < other_src && other_src < edge_trg;
        bool trg_between = edge_src < other_trg && other_trg < edge_trg;
        result += (src_between && other_trg > edge_trg) ||
            (trg_between && other_src < edge_src);
    }
    return result;
}
