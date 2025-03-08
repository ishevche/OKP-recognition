#include <boost/graph/depth_first_search.hpp>
#include "solvers.h"
#include "bicomponents.h"

class dfs_visitor : public boost::default_dfs_visitor {
public:
    dfs_visitor(SolverParams &params, std::function<void(SolverParams &)> &solver)
            : solver_params(params), solver(solver) {}

    void discover_vertex(bctree_vertex node, const bctree_t &graph) {
        if (!solver_params.converged) { return; }

        if (graph[node].node_type == C_NODE) { return; }
        std::vector<Vertex> component_order;
        SolverParams component_params{
                graph[node].bi_component,
                component_order,
                solver_params.number_of_crossings,
                false
        };
        solver(component_params);

        typedef boost::property_map<Graph, boost::vertex_index_t>::type vertex_index_map_t;
        vertex_index_map_t vertex_index_map = boost::get(boost::vertex_index, graph[node].bi_component);
        std::transform(component_order.begin(), component_order.end(), component_order.begin(),
                       [&graph, &node, &vertex_index_map](const Vertex &v) {
                           return graph[node].original_vertices[boost::get(vertex_index_map, v)];
                       });

        auto cut_node = predecessors.find(node);
        if (cut_node == predecessors.end()) {
            solver_params.ordering.clear();
            solver_params.ordering.insert(solver_params.ordering.end(), component_order.begin(), component_order.end());
            solver_params.converged = component_params.converged;
            solver_params.number_of_crossings = component_params.number_of_crossings;
            return;
        }

        Vertex articulation_point = graph[cut_node->second].articulation_point;
        auto rotate_iter = std::find(component_order.begin(), component_order.end(), articulation_point);
        std::rotate(component_order.begin(), rotate_iter, component_order.end());

        std::vector<Vertex> &global_order = solver_params.ordering;
        auto insert_iter = ++std::find(global_order.begin(), global_order.end(), articulation_point);
        global_order.insert(insert_iter, ++component_order.begin(), component_order.end());

        solver_params.converged = component_params.converged;
        solver_params.number_of_crossings = std::max(component_params.number_of_crossings,
                                                     solver_params.number_of_crossings);
    }

    void tree_edge(Edge edge, const bctree_t &graph) {
        predecessors[boost::target(edge, graph)] = boost::source(edge, graph);
    }

private:
    SolverParams &solver_params;
    std::function<void(SolverParams &)> &solver;
    std::map<bctree_vertex, bctree_vertex> predecessors;
};

void bicomponent_solver(SolverParams &params, std::function<void(SolverParams &)> solver) {
    params.converged = true;
    params.ordering.clear();

    dfs_visitor visitor(params, solver);

    bctree_t bctree = decompose(params.graph);
    std::vector<boost::default_color_type> color_map(num_vertices(bctree));
    auto color = make_iterator_property_map(color_map.begin(), boost::get(boost::vertex_index, bctree));

    boost::depth_first_search(bctree, visitor, color, 0);
}
