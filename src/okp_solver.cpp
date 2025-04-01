#include <boost/graph/connected_components.hpp>
#include <boost/graph/biconnected_components.hpp>
#include <boost/graph/bipartite.hpp>
#include "okp_solver.h"

#include <ranges>

bool okp_solver::solve() {
    if (!is_biconnected()) {
        std::cout << "WARNING: The input graph is not biconnected. Use bicomponent_solver<okp_solver>" << std::endl;
    }
    vertex_order.clear();
    vertex_order.insert(vertex_order.end(), boost::vertices(graph).first, boost::vertices(graph).second);
    if (vertex_order.size() <= 3) {
        return true;
    }

    initialise_table();

    while (crossing_number < 7 && !is_drawable()) {
        add_table_entries(++crossing_number);
    }

    if (crossing_number >= 7) {
        return false;
    }

    print_true_table();
    print_table();

    size_t num_vertices = boost::num_vertices(graph);
    for (size_t v_index = 0; v_index < num_vertices; ++v_index) {
        for (size_t u_index = v_index + 1; u_index < num_vertices; ++u_index) {
            size_t right_side = (1 << num_vertices) - 1;
            right_side ^= (1 << u_index) | (1 << v_index);
            if (dp_table[v_index][u_index].contains(right_side) && !dp_table[v_index][u_index][right_side].empty()) {
                vertex_order = dp_table[v_index][u_index][right_side][0].vertex_order;
                vertex_order.push_back(u_index);
                vertex_order.push_back(v_index);
                return true;
            }
        }
    }

    std::cout << "Something went wrong" << std::endl;
    return false;
}

bool okp_solver::is_biconnected() const {
    const size_t num_components = boost::biconnected_components(graph, boost::dummy_property_map());
    return num_components == 1;
}

void okp_solver::insert_triangle_edges(std::vector<size_t>& triangle_vertex_drawing,
                                       const std::vector<Edge>& ordered_edges,
                                       size_t part_mask, size_t starting_index) {
    for (const Edge& edge : ordered_edges) {
        size_t src_index = boost::get(vertex_index_map, boost::source(edge, graph));
        size_t trg_index = boost::get(vertex_index_map, boost::target(edge, graph));
        if ((1 << src_index & part_mask) != 0) {
            triangle_vertex_drawing[src_index] = starting_index++;
        } else if ((1 << trg_index & part_mask) != 0) {
            triangle_vertex_drawing[trg_index] = starting_index++;
        } else {
            std::cout << "ERROR" << std::endl;
        }
    }
}

bool okp_solver::get_next_permutation(table_entry_t& combined_arrangement) {
    return std::next_permutation(
        combined_arrangement.edge_order.begin(),
        combined_arrangement.edge_order.end());
}

std::vector<Edge> okp_solver::get_ordered_edges(const std::vector<Edge>& edges,
                                                const std::vector<std::pair<int, int>>& edge_order) {
    std::vector<Edge> result(edges.size());
    for (size_t i = 0; i < edges.size(); i++) {
        result[edge_order[i].first] = edges[i];
    }
    return result;
}

bool okp_solver::verify_triangle_drawing(const std::vector<size_t>& triangle_vertex_drawing,
                                         const std::vector<Edge>& ordered_edges, size_t part_mask) const {
    size_t before_index = triangle_vertex_drawing.size();
    size_t after_index = 0;
    for (const Edge& edge : ordered_edges) {
        size_t src_index = boost::get(vertex_index_map, boost::source(edge, graph));
        size_t trg_index = boost::get(vertex_index_map, boost::target(edge, graph));
        if ((1 << src_index & part_mask) == 0) { std::swap(src_index, trg_index); }
        size_t triangle_source_idx = triangle_vertex_drawing[src_index];
        size_t triangle_target_idx = triangle_vertex_drawing[trg_index];
        if (triangle_source_idx < triangle_target_idx) {
            if (before_index < triangle_target_idx) {
                return false;
            }
            before_index = triangle_target_idx;
        } else {
            if (after_index > triangle_target_idx) {
                return false;
            }
            after_index = triangle_target_idx;
        }
    }
    return true;
}

