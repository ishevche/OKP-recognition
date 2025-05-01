#ifndef OKP_RECOGNITION_OKP_SOLVER_H
#define OKP_RECOGNITION_OKP_SOLVER_H


#include "abstract_solver.h"

class dp_solver : public abstract_solver {
    typedef boost::property_map<graph_t, boost::edge_index_t>::const_type edge_index_map_t;
    typedef boost::property_map<graph_t, boost::vertex_index_t>::const_type vertex_index_map_t;

    const vertex_index_map_t vertex_index_map;
    const edge_index_map_t edge_index_map;

public:
    explicit dp_solver(const graph_t& graph, int crossing_number = 0)
        : abstract_solver(graph, crossing_number),
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
    enum triangle_vertex_type_t {
        LINK_SRC      = 0,
        HELPER_PART_A = 1,
        SPLIT         = 2,
        HELPER_PART_B = 3,
        LINK_TRG      = 4,
        HELPER_PIERCE = 5,
        NONE          = 6
    };

    std::pair<vertex_t, vertex_t> active_link;
    std::vector<edge_t> part_a_edges_order;
    std::vector<edge_t> part_b_edges_order;
    std::vector<edge_t> piercing_edges_order;
    table_entry_t combined_entry;
    std::vector<std::vector<std::unordered_map<size_t, std::unordered_map<std::string, table_entry_t>>>> dp_table;

    typedef boost::filtered_graph<graph_t, boost::function<bool(edge_t)>, boost::function<bool(vertex_t)>>
    filtered_graph_t;
    filtered_graph_t filtered_graph;
    std::vector<edge_t> filtered_edges;
    std::vector<std::vector<std::vector<std::unordered_map<size_t, std::vector<edge_t>>>>> dp_table_initialisation;

    typedef std::pair<triangle_vertex_type_t, int> triangle_vertex_t;
    typedef std::pair<triangle_vertex_t, triangle_vertex_t> triangle_edge_t;
    std::vector<int> triangle_edges;
    std::vector<triangle_edge_t> triangle_edges_map;
    std::vector<int> edges_intersection_count;


    bool check_input();
    void static fill_edge_order(std::vector<edge_t>& order_vector,
                                const std::vector<edge_t>& edges,
                                const std::vector<std::pair<int, int>>& edge_order);
    void process_split(size_t right_side, int right_size, const std::vector<edge_t>& piercing_edges,
                       vertex_t split_vertex);
    void process_all_arrangements(const std::vector<edge_t>& piercing_edges,
                                  vertex_t split_vertex, size_t right_side,
                                  const table_entry_t& part_a_entry,
                                  const table_entry_t& part_b_entry);
    void combine_vertex_order(vertex_t split_vertex,
                              const table_entry_t& part_a_entry,
                              const table_entry_t& part_b_entry);
    bool is_drawable();


    void add_table_entries(int k);
    void initialise_table();
    void select_edges(filtered_graph_t::edge_iterator start, const filtered_graph_t::edge_iterator& end, int k);
    void fill_right_sides();
    void populate_right_sides(std::vector<std::pair<std::pair<size_t, int>, std::pair<size_t, int>>>::iterator start,
                              std::vector<std::pair<std::pair<size_t, int>, std::pair<size_t, int>>>::iterator end,
                              size_t cur_side, int count);


    bool check_inner_triangle(vertex_t split_vertex);
    bool check_triangle_consistency(const std::ranges::range auto& edges,
                                    triangle_vertex_type_t edge_type, triangle_vertex_type_t prev_type);
    void add_triangle_edges(const std::ranges::range auto& edges,
                            vertex_t opposite_vertex,
                            triangle_vertex_type_t opposite_type,
                            triangle_vertex_type_t common_type);
    bool count_triangle_intersections(const std::ranges::range auto& part_a_edges,
                                      const std::ranges::range auto& part_b_edges,
                                      const std::ranges::range auto& piercing_edges,
                                      vertex_t split_vertex);

#ifndef NDEBUG
    size_t hits = 0;
    size_t misses = 0;

    void print_table();
#endif
};


#endif //OKP_RECOGNITION_OKP_SOLVER_H
