#ifndef OKP_RECOGNITION_OKP_SOLVER_H
#define OKP_RECOGNITION_OKP_SOLVER_H


#include "solver.h"

class okp_solver : public solver {
    typedef boost::property_map<graph_t, boost::edge_index_t>::const_type edge_index_map_t;
    typedef boost::property_map<graph_t, boost::vertex_index_t>::const_type vertex_index_map_t;

    const vertex_index_map_t vertex_index_map;
    const edge_index_map_t edge_index_map;

public:
    explicit okp_solver(const graph_t& graph, int crossing_number = 0)
        : solver(graph, crossing_number),
          vertex_index_map(get(boost::vertex_index, graph)),
          edge_index_map(get(boost::edge_index, graph)),
          filtered_graph(graph, [&](const edge_t& v) {
                             return std::ranges::find(filtered_edges, v) == filtered_edges.end();
                         }, [&](vertex_t v) {
                             return active_link.first != get(vertex_index_map, v)
                                    && active_link.second != get(vertex_index_map, v);
                         }),
          active_link(graph_t::null_vertex(), graph_t::null_vertex()),
          triangle_edges_map(num_edges(graph)),
          edges_intersection_count(num_edges(graph), 0) {
        triangle_edges.reserve(num_edges(graph));
    }

    bool solve() override;

    struct table_entry_t {
        std::vector<std::pair<int, int>> edge_order;
        std::vector<vertex_t> vertex_order;
    };

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
    std::vector<std::vector<std::vector<std::unordered_map<size_t, std::vector<edge_t>>>>> dp_table_initialisation;
    typedef boost::filtered_graph<graph_t, boost::function<bool(edge_t)>, boost::function<bool(vertex_t)>> filtered_graph_t;
    filtered_graph_t filtered_graph;
    std::pair<vertex_t, vertex_t> active_link;
    std::vector<edge_t> filtered_edges;
    std::vector<int> triangle_edges;
    std::vector<std::pair<std::pair<group_t, int>, std::pair<group_t, int>>> triangle_edges_map;
    std::vector<int> edges_intersection_count;
    std::vector<edge_t> part_a_edges_order;
    std::vector<edge_t> part_b_edges_order;
    std::vector<edge_t> piercing_edges_order;
    table_entry_t combined_arrangement;

    bool is_biconnected() const;
    void static fill_edge_order(std::vector<edge_t>& order_vector,
                                const std::vector<edge_t>& edges,
                                const std::vector<std::pair<int, int>>& edge_order);
    bool count_triangle_intersections(const std::ranges::range auto& part_a_edges,
                                      const std::ranges::range auto& part_b_edges,
                                      const std::ranges::range auto& piercing_edges,
                                      vertex_t split_vertex);
    void process_split(size_t right_side, int right_size, const std::vector<edge_t>& piercing_edges, vertex_t split_vertex);
    bool is_drawable();
    void add_table_entries(int k);
    void initialise_table();
    void select_edges(filtered_graph_t::edge_iterator start, const filtered_graph_t::edge_iterator& end, int k);
    void populate_right_sides(std::vector<std::pair<std::pair<size_t, int>, std::pair<size_t, int>>>::iterator start,
                              std::vector<std::pair<std::pair<size_t, int>, std::pair<size_t, int>>>::iterator end,
                              size_t cur_side, int count);
    void fill_right_sides();
    bool check_triangle_consistency(const std::ranges::range auto& edges,
                                    group_t common_group, group_t prev_group);
    void add_triangle_edges(const std::ranges::range auto& edges,
                            vertex_t opposite_vertex,
                            group_t opposite_group,
                            group_t common_group);

#ifndef NDEBUG
    size_t hits = 0;
    size_t misses = 0;

    void print_table();
#endif
};


#endif //OKP_RECOGNITION_OKP_SOLVER_H
