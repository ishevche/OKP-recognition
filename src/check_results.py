import re


def read_ILP(file_path):
    ILP_results = {}
    with open(file_path, "r") as file:
        data = file.readlines()
    cur_graph = None
    for line in data:
        if line.endswith(":\n"):
            cur_graph = line.split()[-1][:-1]
        if line.startswith("ILP:"):
            k = map(int, re.findall(r'^ILP: (\d*) \d*$', line)[0])
            ILP_results[cur_graph] = list(k)[0]
    return ILP_results

def check_ILP(first, second):
    ILP_results1 = read_ILP(first)
    ILP_results2 = read_ILP(second)
    for key in ILP_results1:
        if ILP_results1[key] != ILP_results2[key]:
            print(f"Graph {key} has different ILP results: {ILP_results1[key]} vs {ILP_results2[key]}")
    

if __name__ == "__main__":
    check_ILP("data/first.txt", "data/second.txt")
