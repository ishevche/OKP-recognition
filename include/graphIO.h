#ifndef OKP_RECOGNITION_GRAPHIO_H
#define OKP_RECOGNITION_GRAPHIO_H

#include <iostream>
#include <vector>
#include <ogdf/basic/Graph.h>

std::istream &operator>>(std::istream &in, ogdf::Graph &G);

void draw_svg(std::ostream &out, const ogdf::Graph &G,
              const std::vector<ogdf::node> &ordering);


#endif //OKP_RECOGNITION_GRAPHIO_H
