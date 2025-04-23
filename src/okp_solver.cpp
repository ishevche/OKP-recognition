#include <boost/graph/connected_components.hpp>
#include <boost/graph/biconnected_components.hpp>
#include <boost/graph/bipartite.hpp>
#include "okp_solver.h"

#include <numeric>
#include <ranges>

bool okp_solver::solve() {
    if (!is_biconnected()) {
        std::cout << "WARNING: The input graph is not biconnected. Use bicomponent_solver<okp_solver>" << std::endl;
    }
    vertex_order.clear();
    vertex_order.insert(vertex_order.end(), vertices(graph).first, vertices(graph).second);
    if (vertex_order.size() <= 3) {
        return true;
    }

    initialise_table();

    while (!is_drawable()) {
        add_table_entries(++crossing_number);
    }

#ifndef NDEBUG
    std::cout << "Entries in all dp_tables: " << misses << "\n"
        "Duplicates not inserted in tables: " << hits << std::endl;
    print_table();
#endif

    int num_vertices = static_cast<int>(boost::num_vertices(graph));
    for (int v_index = 0; v_index < num_vertices; ++v_index) {
        for (int u_index = v_index + 1; u_index < num_vertices; ++u_index) {
            size_t right_side = (1 << num_vertices) - 1;
            right_side ^= (1 << u_index) | (1 << v_index);
            if (dp_table[v_index][u_index].contains(right_side) && !dp_table[v_index][u_index][right_side].empty()) {
                vertex_order = dp_table[v_index][u_index][right_side].begin()->second.vertex_order;
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
    const int num_components = static_cast<int>(biconnected_components(graph, boost::dummy_property_map()));
    return num_components == 1;
}

void okp_solver::fill_edge_order(std::vector<edge_t>& order_vector,
                                 const std::vector<edge_t>& edges,
                                 const std::vector<std::pair<int, int>>& edge_order) {
    order_vector.resize(edges.size());
    for (int i = 0; i < static_cast<int>(edges.size()); i++) {
        order_vector[i] = edges[edge_order[i].first];
    }
}

bool okp_solver::check_triangle_consistency(const std::ranges::range auto& edges,
                                            group_t common_group, group_t prev_group) {
    std::pair prev_trg{prev_group, 0};
    for (const edge_t& edge : edges) {
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

void okp_solver::add_triangle_edges(const std::ranges::range auto& edges,
                                    vertex_t opposite_vertex,
                                    group_t opposite_group,
                                    group_t common_group) {
    for (int i = 0; i < static_cast<int>(edges.size()); ++i) {
        int edge_index = get(edge_index_map, edges[i]);
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

bool okp_solver::count_triangle_intersections(const std::ranges::range auto& part_a_edges,
                                              const std::ranges::range auto& part_b_edges,
                                              const std::ranges::range auto& piercing_edges,
                                              vertex_t split_vertex) {
    auto default_entry = std::make_pair(std::make_pair(NONE, 0), std::make_pair(NONE, 0));
    std::ranges::fill(triangle_edges_map, default_entry);
    triangle_edges.clear();

    add_triangle_edges(part_a_edges, active_link.second, LINK_TRG, PART_A);
    add_triangle_edges(part_b_edges, active_link.first, LINK_SRC, PART_B);
    add_triangle_edges(piercing_edges, split_vertex, SPLIT, PIERCE);

    if (!check_triangle_consistency(part_a_edges, PART_A, LINK_SRC)) return false;
    if (!check_triangle_consistency(part_b_edges, PART_B, SPLIT)) return false;

    auto [link, ok] = edge(active_link.first, active_link.second, graph);
    if (ok) {
        int link_index = get(edge_index_map, link);
        triangle_edges.push_back(link_index);
        triangle_edges_map[link_index] = {{LINK_SRC, 0}, {LINK_TRG, 0}};
    }

    for (int edge_index : triangle_edges) {
        int intersections = 0;
        auto edge_src = triangle_edges_map[edge_index].first;
        auto edge_trg = triangle_edges_map[edge_index].second;
        if (edge_src.first > edge_trg.first) { std::swap(edge_src, edge_trg); }
        for (int other_index : triangle_edges) {
            auto other_src = triangle_edges_map[other_index].first;;
            auto other_trg = triangle_edges_map[other_index].second;;
            if (other_src.first > other_trg.first) { std::swap(other_src, other_trg); }

            bool src_between = edge_src < other_src && other_src < edge_trg;
            bool trg_between = edge_src < other_trg && other_trg < edge_trg;
            intersections += (src_between && other_trg > edge_trg) ||
                (trg_between && other_src < edge_src);
        }
        edges_intersection_count[edge_index] += intersections;
        if (edges_intersection_count[edge_index] > crossing_number) {
            return false;
        }
    }
    return true;
}

void okp_solver::process_split(size_t right_side, int right_size, const std::vector<edge_t>& piercing_edges,
                               vertex_t split_vertex) {
    std::pair uw_link(active_link.first, split_vertex);
    std::pair vw_link(split_vertex, active_link.second);
    if (split_vertex < active_link.first) { std::swap(uw_link.first, uw_link.second); }
    if (split_vertex > active_link.second) { std::swap(vw_link.first, vw_link.second); }
    for (int part_a_size = 0; part_a_size < right_size; part_a_size++) {
        for (const auto& init_entry : dp_table_initialisation[uw_link.first][uw_link.second][part_a_size]) {
            size_t part_a = init_entry.first;
            const auto& part_a_edges = init_entry.second;
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

            for (const auto& part_a_arrangement :
                 dp_table[uw_link.first][uw_link.second][part_a] | std::views::values) {
                fill_edge_order(part_a_edges_order, part_a_edges, part_a_arrangement.edge_order);
                for (const auto& part_b_arrangement :
                     dp_table[vw_link.first][vw_link.second][part_b] | std::views::values) {
                    fill_edge_order(part_b_edges_order, part_b_edges, part_b_arrangement.edge_order);
                    combined_arrangement.edge_order.resize(piercing_edges.size());
                    for (int i = 0; i < static_cast<int>(piercing_edges.size()); i++) {
                        combined_arrangement.edge_order[i].first = i;
                    }

                    do {
                        bool ok = true;
                        fill_edge_order(piercing_edges_order, piercing_edges, combined_arrangement.edge_order);
                        std::ranges::fill(edges_intersection_count, 0);
                        for (int i = 0; i < static_cast<int>(part_a_edges_order.size()); i++) {
                            int edge_index = get(edge_index_map, part_a_edges_order[i]);
                            edges_intersection_count[edge_index] += part_a_arrangement.edge_order[i].second;
                        }
                        for (int i = 0; i < static_cast<int>(part_b_edges_order.size()); i++) {
                            int edge_index = get(edge_index_map, part_b_edges_order[i]);
                            edges_intersection_count[edge_index] += part_b_arrangement.edge_order[i].second;
                            if (edges_intersection_count[edge_index] > crossing_number) {
                                ok = false;
                                break;
                            }
                        }
                        if (!ok) { continue; }

                        if (split_vertex < active_link.first) {
                            if (split_vertex > active_link.second) {
                                ok = count_triangle_intersections(
                                    part_a_edges_order | std::views::reverse,
                                    part_b_edges_order | std::views::reverse,
                                    piercing_edges_order | std::views::reverse,
                                    split_vertex
                                );
                            } else {
                                ok = count_triangle_intersections(
                                    part_a_edges_order | std::views::reverse,
                                    part_b_edges_order,
                                    piercing_edges_order | std::views::reverse,
                                    split_vertex
                                );
                            }
                        } else {
                            if (split_vertex > active_link.second) {
                                ok = count_triangle_intersections(
                                    part_a_edges_order,
                                    part_b_edges_order | std::views::reverse,
                                    piercing_edges_order | std::views::reverse,
                                    split_vertex
                                );
                            } else {
                                ok = count_triangle_intersections(
                                    part_a_edges_order,
                                    part_b_edges_order,
                                    piercing_edges_order | std::views::reverse,
                                    split_vertex
                                );
                            }
                        }
                        if (!ok) { continue; }

                        std::string key;
                        for (int i = 0; i < static_cast<int>(piercing_edges_order.size()); i++) {
                            int edge_index = get(edge_index_map, piercing_edges_order[i]);
                            combined_arrangement.edge_order[i].second = edges_intersection_count[edge_index];
                            key += std::to_string(combined_arrangement.edge_order[i].first) +
                                "_" + std::to_string(combined_arrangement.edge_order[i].second) + " ";
                        }
                        combined_arrangement.vertex_order.clear();
                        if (split_vertex < active_link.first) {
                            combined_arrangement.vertex_order.insert(combined_arrangement.vertex_order.end(),
                                                                     part_a_arrangement.vertex_order.rbegin(),
                                                                     part_a_arrangement.vertex_order.rend());
                        } else {
                            combined_arrangement.vertex_order.insert(combined_arrangement.vertex_order.end(),
                                                                     part_a_arrangement.vertex_order.begin(),
                                                                     part_a_arrangement.vertex_order.end());
                        }
                        combined_arrangement.vertex_order.push_back(split_vertex);
                        if (split_vertex > active_link.second) {
                            combined_arrangement.vertex_order.insert(combined_arrangement.vertex_order.end(),
                                                                     part_b_arrangement.vertex_order.rbegin(),
                                                                     part_b_arrangement.vertex_order.rend());
                        } else {
                            combined_arrangement.vertex_order.insert(combined_arrangement.vertex_order.end(),
                                                                     part_b_arrangement.vertex_order.begin(),
                                                                     part_b_arrangement.vertex_order.end());
                        }

                        auto& cell = dp_table[active_link.first][active_link.second][right_side];
                        if (!cell.contains(key)) {
                            cell[key] = combined_arrangement;
#ifndef NDEBUG
                            misses++;
                        } else {
                            hits++;
#endif
                        }
                    } while (std::ranges::next_permutation(combined_arrangement.edge_order).found);
                }
            }
        }
    }
}

bool okp_solver::is_drawable() {
    dp_table.clear();
    int num_vertices = static_cast<int>(boost::num_vertices(graph));
    dp_table.resize(num_vertices);
    for (int i = 0; i < num_vertices; ++i) {
        dp_table[i].resize(num_vertices);
        for (int j = i + 1; j < num_vertices; ++j) {
            dp_table[i][j][0][""] = {};
        }
    }

    for (int right_size = 1; right_size <= num_vertices - 2; ++right_size) {
        for (int v_index = 0; v_index < num_vertices; ++v_index) {
            active_link.first = v_index;
            for (int u_index = v_index + 1; u_index < num_vertices; ++u_index) {
                active_link.second = u_index;
                for (const auto& [right_side, piercing_edges] : dp_table_initialisation[v_index][u_index][right_size]) {
                    for (int split_vertex = 0; split_vertex < num_vertices; ++split_vertex) {
                        if ((right_side & 1 << split_vertex) == 0) { continue; }
                        process_split(right_side, right_size, piercing_edges, split_vertex);
                    }
                    if (right_size == num_vertices - 2 && !dp_table[v_index][u_index][right_side].empty()) {
                        return true;
                    }
                }
                active_link.second = -1;
            }
            active_link.first = -1;
        }
    }
    return false;
}

void okp_solver::add_table_entries(int k) {
    int num_vertices = static_cast<int>(boost::num_vertices(graph));
    for (int v_index = 0; v_index < num_vertices; ++v_index) {
        active_link.first = v_index;
        for (int u_index = v_index + 1; u_index < num_vertices; ++u_index) {
            active_link.second = u_index;
            select_edges(edges(filtered_graph).first, edges(filtered_graph).second, k);
            active_link.second = -1;
        }
        active_link.first = -1;
    }
}


void okp_solver::initialise_table() {
    dp_table_initialisation.clear();
    int num_vertices = static_cast<int>(boost::num_vertices(graph));
    dp_table_initialisation.resize(num_vertices);
    for (int v_index = 0; v_index < num_vertices; ++v_index) {
        dp_table_initialisation[v_index].resize(num_vertices);
        for (int u_index = 0; u_index < num_vertices; ++u_index) {
            dp_table_initialisation[v_index][u_index].resize(num_vertices - 1);
        }
    }
    for (int k = 0; k <= crossing_number; add_table_entries(k++)) {}
}

void okp_solver::select_edges(filtered_graph_t::edge_iterator start,
                              const filtered_graph_t::edge_iterator& end,
                              int k) {
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
    std::vector<int> component_map(num_vertices(graph));
    int num_components = connected_components(
        filtered_graph,
        make_iterator_property_map(component_map.begin(), vertex_index_map)
    );
    graph_t component_graph(num_components);
    for (edge_t e : filtered_edges) {
        int source_index = static_cast<int>(get(vertex_index_map, source(e, graph)));
        int target_index = static_cast<int>(get(vertex_index_map, target(e, graph)));
        if (component_map[source_index] == component_map[target_index]) {
            return;
        }
        add_edge(component_map[source_index], component_map[target_index], component_graph);
    }
    std::vector<int> cluster_map(num_components);
    std::vector<boost::default_color_type> component_colors(num_components);
    auto component_index_map = get(boost::vertex_index, component_graph);
    int num_clusters = connected_components(
        component_graph,
        make_iterator_property_map(cluster_map.begin(), component_index_map)
    );
    if (!is_bipartite(component_graph, component_index_map,
                      make_iterator_property_map(component_colors.begin(), component_index_map))) {
        return;
    }
    std::vector<std::pair<std::pair<size_t, int>, std::pair<size_t, int>>> sides(num_clusters);
    for (auto [vi, vi_end] = vertices(filtered_graph); vi != vi_end; ++vi) {
        int v_index = static_cast<int>(get(vertex_index_map, *vi));
        int component_index = component_map[v_index];
        int cluster_index = cluster_map[component_index];
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

#ifndef NDEBUG
void okp_solver::print_table() {
    for (int i = 0; i < static_cast<int>(dp_table.size()); ++i) {
        for (int j = 0; j < static_cast<int>(dp_table[i].size()); ++j) {
            std::cout << "Active link: " << i << "-" << j << std::endl;
            std::vector<size_t> entries;
            for (const auto& key : dp_table[i][j] | std::views::keys) {
                entries.emplace_back(key);
            }
            std::ranges::sort(entries);
            for (auto side : entries) {
                int size = 0;
                for (int k = 0; 1UL << k <= side; size += (1 << k++ & side) != 0) {}
                auto edges = std::ranges::find_if(dp_table_initialisation[i][j][size],
                                                  [side](const std::pair<size_t, std::vector<edge_t>>& pair) {
                                                      return pair.first == side;
                                                  })->second;
                std::cout << std::bitset<10>(side) << ": {";
                for (const table_entry_t& entry : dp_table[i][j][side] | std::views::values) {
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
#endif
