import re
import argparse
import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
from collections import defaultdict

def get_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("infiles", nargs="+", help="Input file to include",
                        type=str)
    parser.add_argument("-o", "--output", dest="outfile", default="data/out/output.png",
                         help="Output file", type=str)
    return parser.parse_args()

def read_data(file_path):
    with open(file_path, "r") as file:
        data = file.readlines()
    stats = []
    graph_data = ""
    for line in data:
        data_match = re.match(r'^(.*): (\d+) (\d+)\n$', line)
        graph_match = re.match(r'^\s*\d+: (.*) \((\d+)\)\n$', line)
        if data_match:
            stats.append(graph_data + [data_match.group(1), int(data_match.group(2)), int(data_match.group(3))])
        elif graph_match:
            graph_data = [graph_match.group(1), int(graph_match.group(2))]
    return pd.DataFrame(stats, columns=["graph", "bicomponents", "method", "k", "time"])

def plot_data(data, x, y, hue, outfile):
    sns.boxplot(x=x, y=y, hue=hue, data=data, showfliers=False,
                   palette=sns.cubehelix_palette(data[hue].nunique()))
    plt.savefig(outfile)
    plt.clf()

def main(input_files, output_file):
    data = pd.concat([read_data(file) for file in input_files])
    plot_data(data, "bicomponents", "time", "method", output_file)

if __name__ == "__main__":
    args = get_arguments()
    main(args.infiles, args.outfile)
