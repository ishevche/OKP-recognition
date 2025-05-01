# Data

The data used was obtained from The House of Graphs[^hog] on the 20th of April 2025 with the following query parameters:

[^hog]: K. Coolsaet, S. D'hondt and J. Goedgebeur, House of Graphs 2.0: A database of interesting graphs and more, Discrete Applied Mathematics, 325:97-107, 2023. Available at https://houseofgraphs.org

 - Number of Vertices <= $10$
 - Connected

The search resulted in $2007$ graphs. We downloaded them in Graphviz format and placed them in this folder as a `graphs.txt` file.

Among those $2007$ graphs, $1326$ are biconnected and $681$ are not. The latter is used for experiments demonstrating the effectiveness of bicomponent decomposition. The former graphs are used for every other experiment.

Files with the `CSV` extension in this folder contain the results of the following experiments:
 - `results_60.csv`: comparison of all methods on biconnected graphs, was run with timeout $1$ minute (not used in the thesis, as was extended to `results_600.csv`);
 - `results_600.csv`: similar to `results_60.csv`, but the timeout was $10$ minutes;
 - `results_biconnected.csv`: experiment was run on only non-biconnected graphs and all methods in two configurations; one of them used biconnected decomposition, another did not;
 - `results_ilp_optimisations.csv`: experiment comparing optimisations for `ILP`; was run on all biconnected graphs;
 - `results_sat_optimisations.csv`: similar to `results_ilp_optimisations.csv`, but for `SAT` optimisations.

Files `cubical.dot` and `cubical.png` demonstrate the result returned from the code. See `README.md` on the root level of the project. 
