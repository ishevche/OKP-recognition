#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/fileformats/SvgPrinter.h>
#include "graphIO.h"

std::istream &operator>>(std::istream &in, ogdf::Graph &G) {
    G.clear();
    size_t nodes_count, edges_count;
    in >> nodes_count >> edges_count;
    std::vector<ogdf::node> nodes;
    nodes.reserve(nodes_count);
    for (int i = 0; i < nodes_count; ++i) {
        nodes.push_back(G.newNode(i));
    }
    for (int i = 0; i < edges_count; ++i) {
        size_t u, v;
        in >> u >> v;
        G.newEdge(nodes[u - 1], nodes[v - 1], i);
    }
    return in;
}


void addBezierCurve(ogdf::GraphAttributes &GA, ogdf::edge e,
                    const ogdf::DPoint &start_point,
                    const ogdf::DPoint &control_point,
                    const ogdf::DPoint &end_point,
                    int segments) {
    ogdf::DPolyline &bends = GA.bends(e);
    for (int i = 1; i < segments; ++i) {
        double t = i / static_cast<double>(segments);
        double x = (1 - t) * (1 - t) * start_point.m_x + 2 * (1 - t) * t * control_point.m_x + t * t * end_point.m_x;
        double y = (1 - t) * (1 - t) * start_point.m_y + 2 * (1 - t) * t * control_point.m_y + t * t * end_point.m_y;
        bends.pushBack(ogdf::DPoint(x, y));
    }
}

int count_crossing(const ogdf::edge &edge, const ogdf::Graph &graph, const ogdf::NodeArray<int> &node_order) {
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

void drawSVG(std::ostream &out, const ogdf::Graph &G,
             const std::vector<ogdf::node> &ordering) {
    ogdf::GraphAttributes GA(G);
    ogdf::NodeArray<int> node_order(G);
    GA.directed() = false;

    for (int i = 0; i < ordering.size(); ++i) {
        node_order[ordering[i]] = i;
        GA.x(ordering[i]) = node_order[ordering[i]] * 100;
    }

    ogdf::EdgeArray<int> crossing_number(G);
    int maximal_local_crossing = 0;
    for (ogdf::edge edge: G.edges) {
        int crossings = count_crossing(edge, G, node_order);
        crossing_number[edge] = crossings;
        maximal_local_crossing = std::max(maximal_local_crossing, crossings);
    }

    GA.addAttributes(ogdf::GraphAttributes::edgeStyle);
    for (ogdf::edge edge: G.edges) {
        double source_x = GA.x(edge->source());
        double target_x = GA.x(edge->target());
        ogdf::DPoint source_point(source_x, 0);
        ogdf::DPoint control_point((source_x + target_x) / 2,
                                   std::abs(source_x - target_x) / 2);
        ogdf::DPoint target_point(target_x, 0);
        addBezierCurve(GA, edge, source_point, control_point, target_point, 20);
        if (crossing_number[edge] == maximal_local_crossing) {
            GA.strokeColor(edge) = ogdf::Color::Name::Red;
        }
    }

    ogdf::GraphIO::SVGSettings svgSettings;
    ogdf::SvgPrinter printer(GA, svgSettings);
    printer.draw(out);
}
