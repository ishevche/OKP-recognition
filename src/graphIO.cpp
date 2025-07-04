#include "graphIO.h"

void save_dot(const std::string& file_name, graph_t& graph,
              const std::vector<vertex_t>& ordering,
              int number_of_crossings,
              const boost::dynamic_properties& graph_props) {
    std::ofstream out{file_name};
    int vertices_count = static_cast<int>(num_vertices(graph));
    for (int j = 0; j < static_cast<int>(ordering.size()); ++j) {
        graph[ordering[j]].order = j;
        double x = 2. * std::cos(2. * j * M_PI / vertices_count);
        double y = 2. * std::sin(2. * j * M_PI / vertices_count);
        graph[ordering[j]].location = std::to_string(x) + ','
                                      + std::to_string(y) + '!';
    }
    for (edge_t e : make_iterator_range(edges(graph))) {
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

int count_edge_crossing(const edge_t& edge, const graph_t& graph) {
    int result = 0;
    int edge_src = graph[source(edge, graph)].order;
    int edge_trg = graph[target(edge, graph)].order;
    if (edge_src > edge_trg) { std::swap(edge_src, edge_trg); }
    for (edge_t other : make_iterator_range(edges(graph))) {
        int other_src = graph[source(other, graph)].order;
        int other_trg = graph[target(other, graph)].order;
        if (other_src > other_trg) { std::swap(other_src, other_trg); }
        bool src_between = edge_src < other_src && other_src < edge_trg;
        bool trg_between = edge_src < other_trg && other_trg < edge_trg;
        result += (src_between && other_trg > edge_trg) ||
            (trg_between && other_src < edge_src);
    }
    return result;
}
