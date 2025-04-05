#ifndef OKP_RECOGNITION_GRAPH_H
#define OKP_RECOGNITION_GRAPH_H

#include <string>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list.hpp>

struct VertexStruct {
    std::string name;
    std::string location;
    size_t order;
};

struct EdgeStruct {
    std::string color;
};

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
                              VertexStruct, boost::property<boost::edge_index_t, size_t, EdgeStruct>> Graph;
typedef Graph::vertex_descriptor Vertex;
typedef Graph::edge_descriptor Edge;

#endif //OKP_RECOGNITION_GRAPH_H
