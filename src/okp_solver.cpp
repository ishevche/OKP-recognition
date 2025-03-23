#include <boost/graph/connected_components.hpp>
#include <boost/graph/biconnected_components.hpp>
#include <boost/graph/bipartite.hpp>
#include "okp_solver.h"

void print_table(
    const std::vector<std::vector<std::unordered_map<size_t, std::vector<std::vector<Edge>>>>>& dp_table);

bool okp_solver::solve() {
    if (!is_biconnected()) {
        std::cout << "The input graph has to be biconnected. Use bicomponent_solver<okp_solver>" << std::endl;
        return false;
    }
    vertex_order.clear();
    vertex_order.insert(vertex_order.end(), boost::vertices(graph).first, boost::vertices(graph).second);
    if (vertex_order.size() <= 3) {
        return true;
    }

    initialise_table();

    while (crossing_number < 7 && !is_drawable()) {
        add_table_entries(crossing_number++);
    }

    if (crossing_number >= 7) {
        return false;
    }

    // TODO: Fill vertex_order based on filled dp_table

    return true;
}

bool okp_solver::is_biconnected() {
    return true;
    std::vector<Vertex> articulation_points;
    boost::articulation_points(graph, std::back_inserter(articulation_points));
    return articulation_points.empty();
}

bool okp_solver::is_drawable() {
    return true;
}

void okp_solver::add_table_entries(size_t k) {
    for (auto [vi, vi_end] = boost::vertices(graph); vi != vi_end; ++vi) {
        size_t v_index = boost::get(vertex_index_map, *vi);
        active_link.first = *vi;
        for (auto [ui, ui_end] = boost::vertices(graph); ui != ui_end; ++ui) {
            size_t u_index = boost::get(vertex_index_map, *ui);
            if (v_index >= u_index) { continue; }
            active_link.second = *ui;
            select_edges(boost::edges(graph).first, boost::edges(graph).second, k);
            active_link.second = Graph::null_vertex();
        }
        active_link.first = Graph::null_vertex();
    }
}


void okp_solver::initialise_table() {
    dp_table_initialisation.clear();
    size_t num_vertices = boost::num_vertices(graph);
    dp_table_initialisation.resize(num_vertices);
    for (auto [vi, vi_end] = boost::vertices(graph); vi != vi_end; ++vi) {
        size_t v_index = boost::get(vertex_index_map, *vi);
        dp_table_initialisation[v_index].resize(num_vertices);
    }
    for (int k = 0; k <= crossing_number; add_table_entries(k++));

    // print_table(dp_table_initialisation);
}

void okp_solver::select_edges(Graph::edge_iterator start, Graph::edge_iterator end, size_t k) {
    if (k == 0) {
        fill_right_sides();
        return;
    }
    while (start != end) {
        Vertex src = boost::source(*start, graph);
        Vertex trg = boost::target(*start, graph);
        if (src == active_link.first || src == active_link.second ||
            trg == active_link.first || trg == active_link.second) {
            ++start;
            continue;
        }
        filtered_edges.push_back(*start);
        select_edges(++start, end, k - 1);
        filtered_edges.pop_back();
    }
}

void okp_solver::fill_right_sides() {
    std::vector<size_t> component_map(boost::num_vertices(graph));
    size_t num_components = boost::connected_components(filtered_graph,
                                                        boost::make_iterator_property_map(
                                                            component_map.begin(), vertex_index_map));
    Graph component_graph(num_components);
    for (Edge e : filtered_edges) {
        size_t source_index = boost::get(vertex_index_map, boost::source(e, graph));
        size_t target_index = boost::get(vertex_index_map, boost::target(e, graph));
        if (component_map[source_index] == component_map[target_index]) {
            return;
        }
        boost::add_edge(component_map[source_index], component_map[target_index], component_graph);
    }
    std::vector<size_t> cluster_map(num_components);
    std::vector<boost::default_color_type> component_colors(num_components);
    auto component_index_map = boost::get(boost::vertex_index, component_graph);
    size_t num_clusters = boost::connected_components(component_graph,
                                                      boost::make_iterator_property_map(
                                                          cluster_map.begin(), component_index_map));
    if (!boost::is_bipartite(component_graph, component_index_map,
                             boost::make_iterator_property_map(component_colors.begin(), component_index_map))) {
        return;
    }
    std::vector<std::pair<size_t, size_t>> sides(num_clusters);
    for (auto [vi, vi_end] = boost::vertices(filtered_graph); vi != vi_end; ++vi) {
        size_t v_index = boost::get(vertex_index_map, *vi);
        size_t component_index = component_map[v_index];
        size_t cluster_index = cluster_map[component_index];
        boost::default_color_type component_color = component_colors[component_index];
        if (component_color == boost::default_color_type::white_color) {
            sides[cluster_index].first |= 1 << v_index;
        } else {
            sides[cluster_index].second |= 1 << v_index;
        }
    }

    populate_right_sides(sides.begin(), sides.end(), 0);
}

void okp_solver::populate_right_sides(std::vector<std::pair<size_t, size_t>>::iterator start,
                                      std::vector<std::pair<size_t, size_t>>::iterator end, size_t cur_side) {
    if (start == end) {
        dp_table_initialisation[active_link.first][active_link.second][cur_side].push_back(filtered_edges);
        return;
    }
    auto [r, l] = *start;
    ++start;
    populate_right_sides(start, end, cur_side | r);
    populate_right_sides(start, end, cur_side | l);
}

void print_table(const std::vector<std::vector<std::unordered_map<size_t, std::vector<std::vector<Edge>>>>>& dp_table) {
    for (int i = 0; i < dp_table.size(); ++i) {
        for (int j = 0; j < dp_table[i].size(); ++j) {
            std::cout << "Active link: " << i << "-" << j << std::endl;
            std::vector<std::pair<std::size_t, std::vector<std::vector<Edge>>>> entries(dp_table[i][j].begin(), dp_table[i][j].end());
            std::ranges::sort(entries);
            for (auto [side, edges_set] : entries) {
                std::cout << std::bitset<10>(side) << ": ";
                for (auto edges : edges_set) {
                    std::cout << "[";
                    for (auto edge : edges) {
                        std::cout << edge << ", ";
                    }
                    std::cout << "], ";
                }
                std::cout << std::endl;
            }
            std::cout << std::endl;
        }
    }
}
