#include <ogdf/basic/basic.h>
#include <ogdf/fileformats/GraphIO.h>
#include <gurobi_c++.h>
#include <filesystem>
#include "graphIO.h"
#include "bcTree.h"
#include "solvers.h"
#include "timer.h"

void draw(const ogdf::Graph &graph, const std::vector<ogdf::node> &ordering, const std::string &graph_string,
        const std::string &file_name);

int max_local_crossing(const ogdf::Graph &graph, const std::vector<ogdf::node> &ordering);

int main() {
    std::ifstream in("data/graphs.g6");
    std::string input_string;
    std::vector<std::string> graph_strings;
    while (in >> input_string) {
        graph_strings.push_back(input_string);
    }
    in.close();

    for (int i = 0; i < graph_strings.size(); ++i) {
        const std::string &graph_string = graph_strings[i];
        std::cout << std::setw(4) << i + 1 << ": " << graph_string;
        ogdf::Graph graph;
        std::stringstream graph_ss(graph_string);
        ogdf::GraphIO::readGraph6(graph, graph_ss);

        std::vector<ogdf::node> ILP_order;
        SolverParams params{graph, ILP_order};
	
        auto start = get_current_time_fenced();
        int components = process(params, ILPSolver);
        auto end = get_current_time_fenced();

        std::cout << " (" << components << ")" << std::endl;

        if (components == 1) {
            continue;
        }

        int actual_crossings = max_local_crossing(graph, params.ordering);
        if (actual_crossings != params.number_of_crossings) {
            std::cout << "ERROR (returned crossing number does not equal actual): returned " 
                << params.number_of_crossings << " actual " << actual_crossings << std::endl;
        }

        std::cout << "ILP (bctree): " << params.number_of_crossings << " " << to_ns(end - start) << std::endl;

        start = get_current_time_fenced();
        ILPSolver(params);
        end = get_current_time_fenced();

        actual_crossings = max_local_crossing(graph, params.ordering);
        if (actual_crossings != params.number_of_crossings) {
            std::cout << "ERROR (returned crossing number does not equal actual): returned " 
                << params.number_of_crossings << " actual " << actual_crossings << std::endl;
        }

        std::cout << "ILP (basic) : " << params.number_of_crossings << " " << to_ns(end - start) << std::endl;
    }
    return 0;
}

int count_edge_crossing(const ogdf::edge &edge, const ogdf::Graph &graph, const ogdf::NodeArray<int> &node_order) {
    int result = 0;
    int edge_src = node_order[edge->source()];
    int edge_trg = node_order[edge->target()];
    if (edge_src > edge_trg) { std::swap(edge_src, edge_trg); }
    for (const ogdf::edge &other: graph.edges) {
        int other_src = node_order[other->source()];
        int other_trg = node_order[other->target()];
        if (other_src > other_trg) { std::swap(other_src, other_trg); }
        bool src_between = edge_src < other_src && other_src < edge_trg;
        bool trg_between = edge_src < other_trg && other_trg < edge_trg;
        result += (src_between && other_trg > edge_trg) ||
            (trg_between && other_src < edge_src);
    }
    return result;
}

int max_local_crossing(const ogdf::Graph &graph, const std::vector<ogdf::node> &ordering) {
    ogdf::NodeArray<int> node_order(graph);
    for (int i = 0; i < ordering.size(); ++i) {
        node_order[ordering[i]] = i;
    }
    int result = 0;
    for (ogdf::edge edge: graph.edges) {
        int edge_crossings = count_edge_crossing(edge, graph, node_order);
        result = std::max(result, edge_crossings);
    }
    return result;
}

void draw(const ogdf::Graph &graph, const std::vector<ogdf::node> &ordering,
        const std::string &graph_string, const std::string &file_name) {
    std::filesystem::create_directories("data/out/" + graph_string);
    std::ofstream out{"data/out/" + graph_string + "/" + file_name};
    drawSVG(out, graph, ordering);
    out.close();
}

//    Graphs:
// Number of Vertices <= 10
// Minimum degree > 1
// Connected
// Number of Edges >= 2 * Number of Vertices
