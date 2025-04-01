#ifndef OKP_RECOGNITION_OKP_SOLVER_H
#define OKP_RECOGNITION_OKP_SOLVER_H


#include "solver.h"

class okp_solver : public solver {
    typedef boost::property_map<Graph, boost::edge_index_t>::const_type edge_index_map_t;
    typedef boost::property_map<Graph, boost::vertex_index_t>::const_type vertex_index_map_t;

    const vertex_index_map_t vertex_index_map;
    const edge_index_map_t edge_index_map;

public:
    explicit okp_solver(const Graph& graph, size_t crossing_number = 0)
        : solver(graph, crossing_number),
          vertex_index_map(boost::get(boost::vertex_index, graph)),
          edge_index_map(boost::get(boost::edge_index, graph)),
          filtered_graph(graph, [&](Edge v) {
                             return std::find(filtered_edges.begin(), filtered_edges.end(), v) == filtered_edges.end();
                         }, [&](Vertex v) {
                             return active_link.first != boost::get(vertex_index_map, v)
                                    && active_link.second != boost::get(vertex_index_map, v);
                         }),
          active_link(Graph::null_vertex(), Graph::null_vertex()) {}

    bool solve() override;

    struct table_entry_t {
        std::vector<std::pair<int, int>> edge_order;
        std::vector<Vertex> vertex_order;
    };

private:
    std::vector<std::vector<std::unordered_map<size_t, std::vector<table_entry_t>>>> dp_table;
    std::vector<std::vector<std::vector<std::vector<std::pair<size_t, std::vector<Edge>>>>>> dp_table_initialisation;
    typedef boost::filtered_graph<Graph, boost::function<bool(Edge)>, boost::function<bool(Vertex)>> filtered_graph_t;
    filtered_graph_t filtered_graph;
    std::pair<size_t, size_t> active_link;
    std::vector<Edge> filtered_edges;

    bool is_biconnected() const;
    void insert_triangle_edges(std::vector<size_t>& triangle_vertex_drawing,
                                           const std::vector<Edge>& ordered_edges,
                                           size_t part_mask, size_t starting_index);
    bool get_next_permutation(table_entry_t& combined_arrangement);
    std::vector<Edge> get_ordered_edges(const std::vector<Edge>& edges,
                                        const std::vector<std::pair<int, int>>& edge_order);
    bool verify_triangle_drawing(const std::vector<size_t>& triangle_vertex_drawing,
                                 const std::vector<Edge>& ordered_edges, size_t part_mask) const;
    bool count_triangle_intersections(const std::vector<Edge>& part_a_edges,
                                      const std::vector<Edge>& part_b_edges,
                                      const std::vector<Edge>& piercing_edges,
                                      size_t split_vertex,
                                      std::vector<size_t>& intersections_count,
                                      const std::vector<size_t>& triangle_drawing);
    bool check_triangle_consistency(const std::vector<Edge>& edges,
                                    const std::vector<size_t>& triangle_drawing,
                                    std::vector<Edge>& triangle_edges,
                                    size_t min_index, size_t max_index);
    void print_true_table();
    void process_split(size_t right_side, int right_size, const std::vector<Edge>& piercing_edges, size_t split_vertex);
    bool is_drawable();
    void add_table_entries(size_t k);
    void initialise_table();
    void select_edges(filtered_graph_t::edge_iterator start, filtered_graph_t::edge_iterator end, size_t k);
    void populate_right_sides(std::vector<std::pair<std::pair<size_t, int>, std::pair<size_t, int>>>::iterator start,
                              std::vector<std::pair<std::pair<size_t, int>, std::pair<size_t, int>>>::iterator end,
                              size_t cur_side, int count);
    void fill_right_sides();
    void print_table();

    bool is_same(const Edge& e1, const Edge& e2) const;
};


#endif //OKP_RECOGNITION_OKP_SOLVER_H
