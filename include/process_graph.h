#ifndef OKP_RECOGNITION_PROCESS_GRAPH_H
#define OKP_RECOGNITION_PROCESS_GRAPH_H

#include <string>
#include <vector>
#include <ogdf/basic/basic.h>
#include <ogdf/basic/Graph.h>

void process_graph(const std::string& graph_string);

int count_edge_crossing(const ogdf::edge &edge, const ogdf::Graph &graph, const ogdf::NodeArray<int> &node_order);
int max_local_crossing(const ogdf::Graph &graph, const std::vector<ogdf::node> &ordering);


#endif //OKP_RECOGNITION_PROCESS_GRAPH_H
