#include <boost/graph/connected_components.hpp>
#include <boost/graph/biconnected_components.hpp>
#include <boost/graph/bipartite.hpp>
#include <boost/algorithm/string/join.hpp>
#include "okp_solver.h"

#include <numeric>
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

#ifndef NDEBUG
    print_table();
#endif

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
    const size_t num_components = biconnected_components(graph, boost::dummy_property_map());
    return num_components == 1;
}


std::vector<Edge> okp_solver::get_ordered_edges(const std::vector<Edge>& edges,
                                                const std::vector<std::pair<int, int>>& edge_order) {
    std::vector<Edge> result(edges.size());
    for (size_t i = 0; i < edges.size(); i++) {
        result[i] = edges[edge_order[i].first];
    }
    return result;
}

bool okp_solver::check_triangle_consistency(const std::vector<Edge>& edges,
                                            group_t common_group, group_t prev_group) {
    std::pair<group_t, size_t> prev_trg{prev_group, 0};
    for (const Edge& edge : edges) {
        const auto& triangle_edge = triangle_edges_map[get(edge_index_map, edge)];
        auto src = triangle_edge.first;
        auto trg = triangle_edge.second;
        if (src.first != common_group) {
            std::swap(src, trg);
        }
        if ((trg < src && (trg > prev_trg || prev_trg >= src)) ||
            (trg > src && trg > prev_trg && prev_trg >= src)) {
            return false;
        }
        prev_trg = trg;
    }
    return true;
}

void okp_solver::add_triangle_edges(const std::vector<Edge>& edges,
                                    Vertex opposite_vertex,
                                    group_t opposite_group,
                                    group_t common_group) {
    for (size_t i = 0; i < edges.size(); ++i) {
        size_t edge_index = get(edge_index_map, edges[i]);
        if (source(edges[i], graph) == opposite_vertex ||
            target(edges[i], graph) == opposite_vertex) {
            triangle_edges_map[edge_index].second = {opposite_group, 0};
        }
        if (triangle_edges_map[edge_index].first.first == NONE) {
            triangle_edges_map[edge_index].first = {common_group, i};
            triangle_edges.push_back(edge_index);
        } else {
            triangle_edges_map[edge_index].second = {common_group, i};
        }
    }
}

