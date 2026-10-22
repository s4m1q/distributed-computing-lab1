import random
import sys

n = int(sys.argv[1])
seed = int(sys.argv[2]) if len(sys.argv) > 2 else 1
random.seed(seed)

print(n)
for _ in range(n):
    m = random.uniform(1e22, 1e25)
    x, y, z = (random.uniform(-1e12, 1e12) for _ in range(3))
    vx, vy, vz = (random.uniform(-1e4, 1e4) for _ in range(3))
    print(f"{m:.6e} {x:.6e} {y:.6e} {z:.6e} {vx:.6e} {vy:.6e} {vz:.6e}")
