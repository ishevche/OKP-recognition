import re
import matplotlib.pyplot as plt

def read_data(file_path):
    ILP_stats = {}
    SAT_pos_stats = {}
    SAT_neg_stats = {}
    with open(file_path, "r") as file:
        data = file.readlines()
    for line in data:
        if line.startswith("ILP:"):
            k, time = map(int, re.findall(r'^ILP: (\d*) (\d*)$', line)[0])
            ILP_stats.setdefault(k, []).append(time)
        elif line.startswith("SAT:"):
            k, sat, time = map(int, re.findall(r'^SAT: (\d*) (0|1) (\d*)$', line)[0])
            if sat == 0:
                SAT_neg_stats.setdefault(k, []).append(time)
            else:
                SAT_pos_stats.setdefault(k, []).append(time)
    return ILP_stats, SAT_pos_stats, SAT_neg_stats

def plot_data(data):
    ILP_stats, SAT_pos_stats, SAT_neg_stats = data

    def plot_dict(data_dict, filename, y_limits):
        keys = sorted(data_dict.keys())
        values = [data_dict[key] for key in keys]
        plt.boxplot(values)
        plt.xticks(range(1, len(keys) + 1), keys)
        plt.ylim(y_limits)
        plt.savefig(filename)
        plt.clf()

    all_values = []
    for stats in [ILP_stats, SAT_pos_stats, SAT_neg_stats]:
        for key in stats:
            all_values.extend(stats[key])
    y_limits = (min(all_values), max(all_values) * 0.15)

    print(max(all_values))

    plot_dict(ILP_stats, "data/out/ILP.png", y_limits)
    plot_dict(SAT_pos_stats, "data/out/SAT_pos.png", y_limits)
    plot_dict(SAT_neg_stats, "data/out/SAT_neg.png", y_limits)

if __name__ == "__main__":
    data = read_data("data/output.txt")
    plot_data(data)
