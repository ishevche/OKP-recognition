#ifndef OKP_RECOGNITION_GRAPHIO_H
#define OKP_RECOGNITION_GRAPHIO_H

#include <iostream>
#include <ogdf/basic/Graph.h>

std::istream &operator>>(std::istream &in, ogdf::Graph &G);

void drawSVG(std::ostream &out, const ogdf::Graph &G,
             const ogdf::NodeArray<int> &node_order);


#endif //OKP_RECOGNITION_GRAPHIO_H
