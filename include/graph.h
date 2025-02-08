#ifndef OKP_RECOGNITION_GRAPH_H
#define OKP_RECOGNITION_GRAPH_H

#include <string>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list.hpp>

struct Vertex {
    std::string name;
};
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, Vertex> Graph;

#endif //OKP_RECOGNITION_GRAPH_H
