from matplotlib import rcParams
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import pandas as pd
import networkx as nx

PALETTE = "muted"
rcParams['font.size'] = 32
rcParams["text.usetex"] = True


def read_input(input_file):
    data = pd.read_csv(input_file)
    data = data[data['success'] == True]
    inconsistencies = data.groupby("graph_dot")["cr"].nunique()
    if (inconsistencies > 1).any():
        raise ValueError(f"Inconsistent crossing number in the input data")
    data['time'] /= 10 ** 9
    data["vertices"] = data["graph_g6"].apply(lambda x: nx.from_graph6_bytes(x.encode()).number_of_nodes())
    data = data.astype({"cr": "int64", "vertices": "int64"})
    return data


def visualise_biconnected(input_file, output_file, methods):
    results = read_input(input_file)
    results = results[(4 <= results['vertices']) & (results['cr'] <= 7)]
    results = results.rename({"executable": "Decomposition"}, axis=1)
    results = results.replace({"Decomposition": {"bin/okp-recognition-obj --no_bct": "No",
                                                 "bin/okp-recognition-obj": "Yes"}})
    for method in methods:
        data = results[results['method'] == method]
        fig, ax = plt.subplots(1, 2, figsize=(21, 7.6))
        plt.subplots_adjust(left=0.07, right=0.99, top=0.97, bottom=0.13)

        sns.boxplot(x="cr", y="time", hue="Decomposition", data=data, showfliers=False,
                    palette=sns.color_palette(PALETTE)[:2],
                    ax=ax[0], log_scale=(False, True))
        ax[0].set_xlabel("Crossing number")
        ax[0].set_ylabel("Time in seconds")

        sns.boxplot(x="vertices", y="time", hue="Decomposition", data=data, showfliers=False,
                    palette=sns.color_palette(PALETTE)[:2],
                    ax=ax[1], log_scale=(False, True))
        ax[1].set_xlabel("Number of vertices")
        ax[1].set_ylabel("Time in seconds")

        fig.savefig(output_file.format(method=method))
        plt.clf()


def visualise_comparison(input_file, output_file):
    data = read_input(input_file)
    data = data.rename({"method": "Algorithm"}, axis=1)
    data = data.replace({"Algorithm": {"ilp": "\\textsf{ILP}", "sat": "\\textsf{SAT}", "dp": "\\textsf{DP}"}})
    fig, ax = plt.subplots(1, 1, figsize=(21, 7.6))
    plt.subplots_adjust(left=0.07, right=0.99, top=0.97, bottom=0.13)
    sns.boxplot(x="cr", y="time", hue="Algorithm", data=data, showfliers=False,
                palette=sns.color_palette(PALETTE)[2:5],
                hue_order=["\\textsf{ILP}", "\\textsf{SAT}", "\\textsf{DP}"],
                ax=ax, log_scale=(False, True))
    ax.set_xlabel("Crossing number")
    ax.set_ylabel("Time in seconds")
    fig.savefig(output_file)
    plt.clf()


def visualise_comparison_by_cr(input_file, output_file, crossing_numbers):
    result = read_input(input_file)
    result = result.rename({"method": "Algorithm"}, axis=1)
    result = result.replace({"Algorithm": {"ilp": "\\textsf{ILP}", "sat": "\\textsf{SAT}", "dp": "\\textsf{DP}"}})
    for crossing_number in crossing_numbers:
        data = result[result['cr'] == crossing_number]
        fig, ax = plt.subplots(1, 1, figsize=(10.5, 7.6))
        plt.subplots_adjust(left=0.13, right=0.99, top=0.97, bottom=0.13)

        sns.boxplot(x="vertices", y="time", hue="Algorithm", data=data, showfliers=False,
                    palette=sns.color_palette(PALETTE)[2:5],
                    hue_order=["\\textsf{ILP}", "\\textsf{SAT}", "\\textsf{DP}"],
                    ax=ax, log_scale=(False, True))
        ax.set_xlabel("Number of vertices")
        ax.set_ylabel("Time in seconds")

        fig.savefig(output_file.format(crossing_number=crossing_number))
        plt.clf()


def visualise_ilp_optimisations(input_file, output_file):
    data = read_input(input_file)
    data = data.rename({"executable": "Configuration"}, axis=1)
    data = data.replace({"Configuration": {"bin/okp-recognition": "No optimisations",
                                           "bin/okp-recognition-obj": "Objective optimisation",
                                           "bin/okp-recognition-exact": "Exact crossing variables",
                                           "bin/okp-recognition-exact-obj": "Both"}})
    fig, ax = plt.subplots(1, 1, figsize=(21, 7.6))
    plt.subplots_adjust(left=0.07, right=0.99, top=0.97, bottom=0.13)
    colors = sns.color_palette(PALETTE)[5:]
    sns.boxplot(x="cr", y="time", hue="Configuration", data=data, showfliers=False,
                palette=colors[:2][::-1] + colors[3:],
                hue_order=["No optimisations", "Objective optimisation", "Exact crossing variables", "Both"],
                ax=ax, log_scale=(False, True))
    ax.set_xlabel("Crossing number")
    ax.set_ylabel("Time in seconds")
    fig.savefig(output_file)
    plt.clf()


def visualise_sat_optimisations(input_file, output_file):
    data = read_input(input_file)
    data = data.rename({"executable": "Configuration"}, axis=1)
    data = data.replace({"Configuration": {"bin/okp-recognition": "No optimisations",
                                           "bin/okp-recognition-exact": "Exact crossing variables"}})
    fig, ax = plt.subplots(1, 1, figsize=(10.5, 7.6))
    plt.subplots_adjust(left=0.13, right=0.99, top=0.97, bottom=0.13)
    sns.boxplot(x="cr", y="time", hue="Configuration", data=data, showfliers=False,
                palette=sns.color_palette(PALETTE)[6::2],
                hue_order=["No optimisations", "Exact crossing variables"],
                ax=ax, log_scale=(False, True))
    ax.set_xlabel("Crossing number")
    ax.set_ylabel("Time in seconds")
    fig.savefig(output_file)
    plt.clf()


def visualise_outcomes(input_file, output_file, methods):
    rcParams['font.size'] = 36
    data = pd.read_csv(input_file)
    data['time'] /= 10 ** 9
    data["vertices"] = data["graph_g6"].apply(lambda x: nx.from_graph6_bytes(x.encode()).number_of_nodes())
    data = data.astype({"vertices": "int64"})

    success = data["success"] == True
    timeout = (data["success"] == False) & (data["time"] == 600)
    memory = data["success"].isna()
    data["Outcome"] = ""
    data["Outcome"].where(~success, "OK", inplace=True)
    data["Outcome"].where(~timeout, "Timed out", inplace=True)
    data["Outcome"].where(~memory, "Exceeded memory", inplace=True)

    for method in methods:
        fig, ax = plt.subplots(1, 1, figsize=(10.5, 7.6))
        plt.subplots_adjust(left=0.13, right=0.99, top=0.97, bottom=0.13)
        sns.histplot(x="vertices", hue="Outcome", data=data[data["method"] == method],
                     multiple="stack", ax=ax, discrete=True,
                     palette=sns.color_palette()[2::-1],
                     hue_order=["OK", "Timed out", "Exceeded memory"])
        ax.set_xticks(data["vertices"].unique())
        ax.set_xlabel("Number of vertices")
        ax.set_ylabel("Number of graphs")
        fig.savefig(output_file.format(method=method))
        plt.clf()

    rcParams['font.size'] = 32
