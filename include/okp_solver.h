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
          vertex_index_map(get(boost::vertex_index, graph)),
          edge_index_map(get(boost::edge_index, graph)),
          filtered_graph(graph, [&](const Edge& v) {
                             return std::ranges::find(filtered_edges, v) == filtered_edges.end();
                         }, [&](Vertex v) {
                             return active_link.first != get(vertex_index_map, v)
                                    && active_link.second != get(vertex_index_map, v);
                         }),
          active_link(Graph::null_vertex(), Graph::null_vertex()),
          triangle_edges_map(num_edges(graph)),
          edges_intersection_count(num_edges(graph), 0) {
        triangle_edges.reserve(num_edges(graph));
    }

    bool solve() override;

    struct table_entry_t {
        std::vector<std::pair<int, int>> edge_order;
        std::vector<Vertex> vertex_order;
    };

#ifndef NDEBUG
    size_t hits(0);
    size_t misses(0);
#endif

private:
    enum group_t {
        LINK_SRC = 0,
        PART_A   = 1,
        SPLIT    = 2,
        PART_B   = 3,
        LINK_TRG = 4,
        PIERCE   = 5,
        NONE     = 6
    };

    std::vector<std::vector<std::unordered_map<size_t, std::unordered_map<std::string, table_entry_t>>>> dp_table;
    std::vector<std::vector<std::vector<std::unordered_map<size_t, std::vector<Edge>>>>> dp_table_initialisation;
    typedef boost::filtered_graph<Graph, boost::function<bool(Edge)>, boost::function<bool(Vertex)>> filtered_graph_t;
    filtered_graph_t filtered_graph;
    std::pair<size_t, size_t> active_link;
    std::vector<Edge> filtered_edges;
    std::vector<size_t> triangle_edges;
    std::vector<std::pair<std::pair<group_t, int>, std::pair<group_t, int>>> triangle_edges_map;
    std::vector<size_t> edges_intersection_count;
    std::vector<Edge> part_a_edges_order;
    std::vector<Edge> part_b_edges_order;
    std::vector<Edge> piercing_edges_order;
    table_entry_t combined_arrangement;

    bool is_biconnected() const;
    void static fill_edge_order(std::vector<Edge>& order_vector,
                                const std::vector<Edge>& edges,
                                const std::vector<std::pair<int, int>>& edge_order);
    bool count_triangle_intersections(const std::ranges::range auto& part_a_edges,
                                      const std::ranges::range auto& part_b_edges,
                                      const std::ranges::range auto& piercing_edges,
                                      size_t split_vertex);
    void process_split(size_t right_side, int right_size, const std::vector<Edge>& piercing_edges, size_t split_vertex);
    bool is_drawable();
    void add_table_entries(size_t k);
    void initialise_table();
    void select_edges(filtered_graph_t::edge_iterator start, const filtered_graph_t::edge_iterator& end, size_t k);
    void populate_right_sides(std::vector<std::pair<std::pair<size_t, int>, std::pair<size_t, int>>>::iterator start,
                              std::vector<std::pair<std::pair<size_t, int>, std::pair<size_t, int>>>::iterator end,
                              size_t cur_side, int count);
    void fill_right_sides();
    void print_table();
    bool check_triangle_consistency(const std::ranges::range auto& edges,
                                    group_t common_group, group_t prev_group);
    void add_triangle_edges(const std::ranges::range auto& edges,
                            Vertex opposite_vertex,
                            group_t opposite_group,
                            group_t common_group);
};


#endif //OKP_RECOGNITION_OKP_SOLVER_H