bool okp_solver::check_triangle_consistency(const std::vector<Edge>& edges,
                                            const std::vector<size_t>& triangle_drawing,
                                            std::vector<Edge>& triangle_edges,
                                            size_t min_index, size_t max_index) {
    size_t previous_end_index = min_index;
    for (const Edge& edge : edges) {
        size_t src_vertex = boost::get(vertex_index_map, boost::source(edge, graph));
        size_t trg_vertex = boost::get(vertex_index_map, boost::target(edge, graph));
        size_t src_index = triangle_drawing[src_vertex];
        size_t trg_index = triangle_drawing[trg_vertex];
        if (min_index <= trg_index && trg_index <= max_index) {
            std::swap(src_index, trg_index);
        }
        if ((trg_index < min_index && (trg_index > previous_end_index || previous_end_index >= max_index)) ||
            (trg_index > max_index && trg_index > previous_end_index && previous_end_index >= max_index)) {
            return false;
        }
        previous_end_index = trg_index;
        triangle_edges.push_back(edge);
    }
    return true;
}

bool okp_solver::count_triangle_intersections(const std::vector<Edge>& part_a_edges,
                                              const std::vector<Edge>& part_b_edges,
                                              const std::vector<Edge>& piercing_edges,
                                              size_t split_vertex,
                                              std::vector<size_t>& intersections_count,
                                              const std::vector<size_t>& triangle_drawing) {
    std::vector<Edge> triangle_edges;

    if (!check_triangle_consistency(part_a_edges, triangle_drawing, triangle_edges, 1, part_a_edges.size()))
        return false;
    if (!check_triangle_consistency(part_b_edges, triangle_drawing, triangle_edges, 2 + part_a_edges.size(),
                                    1 + part_b_edges.size()))
        return false;
    if (!check_triangle_consistency(part_b_edges, triangle_drawing, triangle_edges,
                                    3 + part_a_edges.size() + part_b_edges.size(),
                                    2 + part_a_edges.size() + part_b_edges.size() + piercing_edges.size()))
        return false;

    auto [link, ok] = boost::edge(active_link.first, active_link.second, graph);
    if (ok) { triangle_edges.push_back(link); }

    for (const Edge& edge : triangle_edges) {
        size_t intersections = 0;
        size_t edge_src = triangle_drawing[boost::get(vertex_index_map, boost::source(edge, graph))];
        size_t edge_trg = triangle_drawing[boost::get(vertex_index_map, boost::target(edge, graph))];
        if (edge_src > edge_trg) { std::swap(edge_src, edge_trg); }
        for (const Edge& other : triangle_edges) {
            size_t other_src = triangle_drawing[boost::get(vertex_index_map, boost::source(other, graph))];
            size_t other_trg = triangle_drawing[boost::get(vertex_index_map, boost::target(other, graph))];
            if (other_src > other_trg) { std::swap(other_src, other_trg); }
            bool src_between = edge_src < other_src && other_src < edge_trg;
            bool trg_between = edge_src < other_trg && other_trg < edge_trg;
            intersections += (src_between && other_trg > edge_trg) ||
                (trg_between && other_src < edge_src);
        }
        size_t edge_index = boost::get(edge_index_map, edge);
        intersections_count[edge_index] += intersections;
        if (intersections_count[edge_index] > crossing_number) {
            return false;
        }
    }
    return true;
}

