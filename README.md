<h1 align="center"> Practical aspects of recognising outer $k$-planar graphs </h1>

This repository contains code for the bachelor thesis `Practical aspects of recognising outer k-planar graphs` by Ivan Shevchenko under the mentorship of [Prof. Dr. Alexander Wolff](https://www.informatik.uni-wuerzburg.de/algo/team/wolff-alexander/).

---

### Structure
This repository is structured in the following way:

```
    .
    ├── cmake            # Folder containing additional cmake files for finding the libraries
    ├── data             # Contains test graphs and the results of the experiments
    ├── include          # Header files for implemented algorithms
    ├── scripts          # Scripts for automated rendering visualisations and experiment setups
    ├── src              # Source files for implemented algorithms
    ├── thesis           # TEX source files for bachelor thesis 
    ├── .gitignore
    ├── CMakeLists.txt   # Defines the build process of the project
    ├── requirements.txt # Specifies requirements for running scripts
    └── README.md
```

### Prerequirements

To use the implemented algorithms, the following is required:
- `cmake` version `3.21.1` or higher;
- any compiler that supports `C++20`;
- `boost` library;
- `gurobi` library, installed according to the official manual;
- `kissat` library, with the path to the installation directory added to the environmental variable `PATH`.

Additionally, for running scripts requires:
- `python3`;
- third-party libraries that can be installed with
```bash
python3 -m pip install -r requirements.txt
```

### Compilation

To compile the implemented algorithms, run the following commands:
```bash
mkdir build; cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

The compiled executables will be placed in the directory `bin` under the root level of the project. The folder will contain four executables with different configurations of methods based on `ILP` and `SAT`:
- `okp-recognition`: is compiled without any optimisations;
- `okp-recognition-obj`: is compiled with additional symmetry-braking term in an objective function of the `ILP`;
- `okp-recognition-exact`: is compiled with additional $16$ constraints for both `ILP` and `SAT`;
- `okp-recognition-exact-obj`: is compiled with both optimisations switched on.

### Usage

All the above-mentioned executables have the same command line interface:
```
Usage:
    okp-recognition[-exact|-obj|-exact-obj] <input_graph> [options]
Allowed options:
  -h [ --help ]            Show help message
  -i [ --input-graph ] arg Input graph in Graphviz format. Required.
  -o [ --output-file ] arg Path to output file used to save graph drawing in 
                           Graphviz format. Default: "" (output is ignored)
  -m [ --method ] arg (=0) Method to use for calculating the drawing of the 
                           input graph. One of the ILP, SAT, DP. Default: ILP
  -b [ --no-bct ]          Include this to disable biconnected decomposition 
                           before passing it to the solver
```

To convert the algorithm's output into a picture, a `neato` layout engine is required.

### Example

As an example of the execution, we run the `ILP` algorithm for [Cubical graph](https://houseofgraphs.org/graphs/1022):
```bash
bin/okp-recognition "graph G {  0; 0 -- 1; 0 -- 2; 0 -- 3; 1; 1 -- 5; 1 -- 6; 2; 2 -- 5; 2 -- 7; 3; 3 -- 6; 3 -- 7; 4; 4 -- 5; 4 -- 6; 4 -- 7; 5; 6; 7;}"\
 -m ILP -o data/cubical.dot
```
After converting the result into `PNG` using:
```bash
neato -Tpng -odata/cubical.png data/cubical.dot
```
we get the following drawing.

![A cubical graph drawing by `ILP`-based algorithm](data/cubical.png)
