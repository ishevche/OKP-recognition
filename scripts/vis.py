#!/usr/bin/env python3
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import pandas as pd
import networkx as nx
from matplotlib import rcParams

rcParams['figure.figsize'] = 10.5, 7
rcParams['font.size'] = 36
rcParams["text.usetex"] = True

df = pd.read_csv("data/results_600.csv")
success = df["success"] == True
timeout = (df["success"] == False) & (df["time"] == 600 * (10 ** 9))
memory = df["success"].isna()
df["Outcome"] = ""
df["Outcome"].where(~success, "OK", inplace=True)
df["Outcome"].where(~timeout, "Timed out", inplace=True)
df["Outcome"].where(~memory, "Exceeded memory", inplace=True)
df["nv"] = df["graph_g6"].apply(lambda x: nx.from_graph6_bytes(x.encode()).number_of_nodes())
df.astype({"nv": "int64"})

fig, ax = plt.subplots(1, 1, figsize=(10.5, 7.6))
plt.subplots_adjust(left=0.13, right=0.99, top=0.97, bottom=0.13)
data = df[df["method"] == "sat"]
sns.histplot(data, x="nv", hue="Outcome", multiple="stack", ax=ax, discrete=True, palette=sns.color_palette()[2::-2])
ax.set_xticks(data["nv"].unique())
ax.set_xlabel("Number of vertices")
ax.set_ylabel("Number of graphs")
fig.savefig("thesis/Figures/experiments/outcome_sat.pdf")
plt.clf()


fig, ax = plt.subplots(1, 1, figsize=(10.5, 7.6))
plt.subplots_adjust(left=0.13, right=0.99, top=0.97, bottom=0.13)
data = df[df["method"] == "ilp"]
sns.histplot(data, x="nv", hue="Outcome", multiple="stack", ax=ax, discrete=True, palette=sns.color_palette()[2:3])
ax.set_xticks(data["nv"].unique())
ax.set_xlabel("Number of vertices")
ax.set_ylabel("Number of graphs")
fig.savefig("thesis/Figures/experiments/outcome_ilp.pdf")
plt.clf()


fig, ax = plt.subplots(1, 1, figsize=(10.5, 7.6))
plt.subplots_adjust(left=0.13, right=0.99, top=0.97, bottom=0.13)
data = df[df["method"] == "okp"]
sns.histplot(data, x="nv", hue="Outcome", multiple="stack", ax=ax, discrete=True, palette=sns.color_palette()[2::-1])
ax.set_xticks(data["nv"].unique())
ax.set_xlabel("Number of vertices")
ax.set_ylabel("Number of graphs")
fig.savefig("thesis/Figures/experiments/outcome_okp.pdf")
plt.clf()