void okp_solver::process_split(size_t right_side, int right_size, const std::vector<Edge>& piercing_edges,
                               size_t split_vertex) {
    std::pair uw_link(active_link.first, split_vertex);
    std::pair vw_link(active_link.second, split_vertex);
    if (split_vertex < active_link.first) { std::swap(uw_link.first, uw_link.second); }
    if (split_vertex < active_link.second) { std::swap(vw_link.first, vw_link.second); }
    for (int part_a_size = 0; part_a_size < right_size; part_a_size++) {
        for (auto [part_a, part_a_edges] : dp_table_initialisation[uw_link.first][uw_link.second][part_a_size]) {
            if ((part_a & right_side) != part_a) { continue; }
            int part_b_size = right_size - part_a_size - 1;
            size_t part_b = right_side ^ part_a ^ 1 << split_vertex;
            if (!dp_table[uw_link.first][uw_link.second].contains(part_a) ||
                !dp_table[vw_link.first][vw_link.second].contains(part_b)) {
                continue;
            }
            auto vw_index = dp_table_initialisation[vw_link.first][vw_link.second][part_b_size];
            auto b_pair = std::ranges::find_if(vw_index,
                                               [part_b](const std::pair<size_t, std::vector<Edge>>& pair) {
                                                   return pair.first == part_b;
                                               });
            if (b_pair == vw_index.end()) { continue; }
            auto part_b_edges = b_pair->second;

            for (auto part_a_arrangement : dp_table[uw_link.first][uw_link.second][part_a]) {
                std::vector<Edge> part_a_edges_order = get_ordered_edges(part_a_edges, part_a_arrangement.edge_order);
                for (auto part_b_arrangement : dp_table[vw_link.first][vw_link.second][part_b]) {
                    std::vector<Edge> part_b_edges_order = get_ordered_edges(
                        part_b_edges, part_b_arrangement.edge_order);
                    table_entry_t combined_arrangement;
                    combined_arrangement.edge_order.resize(piercing_edges.size());
                    for (size_t i = 0; i < piercing_edges.size(); i++) {
                        combined_arrangement.edge_order[i].first = i;
                    }

                    do {
                        std::vector<Edge> piercing_edges_order = get_ordered_edges(
                            piercing_edges, combined_arrangement.edge_order);
                        std::vector<size_t> edges_intersection_count(boost::num_edges(graph), 0);
                        for (size_t i = 0; i < part_a_edges.size(); i++) {
                            size_t edge_index = boost::get(edge_index_map, part_a_edges[i]);
                            edges_intersection_count[edge_index] += part_a_arrangement.edge_order[i].second;
                        }
                        for (size_t i = 0; i < part_b_edges.size(); i++) {
                            size_t edge_index = boost::get(edge_index_map, part_b_edges[i]);
                            edges_intersection_count[edge_index] += part_b_arrangement.edge_order[i].second;
                        }

                        std::vector<size_t> triangle_drawing(boost::num_vertices(graph), -1);

                        triangle_drawing[boost::get(vertex_index_map, active_link.first)] = 0;
                        triangle_drawing[split_vertex] = part_a_edges.size() + 1;
                        triangle_drawing[boost::get(vertex_index_map, active_link.second)] =
                            part_a_edges.size() + part_b_edges.size() + 2;

                        insert_triangle_edges(triangle_drawing, part_a_edges_order, part_a, 1);
                        insert_triangle_edges(triangle_drawing, part_b_edges_order, part_b, 2 + part_a_edges.size());
                        insert_triangle_edges(triangle_drawing, piercing_edges_order, ~right_side,
                                              3 + part_a_edges.size() + part_b_edges.size());

                        bool ok = count_triangle_intersections(part_a_edges_order, part_b_edges_order,
                                                               piercing_edges_order, split_vertex,
                                                               edges_intersection_count, triangle_drawing);
                        if (!ok) { continue; }

                        for (size_t i = 0; i < piercing_edges.size(); i++) {
                            size_t edge_index = boost::get(edge_index_map, piercing_edges[i]);
                            combined_arrangement.edge_order[i].second = edges_intersection_count[edge_index];
                        }
                        std::ranges::reverse(combined_arrangement.edge_order);
                        combined_arrangement.vertex_order = part_a_arrangement.vertex_order;
                        combined_arrangement.vertex_order.push_back(split_vertex); // TODO: fix, it is index, not vertex
                        combined_arrangement.vertex_order.insert(combined_arrangement.vertex_order.end(),
                                                                 part_b_arrangement.vertex_order.begin(),
                                                                 part_b_arrangement.vertex_order.end());
                        dp_table[active_link.first][active_link.second][right_side].push_back(combined_arrangement);
                        std::ranges::reverse(combined_arrangement.edge_order);
                    } while (get_next_permutation(combined_arrangement));
                }
            }
        }
    }
}

