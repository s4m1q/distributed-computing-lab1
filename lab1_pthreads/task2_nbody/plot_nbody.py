import csv
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

src = sys.argv[1] if len(sys.argv) > 1 else "nbody.csv"
dst = sys.argv[2] if len(sys.argv) > 2 else "nbody.png"

with open(src) as f:
    rows = list(csv.reader(f))
header, data = rows[0], [list(map(float, r)) for r in rows[1:]]
n = (len(header) - 1) // 2

plt.figure(figsize=(7, 7))
for q in range(n):
    xs = [r[1 + 2 * q] for r in data]
    ys = [r[2 + 2 * q] for r in data]
    plt.plot(xs, ys, linewidth=1, label=f"тело {q + 1}")
    plt.plot(xs[0], ys[0], "o", markersize=4, color="black")
plt.gca().set_aspect("equal")
plt.xlabel("x, м")
plt.ylabel("y, м")
plt.title(f"Траектории {n} тел, t от 0 до {data[-1][0]:g} с")
if n <= 10:
    plt.legend()
plt.savefig(dst, dpi=150, bbox_inches="tight")
print("Сохранено:", dst)
