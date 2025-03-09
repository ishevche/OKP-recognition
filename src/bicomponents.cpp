#include <boost/graph/biconnected_components.hpp>

#include "bicomponents.h"

bctree_t decompose(const Graph &graph) {
    typedef boost::property_map<Graph, boost::edge_index_t>::const_type edge_index_map_t;
    edge_index_map_t edge_index_map = boost::get(boost::edge_index, graph);

    std::vector<size_t> edge_component(boost::num_edges(graph));
    std::vector<Vertex> articulation_points;
    auto [num_components, _] = boost::biconnected_components(
            graph,
            boost::make_iterator_property_map(edge_component.begin(), edge_index_map),
            std::back_inserter(articulation_points)
    );

    bctree_t tree;

    std::vector<bctree_vertex> b_nodes;
    std::unordered_map<Vertex, bctree_vertex> c_nodes;
    for (int i = 0; i < num_components; ++i) {
        bctree_vertex node = boost::add_vertex(tree);
        tree[node].node_type = B_NODE;
        b_nodes.push_back(node);
    }
    for (const Vertex &articulation_point: articulation_points) {
        bctree_vertex node = boost::add_vertex(tree);
        tree[node].node_type = C_NODE;
        tree[node].articulation_point = articulation_point;
        c_nodes.insert({articulation_point, node});
    }

    std::vector<std::unordered_map<Vertex, Vertex>> vertex_mappings(num_components);
    for (auto [ei, ei_end] = boost::edges(graph); ei != ei_end; ++ei) {
        size_t component_id = edge_component[boost::get(edge_index_map, *ei)];
        bctree_vertex b_node = b_nodes[component_id];
        auto &component = tree[b_node].bi_component;
        Vertex source = boost::source(*ei, graph);
        Vertex target = boost::target(*ei, graph);

        auto &vertex_mapping = vertex_mappings[component_id];
        if (!vertex_mapping.contains(source)) {
            vertex_mapping.insert({source, boost::add_vertex(component)});
            if (c_nodes.contains(source)) {
                boost::add_edge(b_node, c_nodes[source], tree);
            }
        }
        if (!vertex_mapping.contains(target)) {
            vertex_mapping.insert({target, boost::add_vertex(component)});
            if (c_nodes.contains(target)) {
                boost::add_edge(b_node, c_nodes[target], tree);
            }
        }

        boost::add_edge(vertex_mapping[source], vertex_mapping[target], boost::num_edges(component), component);
    }

    for (size_t component_id = 0; component_id < num_components; ++component_id) {
        typedef boost::property_map<Graph, boost::vertex_index_t>::type vertex_index_map_t;

        const Graph &component = tree[b_nodes[component_id]].bi_component;
        vertex_index_map_t vertex_index_map = boost::get(boost::vertex_index, component);
        std::vector<size_t> &backwards_mapping = tree[b_nodes[component_id]].original_vertices;
        backwards_mapping.resize(boost::num_vertices(component));

        for (const auto &[global_v, local_v]: vertex_mappings[component_id]) {
            size_t local_v_index = boost::get(vertex_index_map, local_v);
            backwards_mapping[local_v_index] = global_v;
        }
    }

    return tree;
}