bool okp_solver::is_drawable() {
    dp_table.clear();
    size_t num_vertices = boost::num_vertices(graph);
    dp_table.resize(num_vertices);
    for (int i = 0; i < num_vertices; ++i) {
        dp_table[i].resize(num_vertices);
        for (int j = i + 1; j < num_vertices; ++j) {
            dp_table[i][j][0].emplace_back();
        }
    }

    for (int right_size = 1; right_size <= num_vertices - 2; ++right_size) {
        for (size_t v_index = 0; v_index < num_vertices; ++v_index) {
            active_link.first = v_index;
            for (size_t u_index = v_index + 1; u_index < num_vertices; ++u_index) {
                active_link.second = u_index;
                for (auto [right_side, piercing_edges] : dp_table_initialisation[v_index][u_index][right_size]) {
                    for (size_t split_vertex = 0; split_vertex < num_vertices; ++split_vertex) {
                        if ((right_side & 1 << split_vertex) == 0) { continue; }
                        process_split(right_side, right_size, piercing_edges, split_vertex);
                    }
                    if (right_size == num_vertices - 2 && dp_table[v_index][u_index][right_side].size() != 0) {
                        return true;
                    }
                }
                active_link.second = -1;
            }
            active_link.first = -1;
        }
    }
#ifndef NDEBUG
    print_table();
#endif
    return false;
}

void okp_solver::add_table_entries(size_t k) {
    size_t num_vertices = boost::num_vertices(graph);
    for (size_t v_index = 0; v_index < num_vertices; ++v_index) {
        active_link.first = v_index;
        for (size_t u_index = v_index + 1; u_index < num_vertices; ++u_index) {
            active_link.second = u_index;
            select_edges(boost::edges(filtered_graph).first, boost::edges(filtered_graph).second, k);
            active_link.second = -1;
        }
        active_link.first = -1;
    }
}


void okp_solver::initialise_table() {
    dp_table_initialisation.clear();
    size_t num_vertices = boost::num_vertices(graph);
    dp_table_initialisation.resize(num_vertices);
    for (size_t v_index = 0; v_index < num_vertices; ++v_index) {
        dp_table_initialisation[v_index].resize(num_vertices);
        for (size_t u_index = 0; u_index < num_vertices; ++u_index) {
            dp_table_initialisation[v_index][u_index].resize(num_vertices - 1);
        }
    }
    for (int k = 0; k <= crossing_number; add_table_entries(k++)) {}
}

void okp_solver::select_edges(filtered_graph_t::edge_iterator start, filtered_graph_t::edge_iterator end, size_t k) {
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
    std::vector<std::pair<std::pair<size_t, int>, std::pair<size_t, int>>> sides(num_clusters);
    for (auto [vi, vi_end] = boost::vertices(filtered_graph); vi != vi_end; ++vi) {
        size_t v_index = boost::get(vertex_index_map, *vi);
        size_t component_index = component_map[v_index];
        size_t cluster_index = cluster_map[component_index];
        boost::default_color_type component_color = component_colors[component_index];
        if (component_color == boost::default_color_type::white_color) {
            sides[cluster_index].first.first |= 1 << v_index;
            ++sides[cluster_index].first.second;
        } else {
            sides[cluster_index].second.first |= 1 << v_index;
            ++sides[cluster_index].second.second;
        }
    }

    populate_right_sides(sides.begin(), sides.end(), 0, 0);
}

void okp_solver::populate_right_sides(
    std::vector<std::pair<std::pair<size_t, int>, std::pair<size_t, int>>>::iterator start,
    std::vector<std::pair<std::pair<size_t, int>, std::pair<size_t, int>>>::iterator end,
    size_t cur_side, int count) {
    if (start == end) {
        dp_table_initialisation[active_link.first][active_link.second][count].emplace_back(cur_side, filtered_edges);
        return;
    }
    auto [r, l] = *start;
    ++start;
    populate_right_sides(start, end, cur_side | r.first, count + r.second);
    populate_right_sides(start, end, cur_side | l.first, count + l.second);
}

