#ifndef OKP_RECOGNITION_GRAPH_H
#define OKP_RECOGNITION_GRAPH_H

#include <string>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list.hpp>

struct VertexStruct {
    std::string name;
    std::string location;
    size_t order;
    size_t idx;
};
struct EdgeStruct {
    std::string color;
    size_t idx;
};
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, VertexStruct, EdgeStruct> Graph;
typedef Graph::vertex_descriptor Vertex;
typedef Graph::edge_descriptor Edge;

#endif //OKP_RECOGNITION_GRAPH_H
