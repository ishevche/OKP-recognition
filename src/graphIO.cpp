#include "graphIO.h"
#include "process_graph.h"

void save_dot(const std::string &file_name, Graph &graph,
              const std::vector<Vertex> &ordering,
              int number_of_crossings,
              const boost::dynamic_properties &graph_props) {
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
