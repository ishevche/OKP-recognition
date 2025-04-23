#! /usr/bin/env python3
import argparse
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import pandas as pd


def get_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("x", default="cr", help="column to use along X-axis (default: cr)")
    parser.add_argument("y", default="time", help="column to use along Y-axis (default: time)")
    parser.add_argument("hue", default="method", help="column to use for grouping (default: method)")
    parser.add_argument("-i", "--input", dest="infile", default="data/results.csv",
                        help="file that contains results of the experiment (default: data/results.csv)")
    parser.add_argument("-o", "--output", dest="outfile", default="data/out/output.eps",
                        help="file in which the plot is saved (default: data/out/output.eps)")
    parser.add_argument("--hue-order", dest="hue_order",
                        help="order of groups in legend")
    parser.add_argument("--x-label", dest="x_label", default="Crossing number",
                        help="label for X-axis (default: Crossing number)")
    parser.add_argument("--y-label", dest="y_label", default="Time,s",
                        help="label for Y-axis (default: Time,s)")
    parser.add_argument("--log-scale", dest="log_scale", action=argparse.BooleanOptionalAction,
                        help="use logarithmic scale for Y-axis (default: no)")
    return parser.parse_args()


def read_input(input_files):
    data = pd.read_csv(input_files)
    # data = data[data['success'].notna()]
    data = data[data['success'] == True]
    # data = data[data['cr'] <= 7]
    inconsistencies = data.groupby("graph_dot")["cr"].nunique()
    if (inconsistencies > 1).any():
        raise ValueError(f"Inconsistent crossing number in the input data")
    data['time'] /= 10 ** 9
    data['time'] = data['time'].replace(0., 60. * 10.)
    data = data.astype({"cr": "int64"})
    return data


def main(input_files, output_file, x, y, hue, hue_order, x_label, y_label, log_scale):
    data = read_input(input_files)
    sns.boxplot(x=x, y=y, hue=hue, data=data, showfliers=False,
                palette=sns.cubehelix_palette(data[hue].nunique()),
                hue_order=hue_order.split(",") if hue_order else None)
    plt.xlabel(x_label)
    plt.ylabel(y_label)
    if log_scale:
        plt.yscale('log')
    plt.savefig(output_file)
    plt.clf()


if __name__ == "__main__":
    args = get_arguments()
    main(args.infile, args.outfile, args.x, args.y, args.hue, args.hue_order,
         args.x_label, args.y_label, args.log_scale)
