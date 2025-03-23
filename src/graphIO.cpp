#include "graphIO.h"include "process_graph.h"

void save_dot(const std::string& file_name, Graph& graph,
              const std::vector<Vertex>& ordering,
              int number_of_crossings,
              const boost::dynamic_properties& graph_props) {
    std::ofstream out{file_name};
    for (size_t j = 0; j < ordering.size(); ++j) {
        graph[ordering[j]].order = j;
        double x = 100. * std::cos(2. * j * M_PI / boost::num_vertices(graph));
        double y = 100. * std::sin(2. * j * M_PI / boost::num_vertices(graph));
        graph[ordering[j]].location = std::to_string(x) + ','
                                      + std::to_string(y) + '!';
    }
    for (Edge e : boost::make_iterator_range(boost::edges(graph))) {
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
