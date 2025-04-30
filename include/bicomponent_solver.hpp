#ifndef OKP_RECOGNITION_BICOMPONENT_SOLVER_H
#define OKP_RECOGNITION_BICOMPONENT_SOLVER_H

#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/biconnected_components.hpp>
#include "bctree.h"
#include "abstract_solver.h"

template <class sub_solver>
class bicomponent_solver : public abstract_solver {
public:
    explicit bicomponent_solver(const graph_t& graph, int crossing_number = 0)
        : abstract_solver(graph, crossing_number) {}

    bool solve() override {
        vertex_order.clear();
        if (num_vertices(graph) <= 3) {
            vertex_order.insert(vertex_order.end(), vertices(graph).first, vertices(graph).second);
            return true;
        }

        bool ok = true;
        dfs_visitor visitor(vertex_order, crossing_number, ok);

        bctree_t bctree = decompose(graph);
        std::vector<boost::default_color_type> color_map(num_vertices(bctree));
        auto color = make_iterator_property_map(color_map.begin(), get(boost::vertex_index, bctree));

        depth_first_search(bctree, visitor, color, 0);
        return ok;
    }

private:
    class dfs_visitor : public boost::default_dfs_visitor {
        int& crossing_number;
        std::vector<vertex_t>& vertex_order;
        std::map<bctree_vertex, bctree_vertex> predecessors;
        bool& ok;

    public:
        dfs_visitor(std::vector<vertex_t>& vertex_order, int& crossing_number, bool& ok)
            : crossing_number(crossing_number), vertex_order(vertex_order), ok(ok) {}

        void discover_vertex(bctree_vertex node, const bctree_t& tree) {
            if (!ok) { return; }

            if (tree[node].node_type == C_NODE) { return; }
            sub_solver component_solver(tree[node].bi_component, crossing_number);
            ok = component_solver.solve();

            if (!ok) { return; }

            std::vector<vertex_t>& component_order = component_solver.vertex_order;

            typedef boost::property_map<graph_t, boost::vertex_index_t>::const_type vertex_index_map_t;
            vertex_index_map_t component_index_map = get(boost::vertex_index, tree[node].bi_component);
            std::transform(component_order.begin(), component_order.end(), component_order.begin(),
                           [&tree, &node, &component_index_map](const vertex_t& v) {
                               return tree[node].original_vertices[get(component_index_map, v)];
                           });

            if (vertex_order.empty()) {
                vertex_order.insert(vertex_order.end(), component_order.begin(), component_order.end());
                crossing_number = component_solver.crossing_number;
                return;
            }

            auto cut_node = predecessors.find(node);
            if (cut_node == predecessors.end()) {
                std::cout << "Can not find predecessor!!" << std::endl;
                ok = false;
                return;
            }

            vertex_t articulation_point = tree[cut_node->second].articulation_point;
            auto rotate_iter = std::ranges::find(component_order, articulation_point);
            std::ranges::rotate(component_order, rotate_iter);

            auto insert_iter = ++std::ranges::find(vertex_order, articulation_point);
            vertex_order.insert(insert_iter, ++component_order.begin(), component_order.end());

            crossing_number = std::max(component_solver.crossing_number, crossing_number);
        }

        void tree_edge(const edge_t& edge, const bctree_t& graph) {
            predecessors[target(edge, graph)] = source(edge, graph);
        }
    };

    bctree_t decompose(const graph_t& graph) {
        typedef boost::property_map<graph_t, boost::edge_index_t>::const_type edge_index_map_t;
        edge_index_map_t edge_index_map = get(boost::edge_index, graph);

        std::vector<int> edge_component(num_edges(graph));
        std::vector<vertex_t> articulation_points;
        int num_components = static_cast<int>(biconnected_components(
            graph,
            make_iterator_property_map(edge_component.begin(), edge_index_map),
            std::back_inserter(articulation_points)
        ).first);

        bctree_t tree;

        std::vector<bctree_vertex> b_nodes;
        std::unordered_map<vertex_t, bctree_vertex> c_nodes;
        for (int i = 0; i < num_components; ++i) {
            bctree_vertex node = add_vertex(tree);
            tree[node].node_type = B_NODE;
            b_nodes.push_back(node);
        }
        for (const vertex_t& articulation_point : articulation_points) {
            bctree_vertex node = add_vertex(tree);
            tree[node].node_type = C_NODE;
            tree[node].articulation_point = articulation_point;
            c_nodes.insert({articulation_point, node});
        }

        std::vector<std::unordered_map<vertex_t, vertex_t>> vertex_mappings(num_components);
        for (auto [ei, ei_end] = edges(graph); ei != ei_end; ++ei) {
            int component_id = edge_component[get(edge_index_map, *ei)];
            bctree_vertex b_node = b_nodes[component_id];
            auto& component = tree[b_node].bi_component;
            vertex_t source = boost::source(*ei, graph);
            vertex_t target = boost::target(*ei, graph);

            auto& vertex_mapping = vertex_mappings[component_id];
            if (!vertex_mapping.contains(source)) {
                vertex_mapping.insert({source, add_vertex(component)});
                if (c_nodes.contains(source)) {
                    add_edge(b_node, c_nodes[source], tree);
                }
            }
            if (!vertex_mapping.contains(target)) {
                vertex_mapping.insert({target, add_vertex(component)});
                if (c_nodes.contains(target)) {
                    add_edge(b_node, c_nodes[target], tree);
                }
            }

            add_edge(vertex_mapping[source], vertex_mapping[target], static_cast<int>(num_edges(component)), component);
        }

        for (int component_id = 0; component_id < num_components; ++component_id) {
            typedef boost::property_map<graph_t, boost::vertex_index_t>::type vertex_index_map_t;

            const graph_t& component = tree[b_nodes[component_id]].bi_component;
            vertex_index_map_t vertex_index_map = get(boost::vertex_index, component);
            std::vector<vertex_t>& backwards_mapping = tree[b_nodes[component_id]].original_vertices;
            backwards_mapping.resize(num_vertices(component));

            for (const auto& [global_v, local_v] : vertex_mappings[component_id]) {
                int local_v_index = static_cast<int>(get(vertex_index_map, local_v));
                backwards_mapping[local_v_index] = global_v;
            }
        }

        return tree;
    }
};

#endif //OKP_RECOGNITION_BICOMPONENT_SOLVER_H
