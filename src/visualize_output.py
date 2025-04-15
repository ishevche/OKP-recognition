#! /usr/bin/env python3
import argparse
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import pandas as pd


def get_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("infile", default="data/results.csv",
                        help="File that contains results of the experiment", type=str)
    parser.add_argument("-o", "--output", dest="outfile", default="data/out/output.svg",
                        help="File in which the plot is saved", type=str)
    return parser.parse_args()

def sync_cr(data):
    data['ilp_cr'] = data['ilp_cr'].where(data['ilp_success'], np.nan)
    data['okp_cr'] = data['okp_cr'].where(data['okp_success'], np.nan)
    data['sat_cr'] = data['sat_cr'].where(data['sat_success'], np.nan)
    rows = data[['ilp_cr', 'okp_cr', 'sat_cr']].dropna(how='all')
    diff_cr = rows.min(axis=1) != rows.max(axis=1)
    if diff_cr.any():
        raise ValueError(f"Non consistent crossing numbers:\n{rows[diff_cr]}")
    cr = data[['ilp_cr', 'okp_cr', 'sat_cr']].min(axis=1)
    data['ilp_cr'] = cr
    data['okp_cr'] = cr
    data['sat_cr'] = cr
    return data

def read_input(input_files):
    data = pd.read_csv(input_files)
    data = sync_cr(data)
    data = pd.wide_to_long(data, stubnames=['ilp', 'sat', 'okp'],
                           i=['graph_dot', 'graph_g6'],
                           j='attribute', sep='_', suffix='.+').reset_index()
    data = data.melt(id_vars=['graph_dot', 'graph_g6', 'attribute'],
                     value_vars=['ilp', 'sat', 'okp'],
                     var_name="method")
    data = data.pivot(index=['graph_dot', 'graph_g6', 'method'],
                      columns='attribute', values='value').reset_index()
    # data = data[data['success'].notna()]
    data = data[data['success'] == True]
    # data = data[data['cr'] <= 7]
    data['time'] /= 10 ** 9
    data['time'] = data['time'].replace(0., 60.*10.)
    data = data.astype({"cr": "int64"})
    return data

def main(input_files, output_file):
    data = read_input(input_files)
    sns.boxplot(x="cr", y="time", hue='method', data=data, showfliers=False,
                palette=sns.cubehelix_palette(data['method'].nunique()),
                hue_order=["ilp", "sat", "okp"])
    plt.xlabel("Crossing number")
    plt.ylabel("Time, s")
    plt.yscale('log')
    plt.savefig(output_file)
    plt.clf()

if __name__ == "__main__":
    args = get_arguments()
    main(args.infile, args.outfile)
