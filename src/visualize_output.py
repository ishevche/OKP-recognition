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

def plot_data(data1, data2):
    ILP_stats1, SAT_pos_stats1, SAT_neg_stats1 = data1
    ILP_stats2, SAT_pos_stats2, SAT_neg_stats2 = data2

    def plot_dicts(dict1, dict2, filename, y_limits):
        keys = sorted(set(dict1.keys()).union(dict2.keys()))
        values1 = [dict1.get(key, []) for key in keys]
        values2 = [dict2.get(key, []) for key in keys]

        plt.figure(figsize=(10, 6))
        box1 = plt.boxplot(values1, positions=[x - 0.2 for x in range(1, len(keys) + 1)], widths=0.4, patch_artist=True, boxprops=dict(facecolor="lightblue"))
        box2 = plt.boxplot(values2, positions=[x + 0.2 for x in range(1, len(keys) + 1)], widths=0.4, patch_artist=True, boxprops=dict(facecolor="lightgreen"))

        plt.xticks(range(1, len(keys) + 1), keys)
        plt.ylim(y_limits)
        plt.legend([box1["boxes"][0], box2["boxes"][0]], ["Data1", "Data2"], loc="upper right")
        plt.savefig(filename)
        plt.clf()

    # Determine common y-axis limits
    all_values = []
    for stats in [ILP_stats1, SAT_pos_stats1, SAT_neg_stats1, ILP_stats2, SAT_pos_stats2, SAT_neg_stats2]:
        for key in stats:
            all_values.extend(stats[key])
    y_limits = (min(all_values), max(all_values) * 0.15)

    plot_dicts(ILP_stats1, ILP_stats2, "data/out/ILP.png", y_limits)
    plot_dicts(SAT_pos_stats1, SAT_pos_stats2, "data/out/SAT_pos.png", y_limits)
    plot_dicts(SAT_neg_stats1, SAT_neg_stats2, "data/out/SAT_neg.png", y_limits)

if __name__ == "__main__":
    data1 = read_data("data/first.txt")
    data2 = read_data("data/second.txt")
    plot_data(data1, data2)
