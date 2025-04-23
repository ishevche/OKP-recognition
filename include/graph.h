#ifndef OKP_RECOGNITION_GRAPH_H
#define OKP_RECOGNITION_GRAPH_H

#include <string>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list.hpp>

struct vertex_struct {
    std::string name;
    std::string location;
    int order;
};

struct edge_struct {
    std::string color;
};

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
                              vertex_struct, boost::property<boost::edge_index_t, int, edge_struct>> graph_t;
typedef graph_t::vertex_descriptor vertex_t;
typedef graph_t::edge_descriptor edge_t;

#endif //OKP_RECOGNITION_GRAPH_H
