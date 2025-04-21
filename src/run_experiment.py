#! /usr/bin/env python3
import argparse
import re
import os
import asyncio
import sys

import pandas as pd
import pydot
import networkx as nx


def get_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("graphs_file", help="file containing graphs in Graphviz format", default="data/graphs.txt")
    parser.add_argument("-p", "--subprocesses", help="Number of subprocesses to use at the same time. Default: 1",
                        dest="subprocesses", type=int, default=1)
    parser.add_argument("-b", "--bin-executables",
                        help="Path to an executables separated by ','. Default: bin/okp-recognition-cro",
                        dest="execs", default="bin/okp-recognition-cro")
    parser.add_argument("-o", "--output", help="Output file for all evaluations. Default: data/results.csv",
                        dest="out", default="data/results.csv")
    parser.add_argument("-m", "--methods",
                        help="Methods to use, separated by ','. Possible values: ilp, sat, okp. Default: ilp,sat,okp",
                        dest="methods", default="ilp,sat,okp")
    parser.add_argument("-t", "--timeout", help="Timeout for each subprocess in seconds. Default: 600",
                        dest="timeout", type=int, default=600)
    parser.add_argument("--resume", help="Resume the experiments from the output file. Default: False",
                        dest="resume", action=argparse.BooleanOptionalAction)
    return parser.parse_args()


def dot_to_g6(graph: str):
    pydot_graph = pydot.graph_from_dot_data(graph)
    nx_graph = nx.Graph(nx.nx_pydot.from_pydot(pydot_graph[0]))
    return nx.to_graph6_bytes(nx_graph, header=False).decode().strip()


def initialise_df(graphs_file: str, out_file: str, executables: list, methods: list):
    with open(graphs_file, "r") as in_file:
        graphs_string = in_file.read()
    graphs_strings = re.findall(r'graph\s+\w\s+\{.*?\}', graphs_string)
    graphs = []
    for graph in graphs_strings:
        graphs.append((graph, dot_to_g6(graph), executables, methods))
    df = pd.DataFrame(data=graphs, columns=["graph_dot", "graph_g6", "executable", "method"])
    df = df.reindex(columns=["graph_dot", "graph_g6", "executable", "method", "cr", "time", "success"])
    df = df.explode("executable").explode("method").reset_index(drop=True)
    df.to_csv(out_file, index=False)
    return df


async def gather_results(df: pd.DataFrame, df_lock, out_file: str, timeout: int):
    save_columns = list(filter(lambda x: not x.startswith("done"), df.columns))
    while True:
        async with df_lock:
            todo_graphs_index = df[~df["done"]].index
            if todo_graphs_index.empty:
                break
            graph_index = todo_graphs_index[0]
            df.at[graph_index, "done"] = True
            graph_str = df.at[graph_index, "graph_dot"]
            graph_g6 = df.at[graph_index, "graph_g6"]
            method = df.at[graph_index, "method"]
            executable = df.at[graph_index, "executable"]
        subprocess = await asyncio.create_subprocess_shell(f"{executable} \"{graph_str}\" -m {method}",
                                                           stdout=asyncio.subprocess.PIPE,
                                                           stderr=asyncio.subprocess.PIPE)
        print(f"Running {method} for graph {graph_g6}...")
        try:
            stdout, stderr = await asyncio.wait_for(subprocess.communicate(), timeout=timeout)
            print(f"{method} for graph {graph_g6} completed.")
            if subprocess.returncode != 0:
                print(f"{method} for {graph_g6} failed with return code {subprocess.returncode}:\n{stderr.decode()}",
                      file=sys.stderr)
                continue
            else:
                solved, cr, time = map(int, stdout.decode().split(" "))
        except asyncio.exceptions.TimeoutError:
            subprocess.kill()
            print(f"{method} for graph {graph_g6} timed out.")
            solved, cr, time = False, 0, timeout * (10 ** 9)
        async with df_lock:
            df.at[graph_index, "cr"] = cr
            df.at[graph_index, "time"] = time
            df.at[graph_index, "success"] = bool(solved)
            df.to_csv(out_file, index=False, columns=save_columns)


async def run_experiments(graphs_file: str, subprocesses: int, executables: list,
                          out_file: str, methods: list, timeout: int, resume: bool):
    if os.path.exists(out_file):
        if resume:
            df = pd.read_csv(out_file)
        else:
            sys.exit("Output file already exists. Use --resume to resume the experiments.")
    else:
        df = initialise_df(graphs_file, out_file, executables, methods)
    df["done"] = df.notna().all(axis=1)
    df_lock = asyncio.Lock()
    tasks = [gather_results(df, df_lock, out_file, timeout) for _ in range(subprocesses)]
    await asyncio.gather(*tasks)


if __name__ == "__main__":
    args = get_arguments()
    asyncio.run(run_experiments(
        args.graphs_file, args.subprocesses, args.execs.split(','),
        args.out, args.methods.split(','), args.timeout, args.resume
    ))
