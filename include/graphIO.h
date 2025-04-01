#ifndef OKP_RECOGNITION_GRAPHIO_H
#define OKP_RECOGNITION_GRAPHIO_H

#include <iostream>
#include <vector>
#include "graph.h"

void save_dot(const std::string &file_name, Graph &graph,
              const std::vector<Vertex> &ordering,
              int number_of_crossings,
              const boost::dynamic_properties &graph_props);

int count_edge_crossing(Edge edge, const Graph &graph);

std::string graph_to_g6(const Graph &g);

#endif //OKP_RECOGNITION_GRAPHIO_H
