#include <boost/graph/connected_components.hpp>
#include <boost/graph/biconnected_components.hpp>
#include "okp_solver.h"

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
    std::vector<Vertex> articulation_points;
    boost::articulation_points(graph, std::back_inserter(articulation_points));
    return articulation_points.empty();
}

bool okp_solver::is_drawable() {

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
    dp_table.clear();
    size_t num_vertices = boost::num_vertices(graph);
    dp_table.resize(num_vertices);
    for (auto [vi, vi_end] = boost::vertices(graph); vi != vi_end; ++vi) {
        size_t v_index = boost::get(vertex_index_map, *vi);
        dp_table[v_index].resize(num_vertices);
    }
    for (int k = 0; k <= crossing_number; add_table_entries(k++));
}

void okp_solver::select_edges(Graph::edge_iterator start, Graph::edge_iterator end, size_t k) {
    if (k == 0) {
        fill_right_sides();
        return;
    }
    while (start != end) {
        filtered_edges.push_back(*start);
        select_edges(++start, end, k - 1);
        filtered_edges.pop_back();
    }
}

void okp_solver::fill_right_sides() {
    std::vector<size_t> component_map(boost::num_vertices(graph));
    size_t num_components = boost::connected_components(filtered_graph, boost::make_iterator_property_map(component_map.begin(), vertex_index_map));
    Graph component_graph(num_components);
    for (Edge e : filtered_edges) {
        size_t source_index = boost::get(vertex_index_map, boost::source(e, graph));
        size_t target_index = boost::get(vertex_index_map, boost::target(e, graph));
        if (component_map[source_index] == component_map[target_index]) {
            return;
        }
        boost::add_edge(source_index, target_index, component_graph);
    }

}
