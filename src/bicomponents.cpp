#include <ogdf/decomposition/BCTree.h>
#include <stack>
#include "bicomponents.h"

int solve_for_each_component(SolverParams &solverParams,
                             const std::function<void(SolverParams &)> &solver) {
    ogdf::Graph graph = solverParams.graph;
    ogdf::BCTree bcTree(graph);

     if (bcTree.numberOfBComps() == 1) {
        //solver(solverParams);
        return 1;
    }

   ogdf::node root_c_node = nullptr;
    for (ogdf::node node: bcTree.bcTree().nodes) {
        if (bcTree.typeOfBNode(node) == ogdf::BCTree::BNodeType::CComp) {
            root_c_node = node;
            break;
        }
    }

    std::vector<ogdf::node> &ordering = solverParams.ordering;
    ordering.clear();
    ordering.reserve(graph.numberOfNodes());
    ordering.push_back(bcTree.original(bcTree.cutVertex(root_c_node, root_c_node)));

    std::stack<ogdf::node> dfs_stack;
    ogdf::NodeArray<bool> visited(graph, false);
    dfs_stack.push(root_c_node);

    while (!dfs_stack.empty()) {
        ogdf::node current = dfs_stack.top();
        dfs_stack.pop();

        ogdf::node previous = nullptr;
        for (ogdf::adjEntry adjEntry: current->adjEntries) {
            ogdf::node neighbour = adjEntry->twinNode();
            if (visited[neighbour]) {
                previous = neighbour;
            } else {
                dfs_stack.push(neighbour);
            }
        }

        if (bcTree.typeOfBNode(current) == ogdf::BCTree::BNodeType::BComp) {
            SolverParams partial_solution = solve_biconnected_component(bcTree, current, solverParams, solver);
            solverParams.number_of_crossings = std::max(solverParams.number_of_crossings,
                                                        partial_solution.number_of_crossings);

            std::vector<ogdf::node> &part_ord = partial_solution.ordering;
            ogdf::node orig_cut_node = bcTree.original(bcTree.cutVertex(previous, previous));

            auto insert_iter = std::find(ordering.begin(), ordering.end(), orig_cut_node) + 1;
            auto rotate_iter = std::find(part_ord.begin(), part_ord.end(), orig_cut_node);
            std::rotate(part_ord.begin(), rotate_iter, part_ord.end());
            ordering.insert(insert_iter, part_ord.begin() + 1, part_ord.end());
        }
        visited[current] = true;
    }

    return bcTree.numberOfBComps();
}

SolverParams solve_biconnected_component(ogdf::BCTree &bc_tree, ogdf::node b_component,
                                         const SolverParams &solverParams,
                                         const std::function<void(SolverParams &)> &solve) {
    const ogdf::Graph &originalGraph = solverParams.graph;
    ogdf::Graph component;
    ogdf::NodeArray<ogdf::node> mapping_comp_to_gr(component, nullptr);
    ogdf::NodeArray<ogdf::node> mapping_gr_to_comp(originalGraph, nullptr);

    for (ogdf::edge edge: bc_tree.hEdges(b_component)) {
        ogdf::node src = bc_tree.original(edge->source());
        ogdf::node trg = bc_tree.original(edge->target());

        ogdf::node new_src = mapping_gr_to_comp[src];
        ogdf::node new_trg = mapping_gr_to_comp[trg];
        if (!new_src) {
            new_src = component.newNode();
            mapping_gr_to_comp[src] = new_src;
            mapping_comp_to_gr[new_src] = src;
        }
        if (!new_trg) {
            new_trg = component.newNode();
            mapping_gr_to_comp[trg] = new_trg;
            mapping_comp_to_gr[new_trg] = trg;
        }

        component.newEdge(new_src, new_trg);
    }

    SolverParams params{component, {}, solverParams.number_of_crossings, false};
    solve(params);
    std::vector<ogdf::node> original_ordering;
    original_ordering.reserve(component.numberOfNodes());
    for (ogdf::node new_node: params.ordering) {
        original_ordering.push_back(mapping_comp_to_gr[new_node]);
    }
    params.ordering = original_ordering;
    return params;
}
