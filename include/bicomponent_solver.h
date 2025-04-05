#ifndef OKP_RECOGNITION_BICOMPONENT_SOLVER_H
#define OKP_RECOGNITION_BICOMPONENT_SOLVER_H

#include <boost/graph/depth_first_search.hpp>
#include "bctree.h"
#include "solver.h"

template <class sub_solver>
class bicomponent_solver : public solver {
public:
    explicit bicomponent_solver(const Graph& graph, size_t crossing_number = 0)
        : solver(graph, crossing_number) {}

    bool solve() override {
        vertex_order.clear();

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
        size_t& crossing_number;
        std::vector<Vertex>& vertex_order;
        std::map<bctree_vertex, bctree_vertex> predecessors;
        bool& ok;

    public:
        dfs_visitor(std::vector<Vertex>& vertex_order, size_t& crossing_number, bool& ok)
            : crossing_number(crossing_number), vertex_order(vertex_order), ok(ok) {}

        void discover_vertex(bctree_vertex node, const bctree_t& tree) {
            if (!ok) { return; }

            if (tree[node].node_type == C_NODE) { return; }
            sub_solver component_solver(tree[node].bi_component, crossing_number);
            ok = component_solver.solve();

            if (!ok) { return; }

            std::vector<Vertex>& component_order = component_solver.vertex_order;

            typedef boost::property_map<Graph, boost::vertex_index_t>::const_type vertex_index_map_t;
            vertex_index_map_t component_index_map = get(boost::vertex_index, tree[node].bi_component);
            std::transform(component_order.begin(), component_order.end(), component_order.begin(),
                           [&tree, &node, &component_index_map](const Vertex& v) {
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

            Vertex articulation_point = tree[cut_node->second].articulation_point;
            auto rotate_iter = std::ranges::find(component_order, articulation_point);
            std::ranges::rotate(component_order, rotate_iter);

            auto insert_iter = ++std::ranges::find(vertex_order, articulation_point);
            vertex_order.insert(insert_iter, ++component_order.begin(), component_order.end());

            crossing_number = std::max(component_solver.crossing_number, crossing_number);
        }

        void tree_edge(const Edge& edge, const bctree_t& graph) {
            predecessors[target(edge, graph)] = source(edge, graph);
        }
    };
};

#endif //OKP_RECOGNITION_BICOMPONENT_SOLVER_H
