#! /usr/bin/env python3
from visualisation_methods import (visualise_biconnected,
                                   visualise_comparison,
                                   visualise_comparison_by_cr,
                                   visualise_ilp_optimisations,
                                   visualise_sat_optimisations,
                                   visualise_outcomes)

visualise_biconnected("data/results_biconnected.csv",
                      "thesis/Figures/experiments/bctree_{method}.pdf",
                      ["ilp", "sat", "dp"])
visualise_comparison("data/results_600.csv", "thesis/Figures/experiments/comparison.pdf")
visualise_comparison_by_cr("data/results_600.csv",
                           "thesis/Figures/experiments/comparison_cr{crossing_number}.pdf",
                           [2, 3, 4, 5])
visualise_ilp_optimisations("data/results_ilp_optimisations.csv", "thesis/Figures/experiments/ilp_optimisations.pdf")
visualise_sat_optimisations("data/results_sat_optimisations.csv", "thesis/Figures/experiments/sat_optimisations.pdf")
visualise_outcomes("data/results_600.csv",
                   "thesis/Figures/experiments/outcome_{method}.pdf",
                   ["ilp", "sat", "dp"])
