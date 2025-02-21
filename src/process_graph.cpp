#include <iostream>
#include <filesystem>
#include <ogdf/fileformats/GraphIO.h>
#include "process_graph.h"
//#include "bicomponents.h"
#include "solvers.h"
#include "timer.h"

//void process_graph(const std::string& graph_string) {
//    ogdf::Graph graph;
//    std::stringstream graph_ss(graph_string);
//    ogdf::GraphIO::readGraph6(graph, graph_ss);
//
//    std::vector<ogdf::node> ILP_order;
//    SolverParams params;
//
//    auto start = get_current_time_fenced();
//    int components = solve_for_each_component(params, ILPSolver);
//    auto end = get_current_time_fenced();
//
//    std::cout << " (" << components << ")" << std::endl;
//    if (components == 1) {
//        return;
//    }
//    int actual_crossings = max_local_crossing(graph, params.ordering);
//    if (actual_crossings != params.number_of_crossings) {
//        std::cout << "ERROR (returned crossing number does not equal actual): returned "
//                  << params.number_of_crossings << " actual " << actual_crossings << std::endl;
//    }
//    std::cout << "ILP (bctree): " << params.number_of_crossings << " " << to_ns(end - start) << std::endl;
//
//    start = get_current_time_fenced();
//    ILPSolver(params);
//    end = get_current_time_fenced();
//
//    actual_crossings = max_local_crossing(graph, params.ordering);
//    if (actual_crossings != params.number_of_crossings) {
//        std::cout << "ERROR (returned crossing number does not equal actual): returned "
//                  << params.number_of_crossings << " actual " << actual_crossings << std::endl;
//    }
//    std::cout << "ILP (basic) : " << params.number_of_crossings << " " << to_ns(end - start) << std::endl;
//}


int count_edge_crossing(Edge edge, const Graph &graph){
    int result = 0;
    int edge_src = graph[boost::source(edge, graph)].order;
    int edge_trg = graph[boost::target(edge, graph)].order;
    if (edge_src > edge_trg) { std::swap(edge_src, edge_trg); }
    for (Edge other : boost::make_iterator_range(boost::edges(graph))) {
        int other_src = graph[boost::source(other, graph)].order;
        int other_trg = graph[boost::target(other, graph)].order;
        if (other_src > other_trg) { std::swap(other_src, other_trg); }
        bool src_between = edge_src < other_src && other_src < edge_trg;
        bool trg_between = edge_src < other_trg && other_trg < edge_trg;
        result += (src_between && other_trg > edge_trg) ||
                  (trg_between && other_src < edge_src);
    }
    return result;
}

int max_local_crossing(const Graph &graph) {
    int result = 0;
    for (Edge edge : boost::make_iterator_range(boost::edges(graph))) {
        int edge_crossings = count_edge_crossing(edge, graph);
        result = std::max(result, edge_crossings);
    }
    return result;
}

