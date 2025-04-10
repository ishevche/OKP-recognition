# Data

The data used was obtained from The House of Graphs[^hog] on 10th of April 2025 with the following query parameters:

[^hog]: K. Coolsaet, S. D'hondt and J. Goedgebeur, House of Graphs 2.0: A database of interesting graphs and more, Discrete Applied Mathematics, 325:97-107, 2023. Available at https://houseofgraphs.org

 - Number of Vertices <= 10
 - Minimum Degree > 1
 - Connected
 - Number of Edges >= 2 * Number of Vertices

The search resulted in 779 graphs. We downloaded them in Graphviz format and places in `data` folder as a `graphs.txt` file.