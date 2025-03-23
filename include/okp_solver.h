#ifndef OKP_RECOGNITION_OKP_SOLVER_H
#define OKP_RECOGNITION_OKP_SOLVER_H


#include "solver.h"

class okp_solver : public solver {
    typedef boost::property_map<Graph, boost::edge_index_t>::const_type edge_index_map_t;
    typedef boost::property_map<Graph, boost::vertex_index_t>::const_type vertex_index_map_t;

    const vertex_index_map_t vertex_index_map;
    const edge_index_map_t edge_index_map;
public:

    explicit okp_solver(const Graph &graph, size_t crossing_number = 0)
            : solver(graph, crossing_number),
              vertex_index_map(boost::get(boost::vertex_index, graph)),
              edge_index_map(boost::get(boost::edge_index, graph)),
              filtered_graph(graph, [&](Edge v) {
                  return std::find(filtered_edges.begin(), filtered_edges.end(), v) == filtered_edges.end();
              }, [&](Vertex v) {
                  return active_link.first != v && active_link.second != v;
              }),
              active_link(Graph::null_vertex(), Graph::null_vertex()) {}

    bool solve() override;

    struct table_entry_t {
        std::vector<std::pair<Edge, int>> edge_order;
        std::vector<Vertex> vertex_order;
    };
private:

    std::vector<std::vector<std::unordered_map<size_t, std::vector<table_entry_t>>>> dp_table;
    std::vector<std::vector<std::unordered_map<size_t, std::vector<std::vector<Edge>>>>> dp_table_initialisation;
    typedef boost::filtered_graph<Graph, boost::function<bool(Edge)>, boost::function<bool(Vertex)>> filtered_graph_t;
    filtered_graph_t filtered_graph;
    std::pair<Vertex, Vertex> active_link;
    std::vector<Edge> filtered_edges;

    bool is_biconnected();

    static bool is_drawable();

    void add_table_entries(size_t k);

    void initialise_table();

    void select_edges(Graph::edge_iterator start, Graph::edge_iterator end, size_t k);

    void populate_right_sides(std::vector<std::pair<size_t, size_t>>::iterator start, std::vector<std::pair<size_t, size_t>>::iterator end, size_t cur_side);
    void fill_right_sides();
};


#endif //OKP_RECOGNITION_OKP_SOLVER_H
