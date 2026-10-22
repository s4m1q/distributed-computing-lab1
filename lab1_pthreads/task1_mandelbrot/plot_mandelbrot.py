import csv
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

src = sys.argv[1] if len(sys.argv) > 1 else "mandelbrot.csv"
dst = sys.argv[2] if len(sys.argv) > 2 else "mandelbrot.png"

xs, ys = [], []
with open(src) as f:
    for row in csv.DictReader(f):
        xs.append(float(row["x"]))
        ys.append(float(row["y"]))

plt.figure(figsize=(7, 6))
plt.scatter(xs, ys, s=0.2, color="black", linewidths=0)
plt.gca().set_aspect("equal")
plt.xlabel("Re c")
plt.ylabel("Im c")
plt.title(f"Найдено точек множества Мандельброта: {len(xs)}")
plt.savefig(dst, dpi=150, bbox_inches="tight")
print("Сохранено:", dst)
