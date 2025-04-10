#! /usr/bin/env python3
import argparse
import re
import os
import asyncio

import pandas as pd
import pydot
import networkx as nx


def get_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("graphs_file", help="file containing graphs in Graphviz format", default="data/graphs.txt")
    parser.add_argument("-p", "--subprocesses", help="number of subprocesses to use at the same time",
                        dest="subprocesses", type=int, default=12)
    parser.add_argument("-b", "--bin-executable", help="path to an executable",
                        dest="exec", default="bin/okp-recognition")
    parser.add_argument("-o", "--output", help="output file for all evaluations",
                        dest="out", default="data/results.csv")
    return parser.parse_args()


def dot_to_g6(graph: str):
    pydot_graph = pydot.graph_from_dot_data(graph)
    nx_graph = nx.Graph(nx.nx_pydot.from_pydot(pydot_graph[0]))
    return nx.to_graph6_bytes(nx_graph, header=False).decode().strip()


def initialise_df(graphs_file: str, out_file: str):
    with open(graphs_file, "r") as in_file:
        graphs_string = in_file.read()
    graphs_strings = re.findall(r'graph\s+\w\s+\{.*?\}', graphs_string)
    graphs = []
    for graph in graphs_strings:
        graphs.append((graph, dot_to_g6(graph)))
    df = pd.DataFrame(data=graphs, columns=["graph_dot", "graph_g6"])
    df = df.reindex(columns=["graph_dot", "graph_g6",
                             "ilp_cr", "ilp_time", "ilp_success",
                             "sat_cr", "sat_time", "sat_success",
                             "okp_cr", "okp_time", "okp_success"])
    df.to_csv(out_file, index=False)
    return df


async def gather_results(df: pd.DataFrame, df_lock, executable: str, out_file: str):
    save_columns = list(filter(lambda x: not x.startswith("done"), df.columns))
    while True:
        async with df_lock:
            todo_graphs_index = df[~df["done"]].index
            if todo_graphs_index.empty:
                break
            graph_index = todo_graphs_index[0]
            df.at[graph_index, "done"] = True
            methods = []
            if not df.at[graph_index, "done_ilp"]: methods.append("ilp")
            if not df.at[graph_index, "done_sat"]: methods.append("sat")
            if not df.at[graph_index, "done_okp"]: methods.append("okp")
            graph_str = df.at[graph_index, "graph_dot"]
            graph_g6 = df.at[graph_index, "graph_g6"]
        for method in methods:
            subprocess = await asyncio.create_subprocess_shell(f"{executable} \"{graph_str}\" -m {method}",
                                                               stdout=asyncio.subprocess.PIPE,
                                                               stderr=asyncio.subprocess.PIPE)
            print(f"Running {method} for graph {graph_index} ({graph_g6})...")
            stdout, stderr = await subprocess.communicate()
            print(f"{method} for graph {graph_index} ({graph_g6}) completed.")
            if subprocess.returncode != 0:
                print(f"{method} for {graph_g6} failed with return code {subprocess.returncode}:\n{stderr.decode()}")
            else:
                solved, cr, time = map(int, stdout.decode().split(" "))
                async with df_lock:
                    df.at[graph_index, method + "_cr"] = cr
                    df.at[graph_index, method + "_time"] = time
                    df.at[graph_index, method + "_success"] = bool(solved)
                    df.to_csv(out_file, index=False, columns=save_columns)
        print(f"Graph {graph_index} ({graph_g6}) is completed.")
    print("Subroutine finished.")


async def run_experiments(graphs_file: str, subprocesses: int, executable: str, out_file: str):
    if os.path.exists(out_file):
        df = pd.read_csv(out_file)
    else:
        df = initialise_df(graphs_file, out_file)
    df["done"] = df.notna().all(axis=1)
    df["done_ilp"] = df[["ilp_cr", "ilp_time", "ilp_success"]].notna().all(axis=1)
    df["done_sat"] = df[["sat_cr", "sat_time", "sat_success"]].notna().all(axis=1)
    df["done_okp"] = df[["okp_cr", "okp_time", "okp_success"]].notna().all(axis=1)
    df_lock = asyncio.Lock()
    tasks = [gather_results(df, df_lock, executable, out_file) for _ in range(subprocesses)]
    await asyncio.gather(*tasks)


if __name__ == "__main__":
    args = get_arguments()
    asyncio.run(run_experiments(args.graphs_file, args.subprocesses, args.exec, args.out))