void okp_solver::print_table() {
    for (int i = 0; i < dp_table_initialisation.size(); ++i) {
        for (int j = 0; j < dp_table_initialisation[i].size(); ++j) {
            std::cout << "Active link: " << i << "-" << j << std::endl;
            for (int k = 0; k < dp_table_initialisation[i][j].size(); ++k) {
                std::vector entries(
                    dp_table_initialisation[i][j][k].begin(), dp_table_initialisation[i][j][k].end());
                std::ranges::sort(entries);
                for (auto [side, edges] : entries) {
                    std::cout << std::bitset<10>(side) << ": [";
                    for (auto edge : edges) {
                        std::cout << edge << ", ";
                    }
                    std::cout << "]" << std::endl;
                }
            }
            std::cout << std::endl;
        }
    }


    for (int i = 0; i < dp_table.size(); ++i) {
        for (int j = 0; j < dp_table[i].size(); ++j) {
            std::cout << "Active link: " << i << "-" << j << std::endl;
            std::vector<size_t> entries;
            for (const auto& key : dp_table[i][j] | std::views::keys) {
                entries.emplace_back(key);
            }
            std::ranges::sort(entries);
            for (auto side : entries) {
                std::cout << std::bitset<10>(side) << ": {";
                for (const table_entry_t& entry : dp_table[i][j][side]) {
                    std::cout << "[(";
                    for (auto const& [edge, cross] : entry.edge_order) {
                        std::cout << edge << " " << cross << ",";
                    }
                    std::cout << "), ";
                    for (auto v : entry.vertex_order) {
                        std::cout << v;
                    }
                    std::cout << "],";
                }
                std::cout << "}" << std::endl;
            }
            std::cout << std::endl;
        }
    }
}

void okp_solver::print_true_table() {
    std::set<size_t> vertices;
    std::pair<size_t, size_t> link;
    filtered_graph_t part(graph,
                          [this, &vertices, &link](const Edge& edge) {
                              size_t src = source(edge, graph);
                              size_t trg = target(edge, graph);
                              if (src > trg) { std::swap(src, trg); }
                              return vertices.contains(src) || vertices.contains(trg) ||
                                     (src == link.first && trg == link.second);
                          },
                          [&vertices](const Vertex& vertex) { return true; });

    for (int i = 0; i < dp_table_initialisation.size(); ++i) {
        active_link.first = i;
        for (int j = i + 1; j < dp_table_initialisation[i].size(); ++j) {
            active_link.second = j;
            std::cout << "Active link: " << i << "-" << j << std::endl;
            for (int k = 0; k < dp_table_initialisation[i][j].size(); ++k) {
                std::vector entries(
                    dp_table_initialisation[i][j][k].begin(), dp_table_initialisation[i][j][k].end());
                std::ranges::sort(entries);
                for (auto [side, edges] : entries) {
                    std::cout << std::bitset<10>(side) << ": [";
                    vertices.clear();
                    for (int v = 0; (1 << v) <= side; v++) {
                        if (1 << v & side) {
                            vertices.insert(v);
                        }
                    }
                    std::vector<size_t> order;
                    order.push_back(active_link.first);
                    order.insert(order.end(), vertices.begin(), vertices.end());
                    order.push_back(active_link.second);
                    for (const Edge& edge : edges) {
                        auto src = source(edge, graph);
                        auto trg = target(edge, graph);
                        if (1 << trg & side) { std::swap(src, trg); }
                        order.push_back(trg);
                    }
                    std::sort(&order[1], &order[vertices.size()]);
                    std::sort(&order[2 + vertices.size()], &order[order.size() - 1]);
                    std::vector<size_t> index(num_vertices(graph));
                    do {
                        do {
                            for (size_t idx = 0; idx < order.size(); ++idx) {
                                index[order[idx]] = idx;
                            }
                            for (auto [ei, ei_end] = boost::edges(part); ei != ei_end; ++ei) {
                                // TODO : debug
                            }
                        } while (std::next_permutation(&order[2 + vertices.size()], &order[order.size() - 1]));
                    } while (std::next_permutation(&order[1], &order[vertices.size()]));

                    std::cout << "]" << std::endl;
                }
            }
            std::cout << std::endl;
        }
    }
}

bool okp_solver::is_same(const Edge& e1, const Edge& e2) const {
    size_t src1_index = boost::get(vertex_index_map, boost::source(e1, graph));
    size_t trg1_index = boost::get(vertex_index_map, boost::target(e1, graph));
    size_t src2_index = boost::get(vertex_index_map, boost::source(e2, graph));
    size_t trg2_index = boost::get(vertex_index_map, boost::target(e2, graph));
    if (src1_index > trg1_index) { std::swap(src1_index, trg1_index); }
    if (src2_index > trg2_index) { std::swap(src2_index, trg2_index); }
    return src1_index == trg1_index && src2_index == trg2_index;
}
