#ifndef OKP_RECOGNITION_PROCESS_GRAPH_H
#define OKP_RECOGNITION_PROCESS_GRAPH_H

#include <string>
#include <vector>
#include "graph.h"

void process_graph(const std::string& graph_string);

int count_edge_crossing(Edge edge, const Graph &graph);
int max_local_crossing(const Graph &graph);

void fill_vertex_idx(Graph &graph);
void fill_edge_idx(Graph &graph);


#endif //OKP_RECOGNITION_PROCESS_GRAPH_H
