#ifndef OKP_RECOGNITION_PREPROCESS_H
#define OKP_RECOGNITION_PREPROCESS_H

#include <vector>
#include "graph.h"

enum bctree_node_type_t {
    B_NODE, C_NODE
};

struct bctree_vertex_t {
    bctree_node_type_t node_type;
    graph_t bi_component;
    std::vector<vertex_t> original_vertices;
    vertex_t articulation_point;
};

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, bctree_vertex_t> bctree_t;
typedef bctree_t::vertex_descriptor bctree_vertex;

bctree_t decompose(const graph_t& graph);

#endif //OKP_RECOGNITION_PREPROCESS_H
