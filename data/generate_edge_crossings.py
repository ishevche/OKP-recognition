#! /usr/bin/env python
from itertools import permutations
import subprocess


with open("data/edges_arrangements.ipe") as input_file:
    text = input_file.read()

for perm in permutations("uvst"):
    first, second, third, forth = perm
    u_index = perm.index("u")
    v_index = perm.index("v")
    s_index = perm.index("s")
    t_index = perm.index("t")
    e1_start = 100 + 32 * u_index
    e1_end = 100 + 32 * v_index
    e1_middle = (e1_start + e1_end) // 2
    e1_height = 768 + abs(e1_start - e1_end) // 2
    e2_start = 100 + 32 * s_index
    e2_end = 100 + 32 * t_index
    e2_middle = (e2_start + e2_end) // 2
    e2_height = 768 + abs(e2_start - e2_end) // 2
    with open("data/tmp.ipe", "w") as output_file:
        output_file.write(text.format(**locals()))
    subprocess.run(["iperender", "-pdf", "data/tmp.ipe", f"thesis/Figures/edge_cross/{''.join(perm)}.pdf"])
