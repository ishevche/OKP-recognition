#ifndef OKP_RECOGNITION_PREPROCESS_H
#define OKP_RECOGNITION_PREPROCESS_H

#include <functional>
#include <ogdf/basic/basic.h>
#include "solvers.h"

int process(SolverParams &solverParams, const std::function<void (SolverParams&)>& solver);

#endif //OKP_RECOGNITION_PREPROCESS_H