bool okp_solver::count_triangle_intersections(const std::vector<Edge>& part_a_edges,
                                              const std::vector<Edge>& part_b_edges,
                                              const std::vector<Edge>& piercing_edges,
                                              size_t split_vertex,
                                              std::vector<size_t>& intersections_count) {
    auto default_entry = std::make_pair(std::make_pair(NONE, 0), std::make_pair(NONE, 0));
    std::ranges::fill(triangle_edges_map, default_entry);
    triangle_edges.clear();

    add_triangle_edges(part_a_edges, active_link.second, LINK_TRG, PART_A);
    add_triangle_edges(part_b_edges, active_link.first, LINK_SRC, PART_B);
    add_triangle_edges(piercing_edges, split_vertex, SPLIT, PIERCE);

    if (!check_triangle_consistency(part_a_edges, PART_A, LINK_SRC)) return false;
    if (!check_triangle_consistency(part_b_edges, PART_B, SPLIT)) return false;

    auto [link, ok] = boost::edge(active_link.first, active_link.second, graph);
    if (ok) {
        size_t link_index = get(edge_index_map, link);
        triangle_edges.push_back(link_index);
        triangle_edges_map[link_index] = {{LINK_SRC, 0}, {LINK_TRG, 0}};
    }

    for (size_t edge_index : triangle_edges) {
        size_t intersections = 0;
        auto edge_src = triangle_edges_map[edge_index].first;
        auto edge_trg = triangle_edges_map[edge_index].second;
        if (edge_src.first > edge_trg.first) { std::swap(edge_src, edge_trg); }
        for (size_t other_index : triangle_edges) {
            auto other_src = triangle_edges_map[other_index].first;;
            auto other_trg = triangle_edges_map[other_index].second;;
            if (other_src.first > other_trg.first) { std::swap(other_src, other_trg); }

            bool src_between = edge_src < other_src && other_src < edge_trg;
            bool trg_between = edge_src < other_trg && other_trg < edge_trg;
            intersections += (src_between && other_trg > edge_trg) ||
                (trg_between && other_src < edge_src);
        }
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
    std::pair vw_link(split_vertex, active_link.second);
    if (split_vertex < active_link.first) { std::swap(uw_link.first, uw_link.second); }
    if (split_vertex > active_link.second) { std::swap(vw_link.first, vw_link.second); }
    for (int part_a_size = 0; part_a_size < right_size; part_a_size++) {
        for (auto [part_a, part_a_edges] : dp_table_initialisation[uw_link.first][uw_link.second][part_a_size]) {
            if ((part_a & right_side) != part_a) { continue; }
            int part_b_size = right_size - part_a_size - 1;
            size_t part_b = right_side ^ part_a ^ 1 << split_vertex;
            if (!dp_table[uw_link.first][uw_link.second].contains(part_a) ||
                !dp_table[vw_link.first][vw_link.second].contains(part_b)) {
                continue;
            }
            const auto& vw_index = dp_table_initialisation[vw_link.first][vw_link.second][part_b_size];
            if (!vw_index.contains(part_b)) { continue; }
            const auto& part_b_edges = vw_index.at(part_b);

            for (auto part_a_arrangement : dp_table[uw_link.first][uw_link.second][part_a]) {
                if (split_vertex < active_link.first) {
                    std::ranges::reverse(part_a_arrangement.edge_order);
                    std::ranges::reverse(part_a_arrangement.vertex_order);
                }
                std::vector<Edge> part_a_edges_order = get_ordered_edges(part_a_edges, part_a_arrangement.edge_order);
                for (auto part_b_arrangement : dp_table[vw_link.first][vw_link.second][part_b]) {
                    if (split_vertex > active_link.second) {
                        std::ranges::reverse(part_b_arrangement.edge_order);
                        std::ranges::reverse(part_b_arrangement.vertex_order);
                    }
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
                        std::ranges::fill(edges_intersection_count, 0);
                        for (size_t i = 0; i < part_a_edges_order.size(); i++) {
                            size_t edge_index = get(edge_index_map, part_a_edges_order[i]);
                            edges_intersection_count[edge_index] += part_a_arrangement.edge_order[i].second;
                        }
                        for (size_t i = 0; i < part_b_edges_order.size(); i++) {
                            size_t edge_index = boost::get(edge_index_map, part_b_edges_order[i]);
                            edges_intersection_count[edge_index] += part_b_arrangement.edge_order[i].second;
                        }

                        bool ok = count_triangle_intersections(part_a_edges_order, part_b_edges_order,
                                                               piercing_edges_order, split_vertex,
                                                               edges_intersection_count);
                        if (!ok) { continue; }

                        for (size_t i = 0; i < piercing_edges_order.size(); i++) {
                            size_t edge_index = boost::get(edge_index_map, piercing_edges_order[i]);
                            combined_arrangement.edge_order[i].second = edges_intersection_count[edge_index];
                        }
                        std::ranges::reverse(combined_arrangement.edge_order);
                        combined_arrangement.vertex_order = part_a_arrangement.vertex_order;
                        combined_arrangement.vertex_order.push_back(split_vertex); // TODO: fix, it is index, not vertex
                        combined_arrangement.vertex_order.insert(combined_arrangement.vertex_order.end(),
                                                                 part_b_arrangement.vertex_order.begin(),
                                                                 part_b_arrangement.vertex_order.end());
#ifndef NDEBUG
                        if (active_link.first == 0 && active_link.second == 1 &&
                            combined_arrangement.vertex_order.size() == 5 &&
                            combined_arrangement.vertex_order[0] == 2 &&
                            combined_arrangement.vertex_order[1] == 3 &&
                            combined_arrangement.vertex_order[2] == 5 &&
                            combined_arrangement.vertex_order[3] == 4 &&
                            combined_arrangement.vertex_order[4] == 6) {
                            std::cout << "Active link:" << active_link.first << "-" << active_link.second << "\n";
                            std::cout << std::bitset<10>(right_side) << ": [(";
                            for (auto [edge_id, cross] : combined_arrangement.edge_order) {
                                std::cout << std::to_string(piercing_edges[edge_id].m_source) + std::to_string(
                                    piercing_edges[edge_id].m_target) + " " + std::to_string(cross) << ",";
                            }
                            std::cout << "), ";
                            for (auto v : combined_arrangement.vertex_order) {
                                std::cout << v;
                            }
                            std::cout << "]\nCombined from: " << active_link.first << "-" << split_vertex
                                << "-" << active_link.second << "\n";
                            std::cout << std::bitset<10>(part_a) << ": ";
                            std::cout << "[(";
                            for (auto [edge_id, cross] : part_a_arrangement.edge_order) {
                                std::cout << std::to_string(part_a_edges[edge_id].m_source) + std::to_string(
                                    part_a_edges[edge_id].m_target) + " " + std::to_string(cross) << ",";
                            }
                            std::cout << "), ";
                            for (auto v : part_a_arrangement.vertex_order) {
                                std::cout << v;
                            }
                            std::cout << "]\n";
                            std::cout << std::bitset<10>(part_b) << ": ";
                            std::cout << "[(";
                            for (auto [edge_id, cross] : part_b_arrangement.edge_order) {
                                std::cout << std::to_string(part_b_edges[edge_id].m_source) + std::to_string(
                                    part_b_edges[edge_id].m_target) + " " + std::to_string(cross) << ",";
                            }
                            std::cout << "), ";
                            for (auto v : part_b_arrangement.vertex_order) {
                                std::cout << v;
                            }
                            std::cout << "]\n";
                        }
#endif
                        dp_table[active_link.first][active_link.second][right_side].push_back(combined_arrangement);
                        std::ranges::reverse(combined_arrangement.edge_order);
                    } while (std::ranges::next_permutation(combined_arrangement.edge_order).found);
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
    bool ok = false;

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
    return ok;
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
    std::vector<size_t> component_map(num_vertices(graph));
    size_t num_components = connected_components(
        filtered_graph,
        make_iterator_property_map(component_map.begin(), vertex_index_map)
    );
    Graph component_graph(num_components);
    for (Edge e : filtered_edges) {
        size_t source_index = get(vertex_index_map, source(e, graph));
        size_t target_index = get(vertex_index_map, target(e, graph));
        if (component_map[source_index] == component_map[target_index]) {
            return;
        }
        boost::add_edge(component_map[source_index], component_map[target_index], component_graph);
    }
    std::vector<size_t> cluster_map(num_components);
    std::vector<boost::default_color_type> component_colors(num_components);
    auto component_index_map = get(boost::vertex_index, component_graph);
    size_t num_clusters = connected_components(
        component_graph,
        make_iterator_property_map(cluster_map.begin(), component_index_map)
    );
    if (!is_bipartite(component_graph, component_index_map,
                      make_iterator_property_map(component_colors.begin(), component_index_map))) {
        return;
    }
    std::vector<std::pair<std::pair<size_t, int>, std::pair<size_t, int>>> sides(num_clusters);
    for (auto [vi, vi_end] = vertices(filtered_graph); vi != vi_end; ++vi) {
        size_t v_index = get(vertex_index_map, *vi);
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
        dp_table_initialisation[active_link.first][active_link.second][count][cur_side] = filtered_edges;
        return;
    }
    auto [r, l] = *start;
    ++start;
    populate_right_sides(start, end, cur_side | r.first, count + r.second);
    populate_right_sides(start, end, cur_side | l.first, count + l.second);
}

void okp_solver::print_table() {
    for (int i = 0; i < dp_table.size(); ++i) {
        for (int j = 0; j < dp_table[i].size(); ++j) {
            std::cout << "Active link: " << i << "-" << j << std::endl;
            std::vector<size_t> entries;
            for (const auto& key : dp_table[i][j] | std::views::keys) {
                entries.emplace_back(key);
            }
            std::ranges::sort(entries);
            for (auto side : entries) {
                int size = 0;
                for (int k = 0; 1 << k <= side; size += (1 << k++ & side) != 0) {}
                auto edges = std::ranges::find_if(dp_table_initialisation[i][j][size],
                                                  [side](const std::pair<size_t, std::vector<Edge>>& pair) {
                                                      return pair.first == side;
                                                  })->second;
                std::cout << std::bitset<10>(side) << ": {";
                for (const table_entry_t& entry : dp_table[i][j][side]) {
                    std::cout << "[(";
                    for (auto [edge_id, cross] : entry.edge_order) {
                        std::cout << std::to_string(edges[edge_id].m_source) + std::to_string(
                            edges[edge_id].m_target) + " " + std::to_string(cross) << ",";
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
