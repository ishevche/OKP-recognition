#include <ogdf/basic/basic.h>
#include <ogdf/fileformats/GraphIO.h>
#include <filesystem>
#include "graphIO.h"
#include "solvers.h"
#include "timer.h"

void draw(const ogdf::Graph &graph, const ogdf::NodeArray<int> &ordering, const std::string &graph_string,
          const std::string &file_name);

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
        std::cout << i << "/776 " << graph_string << ":" << std::endl;
        ogdf::Graph graph;
        std::stringstream graph_ss(graph_string);
        ogdf::GraphIO::readGraph6(graph, graph_ss);

        ogdf::NodeArray<int> ILP_order(graph);
        SolverParams params{graph, ILP_order};
        auto start = get_current_time_fenced();
        ILPSolver(params);
        auto end = get_current_time_fenced();

        std::cout << "ILP: " << params.number_of_crossings << " " << to_ns(end-start) << std::endl;
        std::cout << "SAT: 0 0 0\nSAT: 0 0 0\n" << std::endl;

//        ogdf::NodeArray<int> SAT_order(graph);
//        start = get_current_time_fenced();
//        bool found_solution = SATSolver(graph, ILP_bound, SAT_order);
//        end = get_current_time_fenced();
//        std::cout << "SAT: " << ILP_bound << " " << found_solution << " " << to_ns(end-start) << std::endl;
//        if (!found_solution && ILP_bound < 7) {
//            draw(graph, ILP_order, graph_string, "ILP(" + std::to_string(ILP_bound) + ").svg");
//            while (!found_solution) {
//                start = get_current_time_fenced();
//                found_solution = SATSolver(graph, ++ILP_bound, SAT_order);
//                end = get_current_time_fenced();
//                std::cout << "SAT: " << ILP_bound << " " << found_solution << " " << to_ns(end-start) << std::endl;
//            }
//            draw(graph, SAT_order, graph_string, "SAT(" + std::to_string(ILP_bound) + ").svg");
//        } else {
//            start = get_current_time_fenced();
//            found_solution = SATSolver(graph, --ILP_bound, SAT_order);
//            end = get_current_time_fenced();
//            std::cout << "SAT: " << ILP_bound << " " << found_solution << " " << to_ns(end-start) << std::endl;
//            if (found_solution) {
//                draw(graph, ILP_order, graph_string, "ILP(" + std::to_string(ILP_bound) + ").svg");
//                while (found_solution) {
//                    draw(graph, SAT_order, graph_string, "SAT(" + std::to_string(ILP_bound) + ").svg");
//                    start = get_current_time_fenced();
//                    found_solution = SATSolver(graph, --ILP_bound, SAT_order);
//                    end = get_current_time_fenced();
//                    std::cout << "SAT: " << ILP_bound << " " << found_solution << " " << to_ns(end-start) << std::endl;
//                }
//            }
//        }
//        std::cout << std::endl;
    }
    return 0;
}

void draw(const ogdf::Graph &graph, const ogdf::NodeArray<int> &ordering,
          const std::string &graph_string, const std::string &file_name) {
    std::filesystem::create_directories("data/out/" + graph_string);
    std::ofstream out{"out/" + graph_string + "/" + file_name};
    drawSVG(out, graph, ordering);
    out.close();
}

//    Graphs:
// Number of Vertices <= 10
// Minimum degree > 1
// Connected
// Number of Edges >= 2 * Number of Vertices
