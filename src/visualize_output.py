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


def read_input(input_files):
    data = pd.read_csv(input_files)
    # data = data[data['success'].notna()]
    data = data[data['success'] == True]
    # data = data[data['cr'] <= 7]
    data['time'] /= 10 ** 9
    data['time'] = data['time'].replace(0., 60. * 10.)
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
