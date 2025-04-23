#ifndef OKP_RECOGNITION_GRAPHIO_H
#define OKP_RECOGNITION_GRAPHIO_H

#include <vector>
#include "graph.h"

void save_dot(const std::string& file_name, graph_t& graph,
              const std::vector<vertex_t>& ordering,
              int number_of_crossings,
              const boost::dynamic_properties& graph_props);

int count_edge_crossing(const edge_t& edge, const graph_t& graph);

#endif //OKP_RECOGNITION_GRAPHIO_H
